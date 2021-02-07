#include "pch.h"
#include "resource.h"
#include "Marionette.h"



class OshiroAssistApp
{
public:
    static OshiroAssistApp& instance();

    void start();
    void finish();
    bool exit();

    // internal
    bool onInput(mr::OpRecord& rec);

public:
    OshiroAssistApp();
    ~OshiroAssistApp();
    OshiroAssistApp(const OshiroAssistApp&) = delete;

    HWND m_hwnd = nullptr;
    bool m_finished = false;

    std::map<mr::Key, mr::IPlayerPtr> m_keymap;
};


static void HandleClientAreaDrag(HWND hwnd, UINT msg, int mouseX, int mouseY)
{
    static int s_captureX = 0;
    static int s_captureY = 0;

    switch (msg)
    {
    case WM_LBUTTONDOWN:
        s_captureX = mouseX;
        s_captureY = mouseY;
        ::SetCapture(hwnd);
        break;

    case WM_LBUTTONUP:
        ::ReleaseCapture();
        break;

    case WM_MOUSEMOVE:
        if (::GetCapture() == hwnd)
        {
            RECT rc;
            ::GetWindowRect(hwnd, &rc);
            int  newX = rc.left + mouseX - s_captureX;
            int  newY = rc.top + mouseY - s_captureY;
            int  width = rc.right - rc.left;
            int  height = rc.bottom - rc.top;
            UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
            ::SetWindowPos(hwnd, NULL, newX, newY, width, height, flags);
        }
        break;
    }
}

static INT_PTR CALLBACK mrDialogCB(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto GetApp = [hDlg]() -> OshiroAssistApp& {
        return OshiroAssistApp::instance();
    };
    auto CtrlSetText = [hDlg](int cid, const wchar_t* v) {
        ::SetDlgItemTextW(hDlg, cid, v);
    };
    auto CtrlEnable = [hDlg](int cid, bool v) {
        ::EnableWindow(GetDlgItem(hDlg, cid), v);
    };

    INT_PTR ret = FALSE;
    switch (msg) {
    case WM_INITDIALOG:
    {
        // .rc file can not handle unicode. non-ascii characters must be set from program.
        // https://social.msdn.microsoft.com/Forums/ja-JP/fa09ec19-0253-478b-849f-9ae2980a3251
        ::SetWindowTextW(hDlg, L"御城アシスト");
        CtrlSetText(IDC_DESCRIPTION,
            L"Space: 停止/再開\n"
            L"X: 倍速\n"
            L"U: 巨大化 (城娘選択時)\n"
            L"R: 撤退 (城娘選択時)\n"
        );
        ::ShowWindow(hDlg, SW_SHOW);
        ret = TRUE;
        break;
    }

    case WM_CLOSE:
    {
        ::DestroyWindow(hDlg);
        ret = TRUE;

        GetApp().finish();
        break;
    }

    // handle drag & drop and move window
    case WM_MOUSEMOVE:
    {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        HandleClientAreaDrag(hDlg, WM_MOUSEMOVE, x, y);
        break;
    }
    case WM_LBUTTONUP:
    {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        HandleClientAreaDrag(hDlg, WM_LBUTTONUP, x, y);
        break;
    }
    case WM_LBUTTONDOWN:
    {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        HandleClientAreaDrag(hDlg, WM_LBUTTONDOWN, x, y);
        break;
    }

    default:
        break;
    }
    return ret;
}

OshiroAssistApp& OshiroAssistApp::instance()
{
    static OshiroAssistApp s_instance;
    return s_instance;
}

void OshiroAssistApp::start()
{
    mr::LoadKeymap("keymap.txt", [this](mr::Key k, std::string path) {
        auto player = mr::CreatePlayer();
        if (player->load(path.c_str())) {
            player->setMatchTarget(mr::MatchTarget::ForegroundWindow);
            m_keymap[k] = player;
        }
        mrDbgPrint("%d %s\n", k.code, path.c_str());
        });

    auto receiver = mr::GetReceiver();
    receiver->addHandler([this](mr::OpRecord& rec) { return onInput(rec); });

    m_hwnd = ::CreateDialogParam(::GetModuleHandle(nullptr), MAKEINTRESOURCE(IDD_MAINWINDOW), nullptr, mrDialogCB, (LPARAM)this);
    //m_hwnd = ::CreateDialogParam(::GetModuleHandle(nullptr), MAKEINTRESOURCE(IDD_DIALOG1), nullptr, mrDialogCB, (LPARAM)this);

    MSG msg;
    for (;;) {
        while (::PeekMessage(&msg, m_hwnd, 0, 0, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }

        receiver->update();
        for (auto& kvp : m_keymap)
            kvp.second->update();

        mr::SleepMS(1);
        if (m_finished)
            break;
    }
}

OshiroAssistApp::OshiroAssistApp()
{
}

OshiroAssistApp::~OshiroAssistApp()
{
}

void OshiroAssistApp::finish()
{
    m_finished = true;
}

bool OshiroAssistApp::exit()
{
    if (m_hwnd) {
        ::SendMessage(m_hwnd, WM_CLOSE, 0, 0);
        return true;
    }
    else {
        return false;
    }
}

bool OshiroAssistApp::onInput(mr::OpRecord& rec)
{
    static bool s_ctrl, s_alt, s_shift;
    if (rec.type == mr::OpType::KeyDown) {
        // stop if escape is pressed
        if (rec.data.key.code == VK_ESCAPE)
            exit();

        if (rec.data.key.code == VK_CONTROL)
            s_ctrl = true;
        if (rec.data.key.code == VK_MENU)
            s_alt = true;
        if (rec.data.key.code == VK_SHIFT)
            s_shift = true;

        mr::Key k{};
        k.ctrl = s_ctrl;
        k.alt = s_alt;
        k.shift = s_shift;
        k.code = rec.data.key.code;
        auto i = m_keymap.find(k);
        if (i != m_keymap.end())
            i->second->start();
    }
    if (rec.type == mr::OpType::KeyUp) {
        if (rec.data.key.code == VK_CONTROL)
            s_ctrl = false;
        if (rec.data.key.code == VK_MENU)
            s_alt = false;
        if (rec.data.key.code == VK_SHIFT)
            s_shift = false;

#ifdef mrDebug
        if (rec.data.key.code == VK_F9) {
            static bool s_dbg_flag = false;
            s_dbg_flag = !s_dbg_flag;
            mr::DbgSetScreenMatcherWriteout(s_dbg_flag);
        }
#endif
    }

    return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    auto bin_path = mr::GetCurrentModuleDirectory() + "\\bin";
    ::SetDllDirectoryA(bin_path.c_str());

    mr::InitializeScope mri;
    OshiroAssistApp::instance().start();
    return 0;
}
