#pragma once
#include "Memory.h"
#include "Mathem.h"
#include "ImGui 1.91/imgui.h"
#include "ImGui 1.91/imgui_internal.h"
#include "ImGui 1.91/backends/imgui_impl_dx9.h"
#include "ImGui 1.91/backends/imgui_impl_win32.h"
#include "d3d9.h"
#include <dwmapi.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dwmapi.lib")

COLORREF SnapLineCOLOR = RGB(0, 0, 255);
HBRUSH EnemyBrush = CreateSolidBrush(RGB(255, 0, 0));
HWND TargetWnd = FindWindow(0, L"Counter-Strike Source");
HDC HDC_Desktop = GetDC(TargetWnd);

void DrawLine(float StartX, float StartY, float EndX, float EndY, COLORREF Pen)
{
    int a, b = 0;
    HPEN hOPen;
    // penstyle, width, color
    HPEN hNPen = CreatePen(PS_SOLID, 2, Pen);
    hOPen = (HPEN)SelectObject(HDC_Desktop, hNPen);
    // starting point of line
    MoveToEx(HDC_Desktop, StartX, StartY, NULL);
    // ending point of line
    a = LineTo(HDC_Desktop, EndX, EndY);
    DeleteObject(SelectObject(HDC_Desktop, hOPen));
}

void DrawFilledRect(int x, int y, int w, int h)
{
    //We create our rectangle to draw on screen
    RECT rect = { x, y, x + w, y + h };
    //We clear that portion of the screen and display our rectangle
    FillRect(HDC_Desktop, &rect, EnemyBrush);
}


void DrawBorderBox(int x, int y, int w, int h, int thickness)
{
    //Top horiz line
    DrawFilledRect(x, y, w, thickness);
    //Left vertical line
    DrawFilledRect(x, y, thickness, h);
    //right vertical line
    DrawFilledRect((x + w), y, thickness, h);
    //bottom horiz line
    DrawFilledRect(x, y + h, w + thickness, thickness);
}

void DrawESP(int x, int y, float distance, RECT m_Rect)
{
    //ESP RECTANGLE
    int width = 18100 / distance;
    int height = 36000 / distance;
    //DrawBorderBox(x - (width / 2), y - height, width, height, 1);

    //Sandwich ++
    //DrawLine((m_Rect.right - m_Rect.left) / 2, m_Rect.bottom - m_Rect.top, x, y, SnapLineCOLOR);
    
}

bool worldToScreen(const vMatrix& m, Vec3 pos, float* pos_on_screen, RECT m_Rect) {

    float w = 0.0f;
    pos_on_screen[0] = m.matrix[0] * pos.X + m.matrix[1] * pos.Y + m.matrix[2] * pos.Z + m.matrix[3];
    pos_on_screen[1] = m.matrix[4] * pos.X + m.matrix[5] * pos.Y + m.matrix[6] * pos.Z + m.matrix[7]; 
    w = m.matrix[12] * pos.X + m.matrix[13] * pos.Y + m.matrix[14] * pos.Z + m.matrix[15];

    if (w < 0.01f)
        return false;

    float invw = 1.0f / w;
    pos_on_screen[0] *= invw;
    pos_on_screen[1] *= invw;

    //Ширина и высота экрана
    int width = (int)(m_Rect.right - m_Rect.left);
    int height = (int)(m_Rect.bottom - m_Rect.top);

    float x = width / 2;
    float y = height / 2;

    //X и Y нашего объекта в пикселях на экране
    x += 0.5 * pos_on_screen[0] * width + 0.5;
    y -= 0.5 * pos_on_screen[1] * height + 0.5;

    //Получаем координаты для рисования линий на экране с учетом расположения экрана
    pos_on_screen[0] = x + m_Rect.left;
    pos_on_screen[1] = y + m_Rect.top;

    return true;

    /*float screenW = (*(matrix.matrix[3] + 0 * 4) + pos.X) + (*(matrix.matrix[3] + 1 * 4) + pos.Y) + (*(matrix.matrix[3] + 2 * 4) + pos.Z) + *(matrix.matrix[3] + 3 * 4); //*(matrix.matrix[3] + 2 * 4) - так делать не обязательно, можно matrix.matrix[3][2]

    //Если объект находится не в притык к нам, то получаем координаты на экране
    if (screenW < 0.001f) {
        //Координаты на экране
        float screenX = (*(matrix.matrix[0] + 0 * 4) + pos.X) + (*(matrix.matrix[0] + 1 * 4) + pos.Y) + (*(matrix.matrix[0] + 2 * 4) + pos.Z) + *(matrix.matrix[0] + 3 * 4);
        float screenY = (*(matrix.matrix[1] + 0 * 4) + pos.X) + (*(matrix.matrix[1] + 1 * 4) + pos.Y) + (*(matrix.matrix[1] + 2 * 4) + pos.Z) + *(matrix.matrix[1] + 3 * 4);

        //Координаты камеры на экране (Прицела)
        float cameraX = width / 2;
        float cameraY = height / 2;

        //Координаты на экране с учетом перспективы (Отдаленности объекта на экране)
        float X = cameraX + (cameraX * screenX / screenW);
        float Y = cameraY + (cameraY * screenY / screenW);

        return Vec2(X, Y);
    }
    else {
        return Vec2(-1, -1); //За пределами экрана
    }
    */
}

