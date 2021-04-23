#include "napitray.h"
#include "trayloop.h"

#include <objc/objc-runtime.h>
#include <limits.h>

static void menu_callback(id self, SEL cmd, id sender) {
  struct tray_menu *m =
      (struct tray_menu *)((id(*)(id, SEL))objc_msgSend)(((id(*)(id, SEL))objc_msgSend)(sender, sel_registerName("representedObject")), 
                  sel_registerName("pointerValue"));

    if (m != NULL && m->cb != NULL) {
      m->cb(m);
    }
}

static id _tray_menu(struct tray_menu *m) {
    id menu = ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSMenu"), sel_registerName("new"));
    ((id(*)(id, SEL))objc_msgSend)(menu, sel_registerName("autorelease"));
    ((id(*)(id, SEL, bool))objc_msgSend)(menu, sel_registerName("setAutoenablesItems:"), false);

    for (; m != NULL && m->text != NULL; m++) {
      if (strcmp(m->text, "-") == 0) {
        ((id(*)(id, SEL, id))objc_msgSend)(menu, sel_registerName("addItem:"), 
          ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSMenuItem"), sel_registerName("separatorItem")));
      } else {
        id menuItem = ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSMenuItem"), sel_registerName("alloc"));
        ((id(*)(id, SEL))objc_msgSend)(menuItem, sel_registerName("autorelease"));
        ((id(*)(id, SEL, id, SEL, id))objc_msgSend)(menuItem, sel_registerName("initWithTitle:action:keyEquivalent:"),
                  ((id(*)(id, SEL, const char*))objc_msgSend)((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), m->text),
                  sel_registerName("menuCallback:"),
                  ((id(*)(id, SEL, const char*))objc_msgSend)((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), ""));
  
        ((id(*)(id, SEL, bool))objc_msgSend)(menuItem, sel_registerName("setEnabled:"), (m->disabled ? false : true));
          ((id(*)(id, SEL, bool))objc_msgSend)(menuItem, sel_registerName("setState:"), (m->checked ? true : false));
          ((id(*)(id, SEL, id))objc_msgSend)(menuItem, sel_registerName("setRepresentedObject:"),
            ((id(*)(id, SEL, void*))objc_msgSend)((id)objc_getClass("NSValue"), sel_registerName("valueWithPointer:"), m));
  
          ((id(*)(id, SEL, id))objc_msgSend)(menu, sel_registerName("addItem:"), menuItem);
  
          if (m->submenu != NULL) {
            ((id(*)(id, SEL, id, id))objc_msgSend)(menu, sel_registerName("setSubmenu:forItem:"), _tray_menu(m->submenu), menuItem);
      }
    }
  }

  return menu;
}

class Tray : public NapiTray<Tray> {
    public:
        Tray(const Napi::CallbackInfo& info) : NapiTray<Tray>(info) { }
        Napi::Value Start(const Napi::CallbackInfo& info) override{
            Napi::Env env = info.Env();
            TrayLoop<Tray> *loop = new TrayLoop<Tray>(env, this);
            loop->Queue();
            return loop->GetPromise();
        }

        Napi::Value Update(const Napi::CallbackInfo& info) override{
            Napi::Env env = info.Env();
            ((id(*)(id, SEL, id))objc_msgSend)(statusBarButton, sel_registerName("setImage:"), 
                ((id(*)(id, SEL, id))objc_msgSend)((id)objc_getClass("NSImage"), sel_registerName("imageNamed:"), 
                    ((id(*)(id, SEL, char*))objc_msgSend)((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), icon)));

            ((id(*)(id, SEL, id))objc_msgSend)(statusItem, sel_registerName("setMenu:"), _tray_menu(menu));
            return env.Undefined();
        }

        Napi::Value Stop(const Napi::CallbackInfo& info) override{
            ((id(*)(id, SEL, id))objc_msgSend)(app, sel_registerName("terminate:"), app);
            return info[0].Env().Undefined();
        }

        void Loop() override{
            pool = ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSAutoreleasePool"),
                          sel_registerName("new"));
  
            ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSApplication"),
                                sel_registerName("sharedApplication"));
        
            Class trayDelegateClass = objc_allocateClassPair(objc_getClass("NSObject"), "Tray", 0);
            class_addProtocol(trayDelegateClass, objc_getProtocol("NSApplicationDelegate"));
            class_addMethod(trayDelegateClass, sel_registerName("menuCallback:"), (IMP)menu_callback, "v@:@");
            objc_registerClassPair(trayDelegateClass);
        
            id trayDelegate = ((id(*)(id, SEL))objc_msgSend)((id)trayDelegateClass,
                                sel_registerName("new"));
        
            app = ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSApplication"),
                                sel_registerName("sharedApplication"));
        
            ((id(*)(id, SEL, id))objc_msgSend)(app, sel_registerName("setDelegate:"), trayDelegate);
        
            statusBar = ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSStatusBar"),
                                sel_registerName("systemStatusBar"));
        
            statusItem = ((id(*)(id, SEL, float))objc_msgSend)(statusBar, sel_registerName("statusItemWithLength:"), -1.0);
        
            ((id(*)(id, SEL))objc_msgSend)(statusItem, sel_registerName("retain"));
            ((id(*)(id, SEL, bool))objc_msgSend)(statusItem, sel_registerName("setHighlightMode:"), true);
            statusBarButton = ((id(*)(id, SEL))objc_msgSend)(statusItem, sel_registerName("button"));
            
              ((id(*)(id, SEL, id))objc_msgSend)(statusBarButton, sel_registerName("setImage:"), 
                ((id(*)(id, SEL, id))objc_msgSend)((id)objc_getClass("NSImage"), sel_registerName("imageNamed:"), 
                    ((id(*)(id, SEL, char*))objc_msgSend)((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), icon)));

            ((id(*)(id, SEL, id))objc_msgSend)(statusItem, sel_registerName("setMenu:"), _tray_menu(menu));

            ((id(*)(id, SEL, bool))objc_msgSend)(app, sel_registerName("activateIgnoringOtherApps:"), true);

            while(true){
                    id until = ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSDate"), sel_registerName("distantFuture"));
            
                id event = ((id(*)(id, SEL, long, id, id, bool))objc_msgSend)(app, sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:"), 
                            ULONG_MAX, 
                            until, 
                            ((id(*)(id, SEL, const char*))objc_msgSend)((id)objc_getClass("NSString"), 
                            sel_registerName("stringWithUTF8String:"), 
                            "kCFRunLoopDefaultMode"), 
                            true);
                if (event) {
                    ((id(*)(id, SEL, id))objc_msgSend)(app, sel_registerName("sendEvent:"), event);
                }
            }
        }
    private:
        id app;
        id pool;
        id statusBar;
        id statusItem;
        id statusBarButton;
};

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  return Tray::Init(env, exports);
}

NODE_API_MODULE(addon, Init)