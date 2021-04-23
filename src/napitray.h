#ifndef _NAPITRAY_H
#define _NAPITRAY_H

#include <napi.h>
#include "napitrayitem.h"

#define THROW(error) Napi::TypeError::New( env, error ).ThrowAsJavaScriptException()

struct tray_menu;
typedef struct tray_menu {
    char *text;
    int disabled;
    int checked;

    void (*cb)(struct tray_menu*);
    void *context;

    struct tray_menu* submenu;
} tray_menu_t;

//
// Convert JS Objects to Internal Struct
//
void InitTrayMenu(tray_menu_t* r, Napi::Env env, Napi::Object obj);

tray_menu_t* NapiArray2TrayMenu(Napi::Env env, Napi::Array napimenu){

    size_t msize = napimenu.Length();
    struct tray_menu* r = new struct tray_menu[msize+1];

    for (size_t i = 0; i < msize; i++){
        Napi::Value curr = napimenu.Get(i);
        if( curr.IsString() ){
            Napi::Object c = Napi::Object::New(env);
            c.Set("text", curr.As<Napi::String>());
            curr = c;
        }
        if( !curr.IsObject() ){
            THROW("Expected all array elements to be Objects.");
            return NULL;
        }

        InitTrayMenu(&r[i], env, curr.As<Napi::Object>());
    }

    r[msize].text = NULL;
    r[msize].checked = 0;
    r[msize].disabled = 1;
    r[msize].cb = NULL;
    r[msize].submenu = NULL;
    r[msize].context = NULL;
    return r;
}

struct tscb{
    Napi::ThreadSafeFunction callback;
    Napi::ObjectReference obj;
};


void InitTrayMenu(tray_menu_t* r, Napi::Env env, Napi::Object obj){
    if( !obj.Has("text") ) {
        THROW("Object requires \"text\" property.");
        return;
    }
    if( !obj.Get("text").IsString() ){
        THROW("Expected \"text\" property to be a string.");
        return;
    }

    std::string text = obj.Get("text").As<Napi::String>().Utf8Value();
    r->text = new char[text.length()+1];
    strcpy( r->text, text.c_str() );
    
    if( obj.Has("submenu") ){
        if( !obj.Get("submenu").IsArray() ){
            THROW("Expected \"submmenu\" property to be an array.");
            return;
        }
        r->submenu = NapiArray2TrayMenu(env, obj.Get("submenu").As<Napi::Array>());
    }
    else{
        r->submenu = NULL;
    }

    if( obj.Has("disabled") ){
        if( !obj.Get("disabled").IsBoolean() ){
            THROW("Expected \"disabled\" property to be a boolean.");
            return;
        }
        r->disabled = obj.Get("disabled").As<Napi::Boolean>().Value(); 
    }
    else{
        r->disabled = 0;
    }
    

    if( obj.Has("checked") ){
        if( !obj.Get("checked").IsBoolean() ){
            THROW("Expected \"checked\" property to be a boolean.");
            return;
        }
        r->checked = obj.Get("checked").As<Napi::Boolean>().Value(); 
    }
    else{
        r->checked = 0;
    }
    
    if( obj.Has("callback") ){
        if( !obj.Get("callback").IsFunction() ){
            THROW("Expected \"callback\" property to be a function.");
            return;
        }
        Napi::Function napicallback = obj.Get("callback").As<Napi::Function>();
    
        struct tscb *cb = new (struct tscb);
        cb->callback = Napi::ThreadSafeFunction::New(env, napicallback, "click-callback", 0, 1); 
        cb->obj = Napi::Persistent(obj);
        r->context = cb;
        r->cb = [](tray_menu_t *ctx){
            auto callback = [](Napi::Env env, Napi::Function jsCall, void* obj){
                jsCall.Call({((Napi::ObjectReference*) obj)->Value()});
            };
            struct tscb* tsf = (struct tscb*) ctx->context;
            tsf->callback.BlockingCall( &tsf->obj, callback );
        };
    }
    else{
        r->context = NULL;
        r->cb = NULL;
    }
}

template<typename T>
class NapiTray : public Napi::ObjectWrap<T> {
    public:
        static Napi::Object Init(Napi::Env env, Napi::Object exports){
            NapiTrayItem::Init(env, exports);

            Napi::Function func =
                NapiTray::DefineClass(env, "Tray", {
                    NapiTray::InstanceMethod("start", &T::Start),
                    NapiTray::InstanceMethod("update", &T::Update),
                    NapiTray::InstanceMethod("stop", &T::Stop),
                    NapiTray::InstanceAccessor("icon", &T::GetIcon, &T::SetIcon),
                    NapiTray::InstanceAccessor("menu", &T::GetMenu, &T::SetMenu)});
            
            Napi::FunctionReference* constructor = new Napi::FunctionReference();
            *constructor = Napi::Persistent(func);
            env.SetInstanceData(constructor);

            return func;
        }
        
        NapiTray(const Napi::CallbackInfo& info) : Napi::ObjectWrap<T>(info) {
            Napi::Env env = info.Env();
            if( info.Length() < 2 ){
                THROW("Expected two arguments, \"icon\": string and \"menu\": array.");
                return;
            }

            SetIcon(info, info[0].As<Napi::Value>());
            SetMenu(info, info[1].As<Napi::Value>());
        }

        void SetIcon(const Napi::CallbackInfo& info, const Napi::Value& arg){
            Napi::Env env = info.Env();
            if( !arg.IsString() ){
                THROW("Expected argument \"icon\" to be a string.");
                return;
            }
            Napi::String str = arg.As<Napi::String>();

            std::string iname = str.Utf8Value();
            icon = new char[iname.length()+1];
            strcpy(icon, iname.c_str());
        }

        Napi::Value GetIcon(const Napi::CallbackInfo& info){
            return Napi::String::New(info.Env(), icon);
        }

        void SetMenu(const Napi::CallbackInfo& info, const Napi::Value& arg){
            Napi::Env env = info.Env();
            if( !arg.IsArray() ){
                THROW("Expected argument \"menu\" to be an array.");
                return;
            }
            Napi::Array arr = arg.As<Napi::Array>();
            //napimenu = arg.As<Napi::Array>();

            if( arr.Length() < 1 ){
                THROW("Expected argument \"menu\" to have at least one element.");
                return;
            }

            this->menu = NapiArray2TrayMenu(env, arr);
            napimenu = Napi::Reference<Napi::Array>::New(arr, 1);
        }

        Napi::Value GetMenu(const Napi::CallbackInfo& info){
            return napimenu.Value();
        }


        virtual Napi::Value Start(const Napi::CallbackInfo& info) = 0;
        virtual Napi::Value Stop(const Napi::CallbackInfo& info) = 0;
        virtual Napi::Value Update(const Napi::CallbackInfo& info) = 0;
        virtual void Loop() = 0;

        char* icon;
        tray_menu_t* menu;
        
    protected: 
        // Js representation (getter, setter)
        Napi::Reference<Napi::Array> napimenu;

};

#endif