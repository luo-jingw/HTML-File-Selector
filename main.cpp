#include <windows.h>
#include <shellapi.h>
#include <string>
#include <tchar.h>
#include "resource.h"
#include <uxtheme.h>
#include <vssym32.h>
#include <vector>
#include <filesystem>
#include <winreg.h>
#include <shlobj.h>  // 用于文件夹选择对话框
#include <gdiplus.h>  // 添加GDI+支持
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "gdiplus.lib")

// 检查文件是否存在
bool FileExists(LPCTSTR path) {
    DWORD attrs = GetFileAttributes(path);
    return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// 添加结构体存储HTML文件信息
struct HtmlFileInfo {
    std::wstring name;
    std::wstring path;
};

// 添加全局变量存储按钮句柄
HWND g_hwndButton1 = NULL;
HWND g_hwndButton2 = NULL;
HWND g_hwndButton3 = NULL;

// 添加全局变量
HBRUSH g_hBackgroundBrush = NULL;
HBRUSH g_hButtonBrush = NULL;
HBRUSH g_hButtonHoverBrush = NULL;
HWND g_hHoveredButton = NULL;
const int MIN_WINDOW_WIDTH = 200;
const int MIN_WINDOW_HEIGHT = 150;

// 更新颜色常量，使用更专业的深色主题配色
const COLORREF BACKGROUND_COLOR = RGB(18, 24, 38);      // 深邃蓝黑色背景
const COLORREF BUTTON_COLOR = RGB(45, 85, 255);        // 亮蓝色按钮
const COLORREF BUTTON_HOVER_COLOR = RGB(66, 99, 235);  // 高亮蓝色
const COLORREF BUTTON_BORDER = RGB(28, 34, 48);        // 按钮边框颜色
const COLORREF TEXT_COLOR = RGB(255, 255, 255);        // 纯白色文字

// 添加全局变量
std::vector<HWND> g_buttonList;  // 存储所有按钮句柄
const std::wstring HTML_FOLDER = L"E:\\guide_html";  // HTML文件夹路径

// 添加窗口设置相关常量和函数
const TCHAR REG_PATH[] = TEXT("Software\\HTMLSelector");
const TCHAR REG_WIDTH_NAME[] = TEXT("WindowWidth");
const TCHAR REG_HEIGHT_NAME[] = TEXT("WindowHeight");
const TCHAR REG_FOLDER_PATH[] = TEXT("HTMLFolderPath");

// 从注册表读取窗口大小
void LoadWindowSize(int& width, int& height) {
    HKEY hKey;
    width = 450;  // 默认宽度
    height = 300; // 默认高度
    
    if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_PATH, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD size = sizeof(DWORD);
        DWORD dwValue;
        
        if (RegQueryValueEx(hKey, REG_WIDTH_NAME, NULL, NULL, (LPBYTE)&dwValue, &size) == ERROR_SUCCESS) {
            width = dwValue;
        }
        if (RegQueryValueEx(hKey, REG_HEIGHT_NAME, NULL, NULL, (LPBYTE)&dwValue, &size) == ERROR_SUCCESS) {
            height = dwValue;
        }
        
        RegCloseKey(hKey);
    }
}

// 保存窗口大小到注册表
void SaveWindowSize(int width, int height) {
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, REG_PATH, 0, NULL, REG_OPTION_NON_VOLATILE,
                      KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueEx(hKey, REG_WIDTH_NAME, 0, REG_DWORD, (LPBYTE)&width, sizeof(DWORD));
        RegSetValueEx(hKey, REG_HEIGHT_NAME, 0, REG_DWORD, (LPBYTE)&height, sizeof(DWORD));
        RegCloseKey(hKey);
    }
}

// 从注册表读取文件夹路径
std::wstring LoadFolderPath() {
    HKEY hKey;
    TCHAR buffer[MAX_PATH] = {0};
    DWORD bufferSize = sizeof(buffer);
    
    if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_PATH, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueEx(hKey, REG_FOLDER_PATH, NULL, NULL, (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return std::wstring(buffer);
        }
        RegCloseKey(hKey);
    }
    return HTML_FOLDER; // 返回默认路径
}

