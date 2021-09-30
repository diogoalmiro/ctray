#ifndef _NAPITRAY_H
#define _NAPITRAY_H

#include <napi.h>
#include "napitrayitem.h"

#define THROW(error) Napi::TypeError::New( env, error ).ThrowAsJavaScriptException()

template<typename T>
class NapiTray : public Napi::ObjectWrap<T> {
    public:
        static Napi::Object Init(Napi::Env env, Napi::Object exports){
            Napi::Function func =
                NapiTray::DefineClass(env, "Tray", {
                    NapiTray::InstanceMethod("start", &T::Start),
                    NapiTray::InstanceMethod("update", &T::Update),
                    NapiTray::InstanceMethod("stop", &T::Stop),
                    NapiTray::InstanceAccessor("icon", &T::GetIcon, &T::SetIcon),
                    NapiTray::InstanceAccessor("menu", &T::GetMenu, &T::SetMenu)});
            func.Set("MenuItem", NapiTrayItem::Init(env, exports));
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
            icon = arg.As<Napi::String>().Utf8Value();
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
            Napi::Array menu = Napi::Array::New(env, arr.Length());
            Napi::Function constructor = info.Env().GetInstanceData<Napi::FunctionReference>()->Value().Get("MenuItem").As<Napi::Function>();
            for(uint32_t i = 0; i < arr.Length(); i++){
                Napi::Value arg = arr.Get(i);
                if( arg.IsObject() && arg.As<Napi::Object>().InstanceOf(constructor) ){
                    menu[i] = arg;
                }
                else{
                    menu[i] = constructor.New({ arg });
                }
            }
            menuref = Napi::Persistent(menu);
        }

        Napi::Value GetMenu(const Napi::CallbackInfo& info){
            return menuref.Value();
        }

        std::string icon;
        Napi::Reference<Napi::Array> menuref;
};

void* PrepareMenu(Napi::Array currMenu, void *origin);

#endif