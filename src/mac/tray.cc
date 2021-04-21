#include "napitray.h"
#include "trayloop.h"

#include <objc/objc-runtime.h>
#include <limits.h>

static void menu_callback(id self, SEL cmd, id sender) {
  struct tray_menu *m =
      (struct tray_menu *)objc_msgSend(objc_msgSend(sender, sel_registerName("representedObject")), 
                  sel_registerName("pointerValue"));

    if (m != NULL && m->cb != NULL) {
      m->cb(m);
    }
}

static id _tray_menu(struct tray_menu *m) {
    id menu = objc_msgSend((id)objc_getClass("NSMenu"), sel_registerName("new"));
    objc_msgSend(menu, sel_registerName("autorelease"));
    objc_msgSend(menu, sel_registerName("setAutoenablesItems:"), false);

    for (; m != NULL && m->text != NULL; m++) {
      if (strcmp(m->text, "-") == 0) {
        objc_msgSend(menu, sel_registerName("addItem:"), 
          objc_msgSend((id)objc_getClass("NSMenuItem"), sel_registerName("separatorItem")));
      } else {
        id menuItem = objc_msgSend((id)objc_getClass("NSMenuItem"), sel_registerName("alloc"));
        objc_msgSend(menuItem, sel_registerName("autorelease"));
        objc_msgSend(menuItem, sel_registerName("initWithTitle:action:keyEquivalent:"),
                  objc_msgSend((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), m->text),
                  sel_registerName("menuCallback:"),
                  objc_msgSend((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), ""));
  
        objc_msgSend(menuItem, sel_registerName("setEnabled:"), (m->disabled ? false : true));
          objc_msgSend(menuItem, sel_registerName("setState:"), (m->checked ? 1 : 0));
          objc_msgSend(menuItem, sel_registerName("setRepresentedObject:"),
            objc_msgSend((id)objc_getClass("NSValue"), sel_registerName("valueWithPointer:"), m));
  
          objc_msgSend(menu, sel_registerName("addItem:"), menuItem);
  
          if (m->submenu != NULL) {
            objc_msgSend(menu, sel_registerName("setSubmenu:forItem:"), _tray_menu(m->submenu), menuItem);
      }
    }
  }

  return menu;
}

class Tray : public NapiTray<Tray> {
    public:
        Napi::Value Start(){
            Napi::Env env = info.Env();
            TrayLoop<Tray> *loop = new TrayLoop<Tray>(env, this);
            loop->Queue();
            return loop->GetPromise();
        }

        Napi::Value Update(const Napi::CallbackInfo& info) override{
            Napi::Env env = info.Env();
            objc_msgSend(statusBarButton, sel_registerName("setImage:"), 
                objc_msgSend((id)objc_getClass("NSImage"), sel_registerName("imageNamed:"), 
                    objc_msgSend((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), icon)));

            objc_msgSend(statusItem, sel_registerName("setMenu:"), _tray_menu(menu));
            return env.Undefined();
        }

        Napi::Value Stop(const Napi::CallbackInfo& info) override{
            objc_msgSend(app, sel_registerName("terminate:"), app);
            return info[0].Env().Undefined();
        }

        void Loop(){
            pool = objc_msgSend((id)objc_getClass("NSAutoreleasePool"),
                          sel_registerName("new"));
  
            objc_msgSend((id)objc_getClass("NSApplication"),
                                sel_registerName("sharedApplication"));
        
            Class trayDelegateClass = objc_allocateClassPair(objc_getClass("NSObject"), "Tray", 0);
            class_addProtocol(trayDelegateClass, objc_getProtocol("NSApplicationDelegate"));
            class_addMethod(trayDelegateClass, sel_registerName("menuCallback:"), (IMP)menu_callback, "v@:@");
            objc_registerClassPair(trayDelegateClass);
        
            id trayDelegate = objc_msgSend((id)trayDelegateClass,
                                sel_registerName("new"));
        
            app = objc_msgSend((id)objc_getClass("NSApplication"),
                                sel_registerName("sharedApplication"));
        
            objc_msgSend(app, sel_registerName("setDelegate:"), trayDelegate);
        
            statusBar = objc_msgSend((id)objc_getClass("NSStatusBar"),
                                sel_registerName("systemStatusBar"));
        
            statusItem = objc_msgSend(statusBar, sel_registerName("statusItemWithLength:"), -1.0);
        
            objc_msgSend(statusItem, sel_registerName("retain"));
            objc_msgSend(statusItem, sel_registerName("setHighlightMode:"), true);
            statusBarButton = objc_msgSend(statusItem, sel_registerName("button"));
            
              objc_msgSend(statusBarButton, sel_registerName("setImage:"), 
                objc_msgSend((id)objc_getClass("NSImage"), sel_registerName("imageNamed:"), 
                    objc_msgSend((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), icon)));

            objc_msgSend(statusItem, sel_registerName("setMenu:"), _tray_menu(menu));

            objc_msgSend(app, sel_registerName("activateIgnoringOtherApps:"), true);

            while(true){
                    id until = objc_msgSend((id)objc_getClass("NSDate"), sel_registerName("distantFuture"));
            
                id event = objc_msgSend(app, sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:"), 
                            ULONG_MAX, 
                            until, 
                            objc_msgSend((id)objc_getClass("NSString"), 
                            sel_registerName("stringWithUTF8String:"), 
                            "kCFRunLoopDefaultMode"), 
                            true);
                if (event) {
                    objc_msgSend(app, sel_registerName("sendEvent:"), event);
                }
            }
        }
    private:
        id app;
        id pool;
        id statusBar;
        id statusItem;
        id statusBarButton;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  return Tray::Init(env, exports);
}

NODE_API_MODULE(addon, Init)