// 保存文件夹路径到注册表
void SaveFolderPath(const std::wstring& path) {
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, REG_PATH, 0, NULL, REG_OPTION_NON_VOLATILE,
                      KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueEx(hKey, REG_FOLDER_PATH, 0, REG_SZ, 
                     (LPBYTE)path.c_str(), (path.length() + 1) * sizeof(TCHAR));
        RegCloseKey(hKey);
    }
}

// 显示文件夹选择对话框
bool ShowFolderDialog(HWND hwnd, std::wstring& selectedPath) {
    BROWSEINFO bi = {0};
    bi.hwndOwner = hwnd;
    bi.lpszTitle = TEXT("选择HTML文件所在文件夹");
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl != NULL) {
        TCHAR path[MAX_PATH];
        if (SHGetPathFromIDList(pidl, path)) {
            selectedPath = path;
            CoTaskMemFree(pidl);
            return true;
        }
        CoTaskMemFree(pidl);
    }
    return false;
}

// 扫描HTML文件夹
std::vector<HtmlFileInfo> ScanHtmlFiles() {
    std::vector<HtmlFileInfo> files;
    namespace fs = std::filesystem;
    
    std::wstring folderPath = LoadFolderPath();
    if (!fs::exists(folderPath)) {
        return files;
    }

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.path().extension() == L".html") {
            HtmlFileInfo info;
            info.path = entry.path().wstring();
            info.name = entry.path().stem().wstring();  // 获取不带扩展名的文件名
            files.push_back(info);
        }
    }
    return files;
}

// 更新按钮布局函数
void LayoutButtons(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    
    const int BUTTON_WIDTH = 250;          // 普通按钮宽度
    const int SMALL_BUTTON_WIDTH = 120;    // 小按钮宽度
    const int BUTTON_HEIGHT = 45;
    const int BUTTON_SPACING = 15;         // 按钮间距
    
    size_t normalButtonCount = g_buttonList.size() - 2;  // 减去设置和退出按钮
    
    // 计算HTML按钮的总高度
    int TOTAL_HEIGHT = (BUTTON_HEIGHT * normalButtonCount) + 
                      (BUTTON_SPACING * (normalButtonCount - 1)) +
                      BUTTON_SPACING + BUTTON_HEIGHT; // 额外的间距和一行用于底部按钮
    
    // 计算起始Y坐标以居中显示所有按钮
    int startY = (rect.bottom - TOTAL_HEIGHT) / 2;
    int centerX = rect.right / 2;
    
    // 设置HTML按钮位置
    for (size_t i = 0; i < normalButtonCount; ++i) {
        int x = centerX - (BUTTON_WIDTH / 2);
        int y = startY + (BUTTON_HEIGHT + BUTTON_SPACING) * i;
        SetWindowPos(g_buttonList[i], NULL, x, y, BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER);
    }
    
    // 设置底部两个按钮位置
    if (g_buttonList.size() >= 2) {
        int bottomY = startY + (BUTTON_HEIGHT + BUTTON_SPACING) * normalButtonCount;
        int leftX = centerX - SMALL_BUTTON_WIDTH - (BUTTON_SPACING / 2);
        int rightX = centerX + (BUTTON_SPACING / 2);
        
        // 设置按钮
        SetWindowPos(g_buttonList[g_buttonList.size() - 2], NULL, 
                    leftX, bottomY, SMALL_BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER);
        
        // 退出按钮
        SetWindowPos(g_buttonList[g_buttonList.size() - 1], NULL, 
                    rightX, bottomY, SMALL_BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER);
    }
}