//Funcs of render ImGui
//Информация об окне, которое будет наслаиваться на окно игры
namespace OverlayWindow {
    WNDCLASSEX WindowClass;
    HWND Hwnd;
    LPCSTR Name;
}

namespace DirectX9Interface {
    IDirect3D9Ex* Direct3D9 = NULL;
    IDirect3DDevice9Ex* pDevice = NULL;
    D3DPRESENT_PARAMETERS pParams = { NULL };
    MARGINS Margin = { -1 };
    MSG Message = { NULL };
}

void Draw() {
    const WCHAR* procName = L"hl2.exe";
    Memory mem = Memory{ procName };
    RECT m_Rect;
    LPCVOID mainPlayerAddr = (LPCVOID)mem.ReadAddr<DWORD>((DWORD)mem.ClientModuleAddr + 0x004C88E8); //Получение указателя на структуру нашего игрока.
    Player mainPlayer = Player(Vec3(mem.ReadAddr<float>((DWORD)mainPlayerAddr + 0x260 + 0x0), mem.ReadAddr<float>((DWORD)mainPlayerAddr + 0x260 + 0x4), mem.ReadAddr<float>((DWORD)mainPlayerAddr + 0x260 + 0x8)),
        mem.ReadAddr<int>((DWORD)mainPlayerAddr + 0x9C),
        mem.ReadAddr<int>((DWORD)mainPlayerAddr + 0x94)
    );
    mainPlayer.axis_XY = mem.ReadAddr<float>((DWORD)mem.EngineModuleAddr + 0x47C33C + 0x0); //(DWORD в памяти, но если взять байты и во float переменную считать, то получим float значение (Как и нужно)) - UPD: это уже не правильно, так делали из-за того, что CE не правильно читал структуру
    mainPlayer.axis_XZ = mem.ReadAddr<float>((DWORD)mem.EngineModuleAddr + 0x47C33C + 0x4); //float
    int numPlayers = mem.ReadAddr<int>((DWORD)mem.EngineModuleAddr + 0x5EC82C); //engine.dll + m_iNumPlayers --> Кол-во игроков на карте
    
    Player entity[32]{};
    for (int i{}; i < numPlayers; i++) {
        LPCVOID entityAddr = (LPCVOID)mem.ReadAddr<DWORD>((DWORD)mem.ClientModuleAddr + 0x4D5AE4 + 0x10 * i); //Получаем указатель на структуру игрока.
        entity[i] = Player(
            Vec3(mem.ReadAddr<float>((DWORD)entityAddr + 0x260 + 0x0), mem.ReadAddr<float>((DWORD)entityAddr + 0x260 + 0x4), mem.ReadAddr<float>((DWORD)entityAddr + 0x260 + 0x8)), //Вектора сущностей(Координаты)
            mem.ReadAddr<DWORD>((DWORD)entityAddr + 0x9C), //m_iTeamNum
            mem.ReadAddr<DWORD>((DWORD)entityAddr + 0x94)  //m_iHealth
        );
    }

    vMatrix matrix;
    matrix.read_vMatrix(mem, (DWORD)mem.EngineModuleAddr + 0x5B0D68);

    GetWindowRect(FindWindow(NULL, L"Counter-Strike Source"), &m_Rect);
    matrix.read_vMatrix(mem, (DWORD)mem.EngineModuleAddr + 0x5B0D68);
    for (int i{}; i < numPlayers; i++) {
        if (entity[i].teamNum == mainPlayer.teamNum)
            continue;
        if (entity[i].health <= 2)
            continue;

        float entityScreenPosition[2]; //Будет хранить координаты пикселей на экране
        if (worldToScreen(matrix, entity[i].vec, entityScreenPosition, m_Rect)) {
            float distance = sqrt(
                (entity[i].vec.X - mainPlayer.vec.X) * (entity[i].vec.X - mainPlayer.vec.X) +
                (entity[i].vec.Y - mainPlayer.vec.Y) * (entity[i].vec.Y - mainPlayer.vec.Y) +
                (entity[i].vec.Z - mainPlayer.vec.Z) * (entity[i].vec.Z - mainPlayer.vec.Z)
            );
            //DrawESP(entityScreenPosition[0] - m_Rect.left, entityScreenPosition[1] - m_Rect.top, distance, m_Rect);
            int width = 18100 / distance;
            int height = 36000 / distance;

            float start_x = (entityScreenPosition[0] - m_Rect.left) - (width / 2);
            float start_y = (entityScreenPosition[1] - m_Rect.top) - height;

            ImGui::GetForegroundDrawList()->AddRect({start_x, start_y}, {start_x + width, start_y + height}, ImColor(255,255,255));// - рисовка esp-бокса
            ImGui::GetForegroundDrawList()->AddLine({ start_x + width / 2, start_y + height }, {(float)(m_Rect.right/2),(float)m_Rect.bottom}, ImColor(255, 255, 255));
        }
    }
}
//Рендеринг всех элементов
void Render() {
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    Draw();  
    ImGui::EndFrame();

    DirectX9Interface::pDevice->SetRenderState(D3DRS_ZENABLE, false);
    DirectX9Interface::pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
    DirectX9Interface::pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

    DirectX9Interface::pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
    if (DirectX9Interface::pDevice->BeginScene() >= 0) {
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        DirectX9Interface::pDevice->EndScene();
    }

    HRESULT result = DirectX9Interface::pDevice->Present(NULL, NULL, NULL, NULL);
    if (result == D3DERR_DEVICELOST && DirectX9Interface::pDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
        ImGui_ImplDX9_InvalidateDeviceObjects();
        DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
        ImGui_ImplDX9_CreateDeviceObjects();
    }
}
//Основной цикл ESP (в отдельном потоке должен быть)
void mainLoopESP() {
    static RECT OldRect;
    ZeroMemory(&DirectX9Interface::Message, sizeof(MSG));
    while (DirectX9Interface::Message.message != WM_QUIT) {
        if (PeekMessage(&DirectX9Interface::Message, OverlayWindow::Hwnd, 0, 0, PM_REMOVE)) {
            TranslateMessage(&DirectX9Interface::Message);
            DispatchMessage(&DirectX9Interface::Message);
        }
        HWND ForegroundWindow = GetForegroundWindow();
        if (ForegroundWindow == TargetWnd) {
            HWND TempProcessHwnd = GetWindow(ForegroundWindow, GW_HWNDPREV);
            SetWindowPos(OverlayWindow::Hwnd, TempProcessHwnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }

        RECT TempRect;
        POINT TempPoint;
        ZeroMemory(&TempRect, sizeof(RECT));
        ZeroMemory(&TempPoint, sizeof(POINT));

        GetClientRect(TargetWnd, &TempRect);
        ClientToScreen(TargetWnd, &TempPoint);

        TempRect.left = TempPoint.x;
        TempRect.top = TempPoint.y;

        ImGuiIO& io = ImGui::GetIO();
        ImGuiViewport* viewport = ImGui::GetMainViewport();  //ImGuiIO& io = ImGui::GetIO(); <- в старых версиях
        viewport->PlatformHandleRaw = TargetWnd;             //io.ImeWindowHandle = Process::Hwnd;
        

        POINT TempPoint2;
        GetCursorPos(&TempPoint2);
        io.MousePos.x = TempPoint2.x - TempPoint.x;
        io.MousePos.y = TempPoint2.y - TempPoint.y;

        if (GetAsyncKeyState(0x1)) {
            io.MouseDown[0] = true;
            io.MouseClicked[0] = true;
            io.MouseClickedPos[0].x = io.MousePos.x;
            io.MouseClickedPos[0].x = io.MousePos.y;
        }
        else {
            io.MouseDown[0] = false;
        }

        if (TempRect.left != OldRect.left || TempRect.right != OldRect.right || TempRect.top != OldRect.top || TempRect.bottom != OldRect.bottom) {
            OldRect = TempRect;
            int width = TempRect.right;
            int height = TempRect.bottom;
            DirectX9Interface::pParams.BackBufferWidth = width;
            DirectX9Interface::pParams.BackBufferHeight = height;
            SetWindowPos(OverlayWindow::Hwnd, (HWND)0, TempPoint.x, TempPoint.y, width, height, SWP_NOREDRAW);
            DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
        }
        Render();
    }
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    if (DirectX9Interface::pDevice != NULL) {
        DirectX9Interface::pDevice->EndScene();
        DirectX9Interface::pDevice->Release();
    }
    if (DirectX9Interface::Direct3D9 != NULL) {
        DirectX9Interface::Direct3D9->Release();
    }
    DestroyWindow(OverlayWindow::Hwnd);
    UnregisterClass(OverlayWindow::WindowClass.lpszClassName, OverlayWindow::WindowClass.hInstance);
}

//Иницализация DirectX
bool DirectXInit() {
    if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &DirectX9Interface::Direct3D9))) {
        return false;
    }
    RECT TempRect;
    GetClientRect(TargetWnd, &TempRect);
    D3DPRESENT_PARAMETERS Params = { 0 };
    Params.Windowed = TRUE;
    Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    Params.hDeviceWindow = OverlayWindow::Hwnd;
    Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
    Params.BackBufferFormat = D3DFMT_A8R8G8B8;
    Params.BackBufferWidth = TempRect.right;
    Params.BackBufferHeight = TempRect.bottom;
    Params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    Params.EnableAutoDepthStencil = TRUE;
    Params.AutoDepthStencilFormat = D3DFMT_D16;
    Params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    Params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

    if (FAILED(DirectX9Interface::Direct3D9->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, OverlayWindow::Hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &Params, 0, &DirectX9Interface::pDevice))) {
        DirectX9Interface::Direct3D9->Release();
        return false;
    }

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.Fonts->AddFontDefault();

    ImGui_ImplWin32_Init(OverlayWindow::Hwnd);
    ImGui_ImplDX9_Init(DirectX9Interface::pDevice);
    DirectX9Interface::Direct3D9->Release();
    return true;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
        return true;

    switch (Message) {
    case WM_DESTROY:
        if (DirectX9Interface::pDevice != NULL) {
            DirectX9Interface::pDevice->EndScene();
            DirectX9Interface::pDevice->Release();
        }
        if (DirectX9Interface::Direct3D9 != NULL) {
            DirectX9Interface::Direct3D9->Release();
        }
        PostQuitMessage(0);
        exit(4);
        break;
    case WM_SIZE:
        if (DirectX9Interface::pDevice != NULL && wParam != SIZE_MINIMIZED) {
            ImGui_ImplDX9_InvalidateDeviceObjects();
            DirectX9Interface::pParams.BackBufferWidth = LOWORD(lParam);
            DirectX9Interface::pParams.BackBufferHeight = HIWORD(lParam);
            //HRESULT hr = DirectX9Interface::pDevice->Reset(&DirectX9Interface::pParams);
            //if (hr == D3DERR_INVALIDCALL)
                //IM_ASSERT(0);
            ImGui_ImplDX9_CreateDeviceObjects();
        }
        break;
    default:
        return DefWindowProc(hWnd, Message, wParam, lParam);
        break;
    }
    return 0;
}

