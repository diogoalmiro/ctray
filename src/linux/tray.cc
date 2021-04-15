#include "napitray.h"

#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>
// See https://wiki.ubuntu.com/DesktopExperienceTeam/ApplicationIndicators

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


class Tray : public NapiTray<Tray> {
    public:
        /*static Napi::Object Init(Napi::Env env, Napi::Object exports){
            Napi::Function func =
                DefineClass(env, "Tray", {
                    InstanceMethod<&Tray::Start>("start"),
                    InstanceMethod<&Tray::Update>("update"),
                    InstanceMethod<&Tray::Stop>("stop"),
                    InstanceAccessor<&Tray::GetIcon, &Tray::SetIcon>("icon", napi_enumerable),
                    InstanceAccessor<&Tray::GetMenu, &Tray::SetMenu>("menu", napi_enumerable)});
            
            Napi::FunctionReference* constructor = new Napi::FunctionReference();
            *constructor = Napi::Persistent(func);
            env.SetInstanceData(constructor);

            return func;
        }*/

        Tray(const Napi::CallbackInfo& info) : NapiTray<Tray>(info) {}

        /*Napi::Value Start(const Napi::CallbackInfo& info) override {
            Napi::Env env = info.Env();
            if( gtk_init_check(0, NULL) == FALSE ){
                THROW("gtk_init_ceck Fail");
                return env.Undefined();
            }
            this->indicator = app_indicator_new(this->tray_application_id, this->icon, APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
            app_indicator_set_status(this->indicator, APP_INDICATOR_STATUS_ACTIVE);
            //this->Update();
            return env.Undefined();
        }

        /*Napi::Value Update(const Napi::CallbackInfo& info) override{
            Napi::Env env = info.Env();
            app_indicator_set_icon(this->indicator, this->icon);
            app_indicator_set_menu(this->indicator, GTK_MENU(_tray_menu(this->menu)));
            return env.Undefined();
        }

        Napi::Value Stop(const Napi::CallbackInfo& info) override{
            return info[0].Env().Undefined();
        }*/  

    private:
        char tray_application_id[80] = "tray-linux-id";
        AppIndicator *indicator = NULL;
        int loop_result = 0;
};



Napi::Object Init(Napi::Env env, Napi::Object exports) {
  return Tray::Init(env, exports);
}

NODE_API_MODULE(addon, Init)