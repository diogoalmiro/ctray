#ifndef _TRAYLOOP_H
#define _TRAYLOOP_H

#include <napi.h>
#include "napitray.h"

template<typename T, typename = std::enable_if<std::is_base_of<NapiTray<T>, T>::value>>
class TrayLoop : public Napi::AsyncWorker {
    public:
        TrayLoop(Napi::Env env, T *ctx) : 
            Napi::AsyncWorker(env),
            ctx(ctx),
            deferred(Napi::Promise::Deferred::New(env)) {}

        void Execute(){
            try{
                ctx->Loop();
            }
            catch(std::exception const& e){
                SetError(e.what());
            }
        }

        void OnOK(){
            deferred.Resolve(Env().Undefined());
        }

        void OnError(Napi::Error const& error){
            deferred.Reject( error.Value() );
        }

        Napi::Promise GetPromise(){
            return deferred.Promise();
        }

    private:
        T *ctx;
        Napi::Promise::Deferred deferred;
};

#endif