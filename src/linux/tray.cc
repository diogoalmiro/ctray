#include "napitray.h"

#include "trayloop.h"
#include <thread>
#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>

static GtkMenuShell *_tray_menu(tray_menu_t *m);

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

        Napi::Value Start(const Napi::CallbackInfo& info) override {
            TrayLoop<Tray> *loop = new TrayLoop<Tray>(info.Env(), this);
            indicator = app_indicator_new(id,icon,APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
            app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
            app_indicator_set_icon(indicator, icon);
            app_indicator_set_menu(indicator, GTK_MENU(_tray_menu(menu)));
            stop.lock();
            loop->Queue();
            return loop->GetPromise();
        }

        Napi::Value Update(const Napi::CallbackInfo& info) override{
            app_indicator_set_menu(indicator, GTK_MENU(_tray_menu(menu)));
            return info.Env().Undefined();
        }

        Napi::Value Stop(const Napi::CallbackInfo& info) override{
            app_indicator_set_status(indicator, APP_INDICATOR_STATUS_PASSIVE);
            ReleaseCallbacks();;
            stop.unlock();
            return info.Env().Undefined();
        }

        void Loop(){ // thread
            stop.lock();
            stop.unlock();
        }
    private:
        AppIndicator* indicator;
        std::mutex stop;
        char id[80] = "tray-indicator-id-nnnn";
};

static void _tray_menu_cb(GtkMenuItem *item, gpointer data) {
    (void)item;
    tray_menu_t *m = (tray_menu_t*)data;
    m->cb(m);
}

static GtkMenuShell *_tray_menu(tray_menu_t *m) {
    GtkMenuShell *menu = (GtkMenuShell *)gtk_menu_new();
    for (; m != NULL && m->text != NULL; m++) {
        GtkWidget *item;
        if (strcmp(m->text, "-") == 0) {
            item = gtk_separator_menu_item_new();
        } else {
            if (m->submenu != NULL) {
                item = gtk_menu_item_new_with_label(m->text);
                gtk_menu_item_set_submenu(GTK_MENU_ITEM(item),
                                        GTK_WIDGET(_tray_menu(m->submenu)));
            } else {
                item = gtk_check_menu_item_new_with_label(m->text);
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), !!m->checked);
            }
            gtk_widget_set_sensitive(item, !m->disabled);
            if (m->cb != NULL) {
                g_signal_connect(item, "activate", G_CALLBACK(_tray_menu_cb), m);
            }
        }
        gtk_widget_show(item);
        gtk_menu_shell_append(menu, item);
    }
    return menu;
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