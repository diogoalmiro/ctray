#ifndef _NAPITRAYITEM_H
#define _NAPITRAYITEM_H

#include <napi.h>

#define THROW(error) Napi::TypeError::New( env, error ).ThrowAsJavaScriptException()

void Identity(const Napi::CallbackInfo& info){}

class NapiTrayItem : Napi::ObjectWrap<NapiTrayItem> {
    
    static Napi::Object Init(Napi::Env env, Napi::Object exports){
        Napi::Function func =
            NapiTray::DefineClass(env, "TrayItem", {
                NapiTray::InstanceAccessor("text", &T::GetText, &T::SetText),
                NapiTray::InstanceAccessor("checked", &T::GetChecked, &T::SetChecked),
                NapiTray::InstanceAccessor("disabled", &T::GetDisabled, &T::SetDisabled),
                NapiTray::InstanceAccessor("callback", &T::GetCallback, &T::SetCallback),
                NapiTray::InstanceAccessor("submenu", &T::GetSubmenu, &T::SetSubmenu)});
        
        Napi::FunctionReference* constructor = new Napi::FunctionReference();
        *constructor = Napi::Persistent(func);
        env.SetInstanceData(constructor);

        return func;
    }

    NapiTrayItem(const Napi::CallbackInfo& info) : Napi::ObjectWrap<NapiTrayItem>(info){
        Napi::Env env = info.Env();
        
        Napi::Value arg = info[0];
        if( arg.IsString() ){
            SetText(info, arg);
            SetChecked(info, Napi::Boolean::New(env, false) );
            SetDisabled(info, Napi::Boolean::New(env, false) );
            SetCallback(info, Napi::Function::New(env, &Identity));
            SetSubmenu(info, Napi::Array::New(env, 0));
        }
        else if( !arg.IsObject() ){
            THROW("Expected all array elements to be Objects or Strings.");
            return;
        }
        else{
            if( !obj.Has("text") ) {
                THROW("Object requires \"text\" property.");
                return;
            }
            if( !obj.Get("text").IsString() ){
                THROW("Expected \"text\" property to be a string.");
                return;
            }
            
            text = obj.Get("text").As<Napi::String>();
    
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
    }



    

    protected:
        Napi::String text;
        Napi::Boolean checked;
        Napi::Boolean disabled;
        Napi::Function callback;
        Napi::Array submenu;
}


#endif