#include "napitray.h"

#include <windows.h>
#include <shellapi.h>
#include <strsafe.h>

#define WM_TRAY_CALLBACK_MESSAGE (WM_APP + 1)
#define WINDCLASSNAME "System Tray"
#define ID_TRAY_FIRST 1000

typedef struct {
    UINT idx;
    void *origin;
} origin_t;

struct callback_info{
    origin_t *origin;
    NapiTrayItem *clicked;
};

std::mutex mtx;

int i = 0;
class Tray : public NapiTray<Tray>{
    public:
        static void TrayThread(Tray *t){
            // window must be created on the thread to recieve messages on a thread different to NodeJS loop
            t->hwnd = CreateWindow(WINDCLASSNAME, NULL, 0, 0, 0, 0, 0, 0, 0, 0, t);
            if (t->hwnd == NULL){
                fprintf(stderr, "CreateWindow failed. Error: 0x%x\n", GetLastError());
                return;
            }
            UpdateWindow(t->hwnd);

            MSG msg;
            while( GetMessage(&msg, t->hwnd, 0, 0) ){
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        static void OnTrayClose(const Napi::CallbackInfo& info){
            Tray* t = (Tray*) info.Data();
            Shell_NotifyIcon(NIM_DELETE, &t->nid);
            if(t->nid.hIcon != 0) {
                DestroyIcon(t->nid.hIcon);
            }
            if(t->hmenu != 0) {
                DestroyMenu(t->hmenu);
            }
            PostMessage(t->hwnd, WM_QUIT, 0, 0);
            t->trayThread.join();
        }
        static void OnTrayUpdate(const Napi::CallbackInfo& info){
            Tray* t = (Tray*) info.Data();
            if( info.Length() < 1 ){
                std::cerr << "OnTrayUpdate: Unknown update, without a argument.\n";
                return;
            }
            if( !info[0].IsString() ){
                std::cerr << "OnTrayUpdate: Unknown update, expected argument to be string. " << 
                    info[0].ToString().Utf8Value() << "recieved.\n";
                return;
            }

            std::string updated = info[0].As<Napi::String>().Utf8Value();
            if( updated.compare("menu") == 0 ){
                HMENU prev = t->hmenu;
                t->origin.idx = 0;
                t->hmenu = (HMENU) PrepareMenu(info, t->menuref.Value(), &t->origin);
                PostMessage(t->hwnd, WM_INITMENUPOPUP, (WPARAM) t->hmenu, 0);
                DestroyMenu(prev);
            }
            else if( updated.compare("icon") == 0 ){
                HICON hicon;
                ExtractIconEx(t->icon.c_str(), 0, NULL, &hicon, 1);
                if (t->nid.hIcon) {
                    DestroyIcon(t->nid.hIcon);
                }
                t->nid.hIcon = hicon;
                Shell_NotifyIcon(NIM_MODIFY, &t->nid);
            }
            else if( updated.compare("tooltip") == 0 ){
                StringCchCopy(t->nid.szTip, ARRAYSIZE(t->nid.szTip), t->tooltip.c_str());
                Shell_NotifyIcon(NIM_MODIFY, &t->nid);
            }
            
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

            // What a mess :') (But a working mess :D)
            NapiTrayItem *item = (NapiTrayItem*) info.Data();
            MENUITEMINFO *iteminfo = (MENUITEMINFO*) item->itemPointer;
            struct callback_info *c = (struct callback_info*) iteminfo->dwItemData;
            Tray *t = (Tray*) c->origin->origin;

            std::string updated = info[0].As<Napi::String>().Utf8Value();
            if( updated.compare("text") == 0 ){
                if( iteminfo->fType & MFT_SEPARATOR ){
                    item->text = "-";
                }
                else{
                    iteminfo->dwTypeData = (LPTSTR) item->text.c_str();
                    SetMenuItemInfo(t->hmenu, iteminfo->wID, FALSE, iteminfo);
                }
            }
            else if( updated.compare("checked") == 0 ){
                CheckMenuItem(t->hmenu, iteminfo->wID, item->checked ? MF_CHECKED : MF_UNCHECKED);
            }
            else if( updated.compare("disabled") == 0 ){
                EnableMenuItem(t->hmenu, iteminfo->wID, item->disabled ? MF_DISABLED : MF_ENABLED);
            }
            else if( updated.compare("submenu") == 0 ){
                if( iteminfo->fMask & MIIM_SUBMENU ){
                    HMENU prev = iteminfo->hSubMenu;
                    iteminfo->hSubMenu = (HMENU) PrepareMenu(info, item->submenupointer.Value(), c->origin);
                    SetMenuItemInfo(t->hmenu, iteminfo->wID, FALSE, iteminfo);
                    DestroyMenu(prev);
                }
                else{
                    item->submenupointer = Napi::Persistent(Napi::Array::New(info.Env(), 0));
                }
            }
        }
        
        Tray(const Napi::CallbackInfo& info) : NapiTray<Tray>(info) {
            mtx.lock();
            sprintf(id, "tray-indicator-id-%04d", i);
            i++;
            mtx.unlock();
            origin.idx = 0;
            origin.origin = this;
            trayThread = std::thread(TrayThread, this);
        }

        char id[80] = "tray-indicator-id-nnnn";
        HWND hwnd;
        HMENU hmenu;
        NOTIFYICONDATA nid;
        origin_t origin;
        std::thread trayThread;
};

void* PrepareMenu(const Napi::CallbackInfo& info, Napi::Array currMenu, void* origin){
    HMENU hmenu = CreatePopupMenu();
    
    origin_t *originT = (origin_t*) origin;
    for(uint32_t i = 0; i < currMenu.Length(); i++){
        MENUITEMINFO *item = (MENUITEMINFO*) NapiTrayItem::Unwrap(currMenu.Get(i).As<Napi::Object>())->PrepareItem(info, origin);
        if(!InsertMenuItem(hmenu, item->wID, FALSE, item)){
            fprintf(stderr, "Error Inserting In Menu %s\n", GetLastError());
        }
    }
    return hmenu;
}

void *NapiTrayItem::PrepareItem(const Napi::CallbackInfo& info, void *origin){
    struct callback_info *c = (struct callback_info*) malloc(sizeof(struct callback_info));
    c->clicked = this;
    c->origin = (origin_t*) origin;

    MENUITEMINFO *item = (MENUITEMINFO*) calloc(1, sizeof(MENUITEMINFO));
    this->itemPointer = item;
    item[0].cbSize = sizeof(MENUITEMINFO);
    item[0].fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE | MIIM_DATA;
    item[0].fType = 0;
    item[0].fState = 0;
    item[0].wID = ++c->origin->idx;
    if( text.compare("-") == 0 ){
        item[0].fType = MFT_SEPARATOR;
    }
    else{
        item[0].dwTypeData = (LPTSTR) text.c_str();

        Napi::Array menu = submenupointer.Value().As<Napi::Array>();
        if( menu.Length() > 0 ){
            item[0].fMask |= MIIM_SUBMENU;
            item[0].hSubMenu = (HMENU) PrepareMenu(info, menu, origin);
        }
        if( checked ){
            item[0].fState |= MFS_CHECKED;
        }
        if( disabled ){
            item[0].fState |= MFS_DISABLED;
        }
    }
    item[0].dwItemData = (ULONG_PTR) c;

    return &item[0];
}

UINT TaskbarRestart;

LRESULT CALLBACK winproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
    Tray *t;
    //printf("MSG 0x%04x THREAD %d\n", msg, std::this_thread::get_id());
    switch (msg){
        case WM_NCCREATE: {
            t = (Tray*) ((CREATESTRUCT*) lparam)->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) ((CREATESTRUCT*)lparam)->lpCreateParams);

            memset(&t->nid, 0, sizeof(NOTIFYICONDATA));
            t->nid.cbSize = sizeof(NOTIFYICONDATA);
            t->nid.hWnd = hwnd;
            t->nid.uID = 0;
            t->nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
            t->nid.uCallbackMessage = WM_TRAY_CALLBACK_MESSAGE;
            
            HICON hicon;
            ExtractIconEx(t->icon.c_str(), 0, NULL, &hicon, 1);
            t->nid.hIcon = hicon;

            StringCchCopy(t->nid.szTip, ARRAYSIZE(t->nid.szTip), t->tooltip.c_str());            
            Shell_NotifyIcon(NIM_ADD, &t->nid);
        }
        case WM_TRAY_CALLBACK_MESSAGE: {
            t = (Tray*) GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if (lparam == WM_LBUTTONUP || lparam == WM_RBUTTONUP) {
                POINT p;
                GetCursorPos(&p);
                SetForegroundWindow(hwnd);
                SendMessage(t->hwnd, WM_INITMENUPOPUP, (WPARAM) t->hmenu, 0);
                WORD cmd = TrackPopupMenu(t->hmenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
                                        p.x, p.y, 0, hwnd, NULL);
                
                if( cmd ){
                    MENUITEMINFO item;
                    memset(&item, 0, sizeof(MENUITEMINFO));
                    item.cbSize = sizeof(MENUITEMINFO);
                    item.fMask = MIIM_ID | MIIM_DATA;
                    if(GetMenuItemInfo(t->hmenu, cmd, FALSE, &item)){
                        struct callback_info* c = (struct callback_info*) item.dwItemData;
                        t->Click(c->clicked);
                    }
                }
                return 0;
            }
            default: {
                t = (Tray*) GetWindowLongPtr(hwnd, GWLP_USERDATA);
                if( msg == TaskbarRestart ){
                    Shell_NotifyIcon(NIM_ADD, &t->nid);
                }
            }
        }
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    WNDCLASSEX *wc = (WNDCLASSEX*) calloc(1, sizeof(WNDCLASSEX));
    wc->cbSize = sizeof(WNDCLASSEX);
    wc->hInstance = GetModuleHandle(NULL);
    wc->lpszClassName = WINDCLASSNAME;
    wc->lpfnWndProc = winproc;
    if (!RegisterClassEx(wc)) {
        THROW("Unable to register windows class.");
        return env.Null().As<Napi::Object>();
    }
    TaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));

    return Tray::Init(env, exports);
}

NODE_API_MODULE(addon, Init)