// 改进按钮绘制函数，使用GDI+实现更高质量渲染
void DrawRoundedButton(HWND hwnd, HDC hdc, RECT* rect, COLORREF color, BOOL isHovered) {
    // 创建内存DC用于双缓冲
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rect->right - rect->left, rect->bottom - rect->top);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    Gdiplus::Graphics graphics(memDC);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

    // 填充背景
    graphics.Clear(Gdiplus::Color(
        GetRValue(BACKGROUND_COLOR),
        GetGValue(BACKGROUND_COLOR),
        GetBValue(BACKGROUND_COLOR)
    ));

    // 创建圆角矩形路径
    Gdiplus::GraphicsPath path;
    float cornerRadius = 12.0f;
    float width = static_cast<float>(rect->right - rect->left);
    float height = static_cast<float>(rect->bottom - rect->top);
    
    // 使用 Gdiplus::REAL 类型的参数
    path.AddArc(0.0f, 0.0f, cornerRadius * 2, cornerRadius * 2, 180.0f, 90.0f);
    path.AddLine(cornerRadius, 0.0f, width - cornerRadius, 0.0f);
    path.AddArc(width - cornerRadius * 2, 0.0f, cornerRadius * 2, cornerRadius * 2, 270.0f, 90.0f);
    path.AddLine(width, cornerRadius, width, height - cornerRadius);
    path.AddArc(width - cornerRadius * 2, height - cornerRadius * 2, cornerRadius * 2, cornerRadius * 2, 0.0f, 90.0f);
    path.AddLine(width - cornerRadius, height, cornerRadius, height);
    path.AddArc(0.0f, height - cornerRadius * 2, cornerRadius * 2, cornerRadius * 2, 90.0f, 90.0f);
    path.AddLine(0.0f, height - cornerRadius, 0.0f, cornerRadius);
    path.CloseFigure();

    // 创建按钮颜色画刷
    Gdiplus::Color btnColor(
        GetRValue(color),
        GetGValue(color),
        GetBValue(color)
    );
    Gdiplus::SolidBrush brush(btnColor);
    
    // 绘制按钮
    graphics.FillPath(&brush, &path);

    // 如果是悬停状态，添加发光效果
    if (isHovered) {
        Gdiplus::Pen pen(Gdiplus::Color(80, 255, 255, 255), 2.0f);
        graphics.DrawPath(&pen, &path);
    }

    // 将内存DC的内容复制到实际DC
    BitBlt(hdc, rect->left, rect->top,
           rect->right - rect->left, rect->bottom - rect->top,
           memDC, 0, 0, SRCCOPY);

    // 清理资源
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}

// 添加刷新按钮函数
void RefreshButtons(HWND hwnd, HINSTANCE hInstance) {
    // 清除现有按钮
    for (HWND button : g_buttonList) {
        DestroyWindow(button);
    }
    g_buttonList.clear();

    // 重新创建按钮
    std::vector<HtmlFileInfo> htmlFiles = ScanHtmlFiles();
    DWORD buttonStyle = WS_VISIBLE | WS_CHILD | BS_OWNERDRAW;
    
    // 创建HTML文件按钮
    for (size_t i = 0; i < htmlFiles.size(); ++i) {
        HWND hButton = CreateWindow(
            TEXT("BUTTON"), 
            htmlFiles[i].name.c_str(),
            buttonStyle, 0, 0, 0, 0,
            hwnd, (HMENU)(i + 1),
            hInstance, NULL
        );
        g_buttonList.push_back(hButton);
    }

    // 创建设置按钮（使用较短的宽度）
    HWND hSettingButton = CreateWindow(
        TEXT("BUTTON"),
        TEXT("设置"),
        buttonStyle, 0, 0, 0, 0,  // 位置和大小将由LayoutButtons设置
        hwnd, (HMENU)(htmlFiles.size() + 1),
        hInstance, NULL
    );
    g_buttonList.push_back(hSettingButton);

    // 创建退出按钮（使用较短的宽度）
    HWND hExitButton = CreateWindow(
        TEXT("BUTTON"),
        TEXT("退出"),  // 文字也缩短
        buttonStyle, 0, 0, 0, 0,  // 位置和大小将由LayoutButtons设置
        hwnd, (HMENU)(htmlFiles.size() + 2),
        hInstance, NULL
    );
    g_buttonList.push_back(hExitButton);

    // 为所有按钮设置字体
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    for (HWND hButton : g_buttonList) {
        SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    LayoutButtons(hwnd);
}

#ifdef UNICODE
int WINAPI wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine,
    int nCmdShow)
