#include "napitray.h"

#include "trayloop.h"
#include <thread>
#include <iostream>
#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>

std::mutex mtx;

int i = 0;
class Tray : public NapiTray<Tray> {
    public:
        Tray(const Napi::CallbackInfo& info) : NapiTray<Tray>(info) {
            mtx.lock();
            sprintf(id, "tray-indicator-id-%04d", i);
            i++;
            mtx.unlock();
        }

        Napi::Value Start(const Napi::CallbackInfo& info) {
            stop.lock();
            if( indicatorPath ){
                free(indicatorPath);
            }
            indicatorPath =(char*) calloc(icon.size()+1, sizeof(char));
            strcpy(indicatorPath, icon.c_str());
            
            indicator = app_indicator_new(id,indicatorPath,APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
            app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
            app_indicator_set_icon(indicator, indicatorPath);
            app_indicator_set_menu(indicator, GTK_MENU(PrepareMenu(menuref.Value(), this)));
            TrayLoop<Tray> *loop = new TrayLoop<Tray>(info.Env(), this);
            loop->Queue();
            return loop->GetPromise();
        }

        Napi::Value Update(const Napi::CallbackInfo& info){
            if( indicatorPath ){
                free(indicatorPath);
            }
            indicatorPath =(char*) calloc(icon.size()+1, sizeof(char));
            strcpy(indicatorPath, icon.c_str());
            app_indicator_set_menu(indicator, GTK_MENU(PrepareMenu(menuref.Value(), this)));
            return info.Env().Undefined();
        }

        Napi::Value Stop(const Napi::CallbackInfo& info){
            app_indicator_set_status(indicator, APP_INDICATOR_STATUS_PASSIVE);
            app_indicator_set_menu(indicator, GTK_MENU(PrepareMenu(Napi::Array::New(info.Env()), this)));
            stop.unlock();
            return info.Env().Undefined();
        }

        void Loop(){ // thread
            stop.lock();
            stop.unlock();
        }

    private:
        AppIndicator* indicator = nullptr;
        std::mutex stop;
        char id[80] = "tray-indicator-id-nnnn";
        char *indicatorPath = nullptr;
};

struct callback_info
{
    Tray *origin;
    NapiTrayItem *clicked;
};


void _tray_menu_cb(GtkMenuItem *item, gpointer data){
    struct callback_info* c = (struct callback_info*) data;
    c->clicked->Click();
}

void _tray_menu_destroy(GtkMenuItem *item, gpointer data){
    ((struct callback_info*)data)->clicked->Destroy();
}

void* PrepareMenu(Napi::Array currMenu, void *origin){
    GtkMenuShell *menu = (GtkMenuShell *)gtk_menu_new();
    for(uint32_t i = 0; i < currMenu.Length(); i++){
        GtkWidget *item = (GtkWidget*) NapiTrayItem::Unwrap(currMenu.Get(i).As<Napi::Object>())->PrepareItem(origin);
        gtk_menu_shell_append(menu, item);
    }
    return menu;
}

void* NapiTrayItem::PrepareItem(void *origin){
    char* label = (char*) calloc(text.size()+1, sizeof(char));
    struct callback_info *c = new struct callback_info();
    c->clicked = this;
    c->origin = (Tray*) origin;
    strcpy(label, text.c_str());
    GtkWidget *item = nullptr;
    if( strcmp(label, "-") == 0 ){
        item = gtk_separator_menu_item_new();
    }
    else{
        Napi::Array menu = submenupointer.Value().As<Napi::Array>();
        if( menu.Length() > 0 ){
            item = gtk_menu_item_new_with_label(label);
            gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), GTK_WIDGET(PrepareMenu(menu, origin)));
        }
        else{
            item = gtk_check_menu_item_new_with_label(label);
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), !!checked);   
        }
        gtk_widget_set_sensitive(item, !disabled);
        onClickCallback = Napi::ThreadSafeFunction::New(Env(), callback.Value(), "onClickCallback", 0, 1);
        g_signal_connect(item, "activate", G_CALLBACK(_tray_menu_cb), c);
        g_signal_connect(item, "destroy", G_CALLBACK(_tray_menu_destroy), c);
    }
    gtk_widget_show(item);
    return item;
}

std::thread gtkThread;

void QuitGtk(void *){
    gtk_main_quit();
    gtkThread.join();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    if( !gtk_init_check(0, NULL) ){
        Napi::Error::New(env, "GTK Init Check Failed").ThrowAsJavaScriptException();
        return env.Null().As<Napi::Object>();
    }

    gtkThread = std::thread(gtk_main);
    napi_add_env_cleanup_hook( env, &QuitGtk, nullptr);

    return Tray::Init(env, exports);
}

NODE_API_MODULE(addon, Init)