//Создание окна, на котором будут рисоваться ESP-боксы, а потом это прозрачное окно будет поверх окна игры
void SetupWindow() {
    static RECT TempRect = { NULL };
    static POINT TempPoint;
    OverlayWindow::WindowClass = {
        sizeof(WNDCLASSEX), 0, WinProc, 0, 0, nullptr, LoadIcon(nullptr, IDI_APPLICATION), LoadCursor(nullptr, IDC_ARROW), nullptr, nullptr, (WCHAR*)OverlayWindow::Name, LoadIcon(nullptr, IDI_APPLICATION)
    };

    RegisterClassEx(&OverlayWindow::WindowClass);
    if (TargetWnd) {
        GetClientRect(TargetWnd, &TempRect);
        ClientToScreen(TargetWnd, &TempPoint);
        TempRect.left = TempPoint.x;
        TempRect.top = TempPoint.y;
    }

    OverlayWindow::Hwnd = CreateWindowEx(NULL, (WCHAR*)OverlayWindow::Name, (WCHAR*)OverlayWindow::Name, WS_POPUP | WS_VISIBLE, TempRect.left, TempRect.top, TempRect.right, TempRect.bottom, NULL, NULL, 0, NULL);
    DwmExtendFrameIntoClientArea(OverlayWindow::Hwnd, &DirectX9Interface::Margin);
    SetWindowLong(OverlayWindow::Hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT);
    ShowWindow(OverlayWindow::Hwnd, SW_SHOW);
    UpdateWindow(OverlayWindow::Hwnd);
}
//Начало работы рендера
void startRender() {
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    FreeConsole();
    std::string s = "ashjdkhjskahdkjsa12398";
    OverlayWindow::Name = s.c_str();
    SetupWindow();
    DirectXInit();
    mainLoopESP();
}