#else
int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
#endif
{
    // 初始化COM库（用于文件夹选择对话框）
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    // 初始化GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // 注册窗口类
    const TCHAR CLASS_NAME[] = TEXT("HTML File Selector");
    
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYICON));  // 添加这一行
    wc.hbrBackground = NULL; // 移除默认背景色
    
    RegisterClass(&wc);

    // 创建自定义按钮类
    WNDCLASS btnClass = {};
    btnClass.lpfnWndProc = DefWindowProc;
    btnClass.hInstance = hInstance;
    btnClass.lpszClassName = TEXT("CustomButton");
    RegisterClass(&btnClass);

    // 读取保存的窗口大小
    int windowWidth, windowHeight;
    LoadWindowSize(windowWidth, windowHeight);

    // 创建窗口
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        TEXT("HTML File Selector"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 
        windowWidth, windowHeight,  // 使用保存的窗口大小
        NULL,
        NULL,
        hInstance,
        NULL
    );

    // 创建主窗口后，添加COM清理
    if (hwnd == NULL) {
        CoUninitialize();
        return 0;
    }

    // 扫描HTML文件并创建按钮
    std::vector<HtmlFileInfo> htmlFiles = ScanHtmlFiles();
    DWORD buttonStyle = WS_VISIBLE | WS_CHILD | BS_OWNERDRAW;
    
    // 创建HTML文件按钮
    for (size_t i = 0; i < htmlFiles.size(); ++i) {
        HWND hButton = CreateWindow(
            TEXT("BUTTON"), 
            htmlFiles[i].name.c_str(),
            buttonStyle,
            0, 0, 0, 0,
            hwnd,
            (HMENU)(i + 1), // 使用索引+1作为按钮ID
            hInstance,
            NULL
        );
        g_buttonList.push_back(hButton);
    }

    // 创建设置按钮（使用较短的宽度）
    HWND hSettingButton = CreateWindow(
        TEXT("BUTTON"),
        TEXT("设置"),
        buttonStyle, 0, 0, 0, 0,  // 位置和大小将由LayoutButtons设置
        hwnd, (HMENU)(htmlFiles.size() + 1),
        hInstance, NULL
    );
    g_buttonList.push_back(hSettingButton);

    // 创建退出按钮（使用较短的宽度）
    HWND hExitButton = CreateWindow(
        TEXT("BUTTON"),
        TEXT("退出"),  // 文字也缩短
        buttonStyle, 0, 0, 0, 0,  // 位置和大小将由LayoutButtons设置
        hwnd, (HMENU)(htmlFiles.size() + 2),
        hInstance, NULL
    );
    g_buttonList.push_back(hExitButton);

    // 更新字体设置为最高质量
    HFONT hFont = CreateFont(22,                    // 字体高度
                            0,                      // 字体宽度
                            0, 0,                   // 倾斜和旋转角度
                            FW_MEDIUM,              // 字重
                            FALSE, FALSE, FALSE,    // 斜体、下划线、删除线
                            DEFAULT_CHARSET,
                            OUT_TT_PRECIS,          // 强制使用TrueType字体
                            CLIP_TT_ALWAYS,         // 裁剪使用TrueType字体
                            CLEARTYPE_NATURAL_QUALITY, // 使用最高质量ClearType
                            FF_DONTCARE | VARIABLE_PITCH, // 变宽字体
                            TEXT("Microsoft YaHei UI"));

    // 为所有按钮设置字体
    for (HWND hButton : g_buttonList) {
        SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    // 创建画刷
    g_hBackgroundBrush = CreateSolidBrush(BACKGROUND_COLOR);    // 深蓝色背景
    g_hButtonBrush = CreateSolidBrush(BUTTON_COLOR);           // 标准蓝色按钮
    g_hButtonHoverBrush = CreateSolidBrush(BUTTON_HOVER_COLOR); // 浅蓝色悬停

    // 初始布局
    LayoutButtons(hwnd);

    ShowWindow(hwnd, nCmdShow);

    // 消息循环
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 在消息循环结束后清理GDI+
    Gdiplus::GdiplusShutdown(gdiplusToken);

    // 在消息循环结束后清理COM
    CoUninitialize();
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        // 创建背景画刷
        g_hBackgroundBrush = CreateSolidBrush(BACKGROUND_COLOR);
        return 0;

    case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            lpMMI->ptMinTrackSize.x = MIN_WINDOW_WIDTH;
            lpMMI->ptMinTrackSize.y = MIN_WINDOW_HEIGHT;
        }
        return 0;

    case WM_CTLCOLORBTN:
        // 设置按钮背景色
        SetBkMode((HDC)wParam, TRANSPARENT);
        return (LRESULT)GetStockObject(WHITE_BRUSH);

    case WM_ERASEBKGND:
        {
            RECT rect;
            GetClientRect(hwnd, &rect);
            FillRect((HDC)wParam, &rect, g_hBackgroundBrush);
            return TRUE;
        }

    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED) {
            RECT rect;
            GetWindowRect(hwnd, &rect);
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;
            SaveWindowSize(width, height);
        }
        LayoutButtons(hwnd);
        break;
    case WM_COMMAND:
        {
            int buttonId = LOWORD(wParam);
            std::vector<HtmlFileInfo> htmlFiles = ScanHtmlFiles();
            
            if (buttonId <= htmlFiles.size()) {
                // HTML文件按钮
                const HtmlFileInfo& fileInfo = htmlFiles[buttonId - 1];
                if (FileExists(fileInfo.path.c_str())) {
                    ShellExecute(NULL, TEXT("open"), fileInfo.path.c_str(), NULL, NULL, SW_SHOWNORMAL);
                } else {
                    std::wstring message = L"找不到文件：" + fileInfo.name + L".html！";
                    MessageBox(hwnd, message.c_str(), TEXT("错误"), MB_OK | MB_ICONERROR);
                }
            }
            else if (buttonId == htmlFiles.size() + 1) {
                // 设置按钮
                std::wstring newPath;
                if (ShowFolderDialog(hwnd, newPath)) {
                    SaveFolderPath(newPath);
                    RefreshButtons(hwnd, GetModuleHandle(NULL));
                }
            }
            else if (buttonId == htmlFiles.size() + 2) {
                // 退出按钮
                PostQuitMessage(0);
            }
        }
        break;
    case WM_DESTROY:
        if (g_hBackgroundBrush) DeleteObject(g_hBackgroundBrush);
        if (g_hButtonBrush) DeleteObject(g_hButtonBrush);
        if (g_hButtonHoverBrush) DeleteObject(g_hButtonHoverBrush);
        PostQuitMessage(0);
        return 0;

    case WM_DRAWITEM:
        if (((LPDRAWITEMSTRUCT)lParam)->CtlType == ODT_BUTTON)
        {
            DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
            RECT rect = dis->rcItem;
            BOOL isHovered = (dis->hwndItem == g_hHoveredButton);
            COLORREF btnColor = isHovered ? BUTTON_HOVER_COLOR : BUTTON_COLOR;
            
            DrawRoundedButton(dis->hwndItem, dis->hDC, &rect, btnColor, isHovered);
            
            // 绘制文本
            SetBkMode(dis->hDC, TRANSPARENT);
            SetTextColor(dis->hDC, TEXT_COLOR);
            TCHAR text[256];
            GetWindowText(dis->hwndItem, text, 256);
            DrawText(dis->hDC, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            
            return TRUE;
        }
        break;

    case WM_MOUSEMOVE:
        {
            POINT pt = {LOWORD(lParam), HIWORD(lParam)};
            HWND hHovered = ChildWindowFromPoint(hwnd, pt);
            if (hHovered != g_hHoveredButton)
            {
                g_hHoveredButton = hHovered;
                // 重绘所有按钮
                for (HWND hButton : g_buttonList) {
                    InvalidateRect(hButton, NULL, TRUE);
                }
            }
        }
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
