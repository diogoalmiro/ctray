#include "napitray.h"
#include "trayloop.h"

#include <windows.h>
#include <shellapi.h>

#define WM_TRAY_CALLBACK_MESSAGE (WM_USER + 1)
#define ID_TRAY_FIRST 1000

LRESULT CALLBACK _tray_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam,
                                       LPARAM lparam) {
  switch (msg) {
  case WM_CLOSE:
    DestroyWindow(hwnd);
    return 0;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  case WM_TRAY_CALLBACK_MESSAGE:
    if (lparam == WM_LBUTTONUP || lparam == WM_RBUTTONUP) {
      POINT p;
      GetCursorPos(&p);
      SetForegroundWindow(hwnd);
      HMENU *hmenu = (HMENU*)GetWindowLongPtr(hwnd, 0);
      WORD cmd = TrackPopupMenu(*hmenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON |
                                          TPM_RETURNCMD | TPM_NONOTIFY,
                                p.x, p.y, 0, hwnd, NULL);
      SendMessage(hwnd, WM_COMMAND, cmd, 0);
      return 0;
    }
    break;
  case WM_COMMAND:
    if (wparam >= ID_TRAY_FIRST) {
      MENUITEMINFO item;
      item.cbSize = sizeof(MENUITEMINFO);
      item.fMask = MIIM_ID | MIIM_DATA;
      HMENU *hmenu = (HMENU*)GetWindowLongPtr(hwnd, 0);
      if (GetMenuItemInfo(*hmenu, wparam, FALSE, &item)) {
        struct tray_menu *menu = (struct tray_menu *)item.dwItemData;
        if (menu != NULL && menu->cb != NULL) {
          menu->cb(menu);
        }
      }
      return 0;
    }
    break;
  }
  return DefWindowProc(hwnd, msg, wparam, lparam);
}

static HMENU _tray_menu(struct tray_menu *m, UINT *id) {
  HMENU hmenu = CreatePopupMenu();
  for (; m != NULL && m->text != NULL; m++, (*id)++) {
    if (strcmp(m->text, "-") == 0) {
      InsertMenu(hmenu, *id, MF_SEPARATOR, TRUE, "");
    } else {
      MENUITEMINFO item;
      memset(&item, 0, sizeof(item));
      item.cbSize = sizeof(MENUITEMINFO);
      item.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE | MIIM_DATA;
      item.fType = 0;
      item.fState = 0;
      if (m->submenu != NULL) {
        item.fMask = item.fMask | MIIM_SUBMENU;
        item.hSubMenu = _tray_menu(m->submenu, id);
      }
      if (m->disabled) {
        item.fState |= MFS_DISABLED;
      }
      if (m->checked) {
        item.fState |= MFS_CHECKED;
      }
      item.wID = *id;
      item.dwTypeData = m->text;
      item.dwItemData = (ULONG_PTR)m;

      InsertMenuItem(hmenu, *id, TRUE, &item);
    }
  }
  return hmenu;
}

int i = 0;
class Tray : public NapiTray<Tray> {
    public:
        Tray(const Napi::CallbackInfo& info) : NapiTray<Tray>(info) {
          sprintf(tray_application_id, "tray-win-id-%d", i++);
        }

        Napi::Value Start(const Napi::CallbackInfo& info) override {
            Napi::Env env = info.Env();
            TrayLoop<Tray> *loop = new TrayLoop<Tray>(env, this);
            loop->Queue();
            return loop->GetPromise();
        }

        Napi::Value Update(const Napi::CallbackInfo& info) override{
            Napi::Env env = info.Env();
            HMENU prevmenu = hmenu;
            UINT id = ID_TRAY_FIRST;
            hmenu = _tray_menu(menu, &id);
            SetWindowLongPtr(hwnd,0, (LONG_PTR)&hmenu);
            SendMessage(hwnd, WM_INITMENUPOPUP, (WPARAM)hmenu, 0);
            HICON icon;
            ExtractIconEx(this->icon, 0, NULL, &icon, 1);
            if (nid.hIcon) {
                DestroyIcon(nid.hIcon);
            }
            nid.hIcon = icon;
            Shell_NotifyIcon(NIM_MODIFY, &nid);

            if (prevmenu != NULL) {
                DestroyMenu(prevmenu);
            }
            return env.Undefined();
        }

        Napi::Value Stop(const Napi::CallbackInfo& info) override{
            Shell_NotifyIcon(NIM_DELETE, &nid);
            if (nid.hIcon != 0) {
              DestroyIcon(nid.hIcon);
            }
            if (hmenu != 0) {
              DestroyMenu(hmenu);
            }
            PostQuitMessage(0);
            UnregisterClass(tray_application_id, GetModuleHandle(NULL));
            return info[0].Env().Undefined();
        }

        void Loop() override{
            // INIT
            memset(&wc, 0, sizeof(wc));
            wc.cbSize = sizeof(WNDCLASSEX);
            wc.lpfnWndProc = _tray_wnd_proc;
            wc.hInstance = GetModuleHandle(NULL);
            wc.lpszClassName = tray_application_id;
            wc.cbWndExtra = sizeof(HMENU*);
            if (!RegisterClassEx(&wc)) {
                return;
            }

            hwnd = CreateWindowEx(0, tray_application_id, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0);
            if (hwnd == NULL) {
                return;
            }
            SetWindowLongPtr(hwnd,0, (LONG_PTR)&hmenu);

            UpdateWindow(hwnd);

            memset(&nid, 0, sizeof(nid));
            nid.cbSize = sizeof(NOTIFYICONDATA);
            nid.hWnd = hwnd;
            nid.uID = 0;
            nid.uFlags = NIF_ICON | NIF_MESSAGE;
            nid.uCallbackMessage = WM_TRAY_CALLBACK_MESSAGE;
            Shell_NotifyIcon(NIM_ADD, &nid);
            // First up
            HMENU prevmenu = hmenu;
            UINT id = ID_TRAY_FIRST;
            hmenu = _tray_menu(menu, &id);
            SendMessage(hwnd, WM_INITMENUPOPUP, (WPARAM)hmenu, 0);
            HICON icon;
            ExtractIconEx(this->icon, 0, NULL, &icon, 1);
            if (nid.hIcon) {
              DestroyIcon(nid.hIcon);
            }
            nid.hIcon = icon;
            Shell_NotifyIcon(NIM_MODIFY, &nid);

            if (prevmenu != NULL) {
              DestroyMenu(prevmenu);
            }

            // LOOP
            while(true){
                MSG msg;
                GetMessage(&msg, NULL, 0, 0);
                if (msg.message == WM_QUIT) {
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

        }

    private:
        WNDCLASSEX wc;
        NOTIFYICONDATA nid;
        char tray_application_id[80] = "tray-linux-id";
        HWND hwnd;
        HMENU hmenu = NULL;
};



Napi::Object Init(Napi::Env env, Napi::Object exports) {
  return Tray::Init(env, exports);
}

NODE_API_MODULE(addon, Init)