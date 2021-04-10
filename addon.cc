#include <napi.h>
#include <thread>
#include "tray/tray.h"

#define THROW( error) Napi::TypeError::New( env, error ).ThrowAsJavaScriptException()

std::thread thread;

void TrayLoop(struct tray *tray, const Napi::Env env, const Napi::Promise::Deferred deferred){
    int r = 0;
    r = tray_init(tray);
    if( r == -1 ) deferred.Reject(Napi::String::New(env, "tray_init returned -1"));
    
    while(tray_loop(1) == 0){};

    tray_exit();
    
    deferred.Resolve(env.Null());
}

struct callbackContext {
    Napi::ThreadSafeFunction callback;
};

void callbackCall(struct tray_menu *tray_menu){
    auto callback = []( Napi::Env env, Napi::Function jsCallback) {
        jsCallback.Call({});
    };

    struct callbackContext* ctx = (callbackContext*) tray_menu->context;
    ctx->callback.BlockingCall( callback );
    
}
void NapiObject2struct(struct tray_menu* r, Napi::Env env, Napi::Object obj);

struct tray_menu* NapiArray2MenuStruct(Napi::Env env, Napi::Array napimenu){

    size_t msize = napimenu.Length();
    struct tray_menu* r = new struct tray_menu[msize+1];

    for (size_t i = 0; i < msize; i++){
        Napi::Value curr = napimenu.Get(i);
        if( !curr.IsObject() ){
            THROW("Expected all array elements to be Objects.");
            return NULL;
        }

        NapiObject2struct(&r[i], env, curr.As<Napi::Object>());
    }

    r[msize].text = NULL;
    r[msize].checked = 0;
    r[msize].disabled = 1;
    r[msize].cb = NULL;
    r[msize].submenu = NULL;
    r[msize].context = NULL;
    return r;
}

void NapiObject2struct(struct tray_menu* r, Napi::Env env, Napi::Object obj){
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
        r->submenu = NapiArray2MenuStruct(env, obj.Get("submenu").As<Napi::Array>());
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
        struct callbackContext* ctx = new callbackContext;
        ctx->callback = Napi::ThreadSafeFunction::New(
            env,
            obj.Get("callback").As<Napi::Function>(),
            text,
            0,
            1,
            []( Napi::Env ){});
        r->context = ctx;
        r->cb = &callbackCall;
    }
    else{
        r->context = NULL;
        r->cb = NULL;
    }
}


Napi::Value Setup(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
    struct tray *thistray = new (struct tray);

    if( info.Length() < 2 ){
        THROW("Expected two arguments, \"icon\": string and \"menu\": array.");
        return env.Null();
    }
    if( !info[0].IsString() ){
        THROW("Expected argument \"icon\" to be a string.");
        return env.Null();
    }
    if( !info[1].IsArray() ){
        THROW("Expected argument \"menu\" to be an array.");
        return env.Null();
    }


    Napi::String icon = info[0].As<Napi::String>();
    Napi::Array menu = info[1].As<Napi::Array>();

    std::string iname = icon.Utf8Value();
    thistray->icon = new char[iname.length()+1];
    strcpy(thistray->icon, iname.c_str());

    thistray->menu = NapiArray2MenuStruct(env, menu);
    
    thread = std::thread (TrayLoop,thistray, env, deferred);


    return deferred.Promise();
}


Napi::Object Export(Napi::Env env, Napi::Object exports){
    exports.Set("setup", Napi::Function::New<Setup>(env));
    return exports;
}

NODE_API_MODULE(addon, Export)