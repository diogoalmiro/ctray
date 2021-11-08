#include "napitray.h"

#include <thread>
#include <iostream>
#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>

int i = 0;
class Tray : public NapiTray<Tray> {
    public:

        static void OnTrayClose(const Napi::CallbackInfo& info){
            Tray *t = (Tray*) info.Data();
            app_indicator_set_menu(t->indicator, GTK_MENU(PrepareMenu(info, Napi::Array::New(info.Env(), 0), t)));
            app_indicator_set_status(t->indicator, APP_INDICATOR_STATUS_PASSIVE);
        }

        static void OnTrayItemUpdate(const Napi::CallbackInfo& info){
            if( info.Length() < 1 ){
                std::cerr << "Unknown update, OnTrayItemUpdate called without a argument.\n";
                return;
            }
            if( !info[0].IsString() ){
                std::cerr << "Unknown update, OnTrayItemUpdate expected argument to be string. " << 
                    info[0].ToString().Utf8Value() << "recieved.\n";
                return;
            }

            NapiTrayItem *item = (NapiTrayItem*) info.Data();
            Tray *t = (Tray*) g_object_get_data(G_OBJECT(item->itemPointer), "originPointer");

            std::string updated = info[0].As<Napi::String>().Utf8Value();
            bool separ = GTK_IS_SEPARATOR_MENU_ITEM(item->itemPointer);
            bool check = GTK_IS_CHECK_MENU_ITEM(item->itemPointer);
            bool menui = GTK_IS_MENU_ITEM(item->itemPointer);

            if( updated.compare("text") == 0 ){
                if( GTK_IS_SEPARATOR_MENU_ITEM(item->itemPointer) ){
                    item->text = "-";
                }
                else{
                    gtk_menu_item_set_label(GTK_MENU_ITEM(item->itemPointer), item->text.c_str());
                }
            }
            else if( updated.compare("checked") == 0 ){
                if( GTK_IS_CHECK_MENU_ITEM(item->itemPointer) ){
                    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item->itemPointer), !!item->checked);
                }
            }
            else if( updated.compare("disabled") == 0 ){
                gtk_widget_set_sensitive(GTK_WIDGET(item->itemPointer), !item->disabled);
            }
            else if( updated.compare("submenu") == 0 ){
                if( GTK_IS_MENU_ITEM(item->itemPointer) ){
                    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item->itemPointer), GTK_WIDGET(PrepareMenu(info, item->submenupointer.Value(), t)));
                }
                else{
                    item->submenupointer = Napi::Persistent(Napi::Array::New(info.Env(), 0));
                }
            }

            return;
        }

        static void OnTrayUpdate(const Napi::CallbackInfo& info){
            if( info.Length() < 1 ){
                std::cerr << "Unknown update, without a argument.\n";
                return;
            }
            if( !info[0].IsString() ){
                std::cerr << "Unknown update, expected argument to be string. " << 
                    info[0].ToString().Utf8Value() << "recieved.\n";
                return;
            }

            Tray *t = (Tray*) info.Data();
            if( !t->indicator ){
                return;
            }

            std::string updated = info[0].As<Napi::String>().Utf8Value();
            if( updated.compare("menu") == 0 ){
                app_indicator_set_menu(t->indicator, GTK_MENU(PrepareMenu(info, t->menuref.Value(), t)));
            }
            else if( updated.compare("icon") == 0 ){
                app_indicator_set_icon(t->indicator, t->icon.c_str());
            }
            else if( updated.compare("tooltip") == 0 ){
                app_indicator_set_title(t->indicator, t->tooltip.c_str());
            }
        }

        Tray(const Napi::CallbackInfo& info) : NapiTray<Tray>(info) {
            sprintf(id, "tray-indicator-id-%04d", i);
            i++;

            indicator = app_indicator_new(id,icon.c_str(),APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
            app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
            app_indicator_set_icon(indicator, icon.c_str());
            app_indicator_set_title(indicator, tooltip.c_str());
            app_indicator_set_menu(indicator, GTK_MENU(PrepareMenu(info, menuref.Value(), this)));
        }

    private:
        AppIndicator* indicator = nullptr;
        char id[80] = "tray-indicator-id-nnnn";
};

struct callback_info{
    Tray *origin;
    NapiTrayItem *clicked;
};


void _tray_menu_cb(GtkMenuItem *item, gpointer data){
    struct callback_info* c = (struct callback_info*) data;
    if( GTK_IS_CHECK_MENU_ITEM(item) ){
        c->clicked->checked = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(item));
    }
    c->origin->Click(c->clicked);
}


void* PrepareMenu(const Napi::CallbackInfo& info, Napi::Array currMenu, void *origin){
    GtkMenuShell *menu = (GtkMenuShell *)gtk_menu_new();
    for(uint32_t i = 0; i < currMenu.Length(); i++){
        GtkWidget *item = (GtkWidget*) NapiTrayItem::Unwrap(currMenu.Get(i).As<Napi::Object>())->PrepareItem(info, origin);
        gtk_menu_shell_append(menu, item);
    }
    ((Tray*) origin)->menuPointer = origin;
    return menu;
}

void* NapiTrayItem::PrepareItem(const Napi::CallbackInfo& info, void *origin){

    struct callback_info *c = new struct callback_info();
    c->clicked = this;
    c->origin = (Tray*) origin;

    GtkWidget *item;
    if( text.compare("-") == 0 ){
        item = gtk_separator_menu_item_new();
    }
    else{
        Napi::Array menu = submenupointer.Value().As<Napi::Array>();
        if( menu.Length() > 0 ){
            item = gtk_menu_item_new_with_label(text.c_str());
            gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), GTK_WIDGET(PrepareMenu(info, menu, c->origin)));
        }
        else{
            item = gtk_check_menu_item_new_with_label(text.c_str());
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), !!checked);   
            g_signal_connect(item, "activate", G_CALLBACK(_tray_menu_cb), c);
        }
        gtk_widget_set_sensitive(item, !disabled);
    }
    gtk_widget_show(item);
    g_object_set_data(G_OBJECT(item), "originPointer", origin);
    itemPointer = item;
    return item;
}

std::thread gtkThread;

void QuitGtk(void *){
    gtk_main_quit();
    gtkThread.join();
    NapiTrayItemConstructor.~Reference();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    if( !gtk_init_check(0, NULL) ){
        Napi::Error::New(env, "GTK Init Check Failed").ThrowAsJavaScriptException();
        return env.Null().As<Napi::Object>();
    }

    gtkThread = std::thread(gtk_main);

    napi_add_env_cleanup_hook(env, &QuitGtk, nullptr);

    return Tray::Init(env, exports);
}

NODE_API_MODULE(addon, Init)