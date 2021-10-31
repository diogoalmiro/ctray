#ifndef _NAPITRAY_H
#define _NAPITRAY_H

#include <napi.h>
#include "napitrayitem.h"

#define THROW(error) Napi::TypeError::New( env, error ).ThrowAsJavaScriptException()

#define EMIT(event_name, ...) info.This().As<Napi::Object>().Get("emit").As<Napi::Function>().Call(info.This(), {Napi::String::New(info.Env(), event_name), __VA_ARGS__})

template<typename T>
class NapiTray : public Napi::ObjectWrap<T> {
    public:
        static Napi::Object Init(Napi::Env env, Napi::Object exports){
            Napi::Function func =
                NapiTray::DefineClass(env, "Tray", {
                    NapiTray::InstanceAccessor("menu", &T::GetMenu, &T::SetMenu),
                    NapiTray::InstanceAccessor("icon", &T::GetTooltip, &T::SetTooltip),
                    NapiTray::InstanceAccessor("tooltip", &T::GetIcon, &T::SetIcon)});

            // Add reference to TrayItem class
            func.Set("TrayItem", NapiTrayItem::Init(env, exports));
            
            // Tray extends EventEmitter
            Napi::Function Events = env.Global().As<Napi::Object>().Get("require").As<Napi::Function>().Call(env.Global(), {Napi::String::New(env, "events")}).As<Napi::Function>();
            Napi::Function SetPrototype = env.Global().Get("Object").As<Napi::Object>().Get("setPrototypeOf").As<Napi::Function>();
            
            SetPrototype.Call(env.Global(), {func.Get("prototype"), Events.Get("prototype")});


            Napi::FunctionReference* constructor = new Napi::FunctionReference();
            *constructor = Napi::Persistent(func);
            env.SetInstanceData(constructor);
            return func;
        }
        
        NapiTray(const Napi::CallbackInfo& info) : Napi::ObjectWrap<T>(info) {
            Napi::Env env = info.Env();
            if( info.Length() < 1 ){
                THROW("Expected argument \"icon\": string.");
                return;
            }
            menuref = Napi::Persistent<Napi::Array>(Napi::Array::New(env, 0));
            tooltip = "";
            icon = "";
            SetIcon(info, info[0].As<Napi::Value>());
        }

        void SetIcon(const Napi::CallbackInfo& info, const Napi::Value& arg){
            Napi::Env env = info.Env();
            if( !arg.IsString() ){
                THROW("Expected argument \"icon\" to be a string.");
                return;
            }
            icon = arg.As<Napi::String>().Utf8Value();
            EMIT("update", Napi::String::New(info.Env(), "icon"));
        }

        Napi::Value GetIcon(const Napi::CallbackInfo& info){
            return Napi::String::New(info.Env(), icon);
        }

        void SetTooltip(const Napi::CallbackInfo& info, const Napi::Value& arg){
            Napi::Env env = info.Env();
            if( !arg.IsString() ){
                THROW("Expected argument \"tooltip\" to be a string.");
                return;
            }
            tooltip = arg.As<Napi::String>().Utf8Value();
            EMIT("update", Napi::String::New(info.Env(), "tooltip"));
        }

        Napi::Value GetTooltip(const Napi::CallbackInfo& info){
            return Napi::String::New(info.Env(), tooltip);
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
            Napi::Function constructor = info.Env().GetInstanceData<Napi::FunctionReference>()->Value().Get("TrayItem").As<Napi::Function>();
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
            EMIT("update", Napi::String::New(env, "menu"));
        }

        Napi::Value GetMenu(const Napi::CallbackInfo& info){
            return menuref.Value();
        }

        std::string icon;
        std::string tooltip;
        Napi::Reference<Napi::Array> menuref;
};

void* PrepareMenu(Napi::Array currMenu, void *origin);

#endif