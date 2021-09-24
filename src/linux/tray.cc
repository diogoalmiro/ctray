#include "napitray.h"

#include "trayloop.h"
#include <thread>
#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>
// See https://wiki.ubuntu.com/DesktopExperienceTeam/ApplicationIndicators
// Reference https://stackoverflow.com/questions/27950493/safety-of-using-pthreads-in-gtk2-0-application#27990662

static gboolean start_tray(gpointer gtray);
static gboolean update_tray(gpointer gtray);
static gboolean stop_tray(gpointer gtray);

int i = 0;
class Tray : public NapiTray<Tray> {
    public:
        Tray(const Napi::CallbackInfo& info) : NapiTray<Tray>(info) {
            sprintf(tray_application_id, "tray-linux-id-%d", i++);
        }

        Napi::Value Start(const Napi::CallbackInfo& info) override {
            TrayLoop<Tray> *loop = new TrayLoop<Tray>(info.Env(), this);
            loop->Queue();
            return loop->GetPromise();
        }

        Napi::Value Update(const Napi::CallbackInfo& info) override{
            GSource *source = g_idle_source_new();
            g_source_set_callback(source, update_tray, this, NULL);
            g_source_attach(source, context);
            g_source_unref(source);
            ReleaseOldCallbacks();
            return info.Env().Undefined();
        }

        Napi::Value Stop(const Napi::CallbackInfo& info) override{
            GSource *source = g_idle_source_new();
            g_source_set_callback(source, stop_tray, this, NULL);
            g_source_attach(source, context);
            g_source_unref(source);
            ReleaseCallbacks();
            gtk_main_quit();
            return info.Env().Undefined();
        }

        GMainContext *context;
        void Loop(){
            if( !gtk_init_check(0, NULL) ){
                return;
            }
            context = g_main_context_default();
            GSource *source = g_idle_source_new();
            g_source_set_callback(source, start_tray, this, NULL);
            g_source_attach(source, context);
            g_source_unref(source);
            gtk_main();
        }

        char tray_application_id[80] = "tray-linux-id-nnn";
        AppIndicator *indicator = NULL;

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

static gboolean start_tray(gpointer gtray){
    Tray *tray = (Tray*)gtray;

    tray->indicator = app_indicator_new(tray->tray_application_id, tray->icon,
                            APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
    app_indicator_set_status(tray->indicator, APP_INDICATOR_STATUS_ACTIVE);
    app_indicator_set_icon(tray->indicator, tray->icon);
    app_indicator_set_menu(tray->indicator, GTK_MENU(_tray_menu(tray->menu)));
    
    return G_SOURCE_REMOVE;
}

static gboolean update_tray(gpointer gtray){
    Tray *tray = (Tray*)gtray;

    app_indicator_set_icon(tray->indicator, tray->icon);
    app_indicator_set_menu(tray->indicator, GTK_MENU(_tray_menu(tray->menu)));

    return G_SOURCE_REMOVE;
}

static gboolean stop_tray(gpointer gtray){
    Tray *tray = (Tray*)gtray;

    app_indicator_set_status(tray->indicator, APP_INDICATOR_STATUS_PASSIVE);

    return G_SOURCE_REMOVE;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  return Tray::Init(env, exports);
}

NODE_API_MODULE(addon, Init)