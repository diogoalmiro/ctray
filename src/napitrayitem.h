#ifndef _NAPITRAYITEM_H
#define _NAPITRAYITEM_H

#include <napi.h>
#include <stdlib.h>

#define THROW(error) Napi::TypeError::New( env, error ).ThrowAsJavaScriptException()

Napi::FunctionReference *NapiTrayItemConstructor;

class NapiTrayItem : public Napi::ObjectWrap<NapiTrayItem> {
    public:
        static Napi::Object Init(Napi::Env env, Napi::Object exports){
            Napi::Function func =
                DefineClass(env, "TrayItem", {
                    InstanceMethod("click", &NapiTrayItem::Click),
                    InstanceMethod("toString", &NapiTrayItem::ToString),
                    InstanceAccessor("text", &NapiTrayItem::GetText, &NapiTrayItem::SetText),
                    InstanceAccessor("checked", &NapiTrayItem::GetChecked, &NapiTrayItem::SetChecked),
                    InstanceAccessor("disabled", &NapiTrayItem::GetDisabled, &NapiTrayItem::SetDisabled),
                    InstanceAccessor("callback", &NapiTrayItem::GetCallback, &NapiTrayItem::SetCallback),
                    InstanceAccessor("submenu", &NapiTrayItem::GetSubmenu, &NapiTrayItem::SetSubmenu)});
            
            NapiTrayItemConstructor = new Napi::FunctionReference(); 
            *NapiTrayItemConstructor = Napi::Persistent(func);
            env.SetInstanceData(NapiTrayItemConstructor);

            return func;
        }

        NapiTrayItem(const Napi::CallbackInfo& info, const Napi::Value& arg) : Napi::ObjectWrap<NapiTrayItem>(info){
            Napi::Env env = info.Env();
            
            Napi::Object obj = info[0].As<Napi::Object>();
            if( info[0].IsString() ){
                obj = Napi::Object::New(env);
                obj.Set("text", info[0].As<Napi::String>());
            }
            else if( !info[0].IsObject() ){
                THROW("use new MenuItem(string) or new MenuItem(object)");
                return;
            }
            SetText(info, obj.Get("text"));
            SetChecked(info, obj.Get("checked"));
            SetDisabled(info, obj.Get("disabled"));
            SetCallback(info, obj.Get("callback"));
            SetSubmenu(info, obj.Get("submenu"));
        }

        NapiTrayItem(const Napi::CallbackInfo& info) : NapiTrayItem(info, info[0]) {}

        Napi::Value GetText (const Napi::CallbackInfo& info){
            return text;
        }
        void SetText(const Napi::CallbackInfo& info, const Napi::Value& arg){
            Napi::Env env = info.Env();
            if( !arg.IsString() ){
                THROW("'text' property must be a string");
                return;
            }
            text = arg.As<Napi::String>();
        }
        Napi::Value GetChecked (const Napi::CallbackInfo& info){
            return checked;
        }
        void SetChecked(const Napi::CallbackInfo& info, const Napi::Value& arg){
            if( arg.IsUndefined() || arg.IsNull() ){
                checked = Napi::Boolean::New(info.Env(), false);
                return;
            }
            checked = arg.ToBoolean();
        }
        Napi::Value GetDisabled (const Napi::CallbackInfo& info){
            return disabled;
        }
        void SetDisabled(const Napi::CallbackInfo& info, const Napi::Value& arg){
            if( arg.IsUndefined() || arg.IsNull() ){
                disabled = Napi::Boolean::New(info.Env(), false);
                return;
            }
            disabled = arg.ToBoolean();
        }
        Napi::Value GetCallback (const Napi::CallbackInfo& info){
            return callback.Value();
        }
        void SetCallback(const Napi::CallbackInfo& info, const Napi::Value& arg){
            if( !arg.IsFunction() ){
                Napi::Function f = Napi::Function();
                callback = Napi::Persistent( f );
            }
            else{
                callback = Napi::Persistent( arg.As<Napi::Function>() );
            }
        }
        Napi::Value GetSubmenu(const Napi::CallbackInfo& info){
            return submenupointer.Value();
        }
        void SetSubmenu(const Napi::CallbackInfo& info, const Napi::Value& arg){
            Napi::Array submenu;
            if( !arg.IsArray() ){
                submenu = Napi::Array::New(info.Env(), 0);
            }
            else{
                Napi::Array array = arg.As<Napi::Array>();
                submenu = Napi::Array::New(info.Env(), array.Length());
                for(uint32_t i = 0; i < array.Length(); i++){
                    Napi::Value arg = array.Get(i);
                    if( arg.IsObject() && arg.As<Napi::Object>().InstanceOf(NapiTrayItemConstructor->Value()) ){
                        submenu[i] = arg;
                    }
                    else{
                        submenu[i] = NapiTrayItemConstructor->New({ array.Get(i) });
                    }
                }
            }
            submenupointer = Napi::Persistent(submenu);
        }

        void Click(const Napi::CallbackInfo& info){
            if( !callback.IsEmpty() ){
                callback.MakeCallback(this->Value(), {}, nullptr);
            }
        }

        Napi::Value ToString(const Napi::CallbackInfo& info){
            return text;
        }

    protected:
        Napi::String text;
        Napi::Boolean checked;
        Napi::Boolean disabled;
        Napi::Reference<Napi::Array> submenupointer;
        Napi::FunctionReference callback;
};

#endif