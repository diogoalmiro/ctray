#ifndef _NAPITRAY_H
#define _NAPITRAY_H

#include <napi.h>
#include "napitrayitem.h"

#define THROW(error) Napi::TypeError::New( env, error ).ThrowAsJavaScriptException()

#define EMIT(event_name, ...) info.This().As<Napi::Object>().Get("emit").As<Napi::Function>().Call(info.This(), {Napi::String::New(info.Env(), event_name), __VA_ARGS__})

template<typename T>
class NapiTray : public Napi::ObjectWrap<T> {
    public:
        static void OnClickCallBack(Napi::Env env, Napi::Function emitter, NapiTrayItem *data){
            Napi::Object TrayItemInstance = data->Value().As<Napi::Object>();
            emitter.Call(TrayItemInstance, {Napi::String::New(env, "click")});
        }

        static Napi::Object Init(Napi::Env env, Napi::Object exports){
            Napi::Function func =
                NapiTray::DefineClass(env, "Tray", {
                    NapiTray::InstanceMethod("close", &T::Close),
                    NapiTray::InstanceAccessor("menu", &T::GetMenu, &T::SetMenu),
                    NapiTray::InstanceAccessor("icon", &T::GetIcon, &T::SetIcon),
                    NapiTray::InstanceAccessor("tooltip", &T::GetTooltip, &T::SetTooltip)});
            
            Napi::Function EventEmitter = env.Global().Get("EventEmitter").As<Napi::Function>();
            if( EventEmitter.IsEmpty() ){
                THROW("Expected EventEmitter constructor on global object.");
                return exports;
            }
            // Tray extends EventEmitter
            func.Get("prototype").As<Napi::Object>().Set("__proto__", EventEmitter.Get("prototype"));
            
            // Add reference to TrayItem class
            NapiTrayItem::Init(env, exports);
            


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
            tooltip = "NapiTray Icon";
            icon = "";

            SetIcon(info, info[0].As<Napi::Value>());


            info.This().As<Napi::Object>()
                .Get("on").As<Napi::Function>()
                .Call(info.This(), {Napi::String::New(info.Env(), "update"), Napi::Function::New<T::OnTrayUpdate>(info.Env(), nullptr, this)});

            info.This().As<Napi::Object>()
                .Get("on").As<Napi::Function>()
                .Call(info.This(), {Napi::String::New(info.Env(), "close"), Napi::Function::New<T::OnTrayClose>(info.Env(), nullptr, this)});
            
            Napi::Function Emit = info.This().As<Napi::Object>().Get("emit").As<Napi::Function>();
            onClickCallback = Napi::ThreadSafeFunction::New(env, Emit, "emit-click", 0, 1);
        }

        Napi::Value Close(const Napi::CallbackInfo& info){
            Napi::Function Emit = info.This().As<Napi::Object>().Get("emit").As<Napi::Function>();
            Emit.Call(info.This(), {Napi::String::New(info.Env(), "close")});
            this->Destroy();
            return info.Env().Undefined();
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
    
            Napi::Function constructor = NapiTrayItemConstructor.Value();
            for(uint32_t i = 0; i < arr.Length(); i++){
                Napi::Value arg = arr.Get(i);
                if( arg.IsObject() && arg.As<Napi::Object>().InstanceOf(constructor) ){
                    menu[i] = arg;
                }
                else{
                    menu[i] = constructor.New({ arg });
                }

                menu.Get(i).As<Napi::Object>()
                    .Get("on").As<Napi::Function>()
                    .Call(menu.Get(i), {Napi::String::New(info.Env(), "update"), Napi::Function::New<T::OnTrayItemUpdate>(info.Env(), nullptr, NapiTrayItem::Unwrap(menu.Get(i).As<Napi::Object>()))});
            }
            menuref = Napi::Persistent(menu);
            EMIT("update", Napi::String::New(env, "menu"));
        }

        Napi::Value GetMenu(const Napi::CallbackInfo& info){
            Napi::Object res = Napi::Object::New(info.Env()); // Menu should not extend array;
            Napi::Array from = menuref.Value();
            for( uint32_t i = 0; i < from.Length(); i++){
                res.Set(i, from.Get(i));
            }
            return res;
        }

        void Click(NapiTrayItem *item){
            onClickCallback.BlockingCall( item, OnClickCallBack );
        }

        void Destroy(){
            onClickCallback.Release();
        }

        std::string icon;
        std::string tooltip;
        Napi::Reference<Napi::Array> menuref;

        Napi::ThreadSafeFunction onClickCallback;

        void* menuPointer = nullptr;
        
};

void* PrepareMenu(const Napi::CallbackInfo& info, Napi::Array currMenu, void*origin);

#endif