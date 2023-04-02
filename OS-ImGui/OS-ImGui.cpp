#pragma once
#include "OS-ImGui.h"

/****************************************************
* Copyright (C)	: Liv
* @file			: OS-ImGui.cpp
* @author		: Liv
* @email		: 1319923129@qq.com
* @version		: 1.0
* @date			: 2023/4/2	13:16
****************************************************/

// D3D11 Device
namespace OSImGui
{
    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool D3DDevice::CreateDeviceD3D(HWND hWnd)
    {
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT createDeviceFlags = 0;
        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
        HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
        if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
            res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_WARP, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
        if (res != S_OK)
            return false;

        CreateRenderTarget();
        return true;
    }

    void D3DDevice::CleanupDeviceD3D()
    {
        CleanupRenderTarget();
        if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
        if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
        if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    }

    void D3DDevice::CreateRenderTarget()
    {
        ID3D11Texture2D* pBackBuffer;
        g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }

    void D3DDevice::CleanupRenderTarget()
    {
        if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
    }
}

// OS-ImGui Base 基础功能
namespace OSImGui
{
    void OSImGui_Base::NewWindow(std::string WindowName, Vec2 WindowSize, std::function<void()> CallBack)
    {
        if (!CallBack)
            throw OSException("CallBack is empty");
        if (WindowName.empty())
            Window.Name = "Window";

        Window.Name = WindowName;
        Window.wName = StringToWstring(Window.Name);
        Window.ClassName = "WindowClass";
        Window.wClassName = StringToWstring(Window.ClassName);
        Window.Size = WindowSize;

        Type = NEW;
        CallBackFn = CallBack;

        if (!CreateMyWindow())
            throw OSException("CreateMyWindow() call failed");

        try {
            InitImGui();
        }
        catch (OSException& e)
        {
            throw e;
        }

        MainLoop();
    }

    void OSImGui_Base::AttachAnotherWindow(std::string DestWindowName, std::string DestWindowClassName, std::function<void()> CallBack)
    {
        if (!CallBack)
            throw OSException("CallBack is empty");
        if (DestWindowName.empty() && DestWindowClassName.empty())
            throw OSException("DestWindowName and DestWindowClassName are empty");

        Window.Name = "Window";
        Window.wName = StringToWstring(Window.Name);
        Window.ClassName = "WindowClass";
        Window.wClassName = StringToWstring(Window.ClassName);
        Window.BgColor = ImColor(0, 0, 0, 0);

        DestWindow.hWnd = FindWindowA(
            (DestWindowClassName.empty() ? NULL : DestWindowClassName.c_str()),
            (DestWindowName.empty() ? NULL : DestWindowName.c_str()));
        if (DestWindow.hWnd == NULL)
            throw OSException("DestWindow isn't exist");
        DestWindow.Name = DestWindowName;
        DestWindow.ClassName = DestWindowClassName;

        Type = ATTACH;
        CallBackFn = CallBack;

        if (!CreateMyWindow())
            throw OSException("CreateMyWindow() call failed");

        try {
            InitImGui();
        }
        catch (OSException& e)
        {
            throw e;
        }

        MainLoop();
    }

    void OSImGui_Base::Quit()
    {
        EndFlag = true;
    }

    bool OSImGui_Base::PeekEndMessage()
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                return true;
        }
        return false;
    }

    void OSImGui_Base::MainLoop()
    {
        while (!EndFlag)
        {
            if (PeekEndMessage())
                break;
            if (Type == ATTACH)
            {
                if (!UpdateWindowData())
                    break;
            }

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            this->CallBackFn();

            ImGui::Render();
            const float clear_color_with_alpha[4] = { Window.BgColor.Value.x, Window.BgColor.Value.y , Window.BgColor.Value.z, Window.BgColor.Value.w };
            Device.g_pd3dDeviceContext->OMSetRenderTargets(1, &Device.g_mainRenderTargetView, NULL);
            Device.g_pd3dDeviceContext->ClearRenderTargetView(Device.g_mainRenderTargetView, clear_color_with_alpha);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            Device.g_pSwapChain->Present(1, 0); // Present with vs
        }
        CleanImGui();
    }

    bool OSImGui_Base::CreateMyWindow()
    {
        WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, Window.wClassName.c_str(), NULL };
        RegisterClassExW(&wc);
        if (Type == ATTACH)
        {
            Window.hWnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT, Window.wClassName.c_str(), Window.wName.c_str(), WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, 100, 100, NULL, NULL, GetModuleHandle(NULL), NULL);
            SetLayeredWindowAttributes(Window.hWnd, 0, 255, LWA_ALPHA);
        }
        else
        {
            Window.hWnd = CreateWindowW(Window.wClassName.c_str(), Window.wName.c_str(), WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU, Window.Pos.x, Window.Pos.y, Window.Size.x, Window.Size.y, NULL, NULL, wc.hInstance, NULL);
        }
        Window.hInstance = wc.hInstance;

        if (!Device.CreateDeviceD3D(Window.hWnd))
        {
            Device.CleanupDeviceD3D();
            UnregisterClassW(wc.lpszClassName, wc.hInstance);
            return false;
        }

        ShowWindow(Window.hWnd, SW_SHOWDEFAULT);
        UpdateWindow(Window.hWnd);

        return Window.hWnd != NULL;
    }

    bool OSImGui_Base::UpdateWindowData()
    {
        POINT Point{};
        RECT Rect{};

        DestWindow.hWnd = FindWindowA(
            (DestWindow.ClassName.empty() ? NULL : DestWindow.ClassName.c_str()),
            (DestWindow.Name.empty() ? NULL : DestWindow.Name.c_str()));
        if (DestWindow.hWnd == NULL)
            return false;

        GetClientRect(DestWindow.hWnd, &Rect);
        ClientToScreen(DestWindow.hWnd, &Point);

        Window.Pos = DestWindow.Pos = Vec2(static_cast<float>(Point.x), static_cast<float>(Point.y));
        Window.Size = DestWindow.Size = Vec2(static_cast<float>(Rect.right), static_cast<float>(Rect.bottom));

        SetWindowPos(Window.hWnd, HWND_TOPMOST, Window.Pos.x, Window.Pos.y, Window.Size.x, Window.Size.y, SWP_SHOWWINDOW);

        // 控制窗口状态切换
        POINT MousePos;
        GetCursorPos(&MousePos);
        ScreenToClient(Window.hWnd, &MousePos);
        ImGui::GetIO().MousePos.x = MousePos.x;
        ImGui::GetIO().MousePos.y = MousePos.y;

        if (ImGui::GetIO().WantCaptureMouse)
            SetWindowLong(Window.hWnd, GWL_EXSTYLE, GetWindowLong(Window.hWnd, GWL_EXSTYLE) & (~WS_EX_LAYERED));
        else
            SetWindowLong(Window.hWnd, GWL_EXSTYLE, GetWindowLong(Window.hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
        return true;
    }

    bool OSImGui_Base::InitImGui()
    {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui::StyleColorsDark();
        io.LogFilename = nullptr;

        if (!ImGui_ImplWin32_Init(Window.hWnd))
            throw OSException("ImGui_ImplWin32_Init() call failed.");
        if (!ImGui_ImplDX11_Init(Device.g_pd3dDevice, Device.g_pd3dDeviceContext))
            throw OSException("ImGui_ImplDX11_Init() call failed.");

        return true;
    }

    void OSImGui_Base::CleanImGui()
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        Device.CleanupDeviceD3D();
        DestroyWindow(Window.hWnd);
        UnregisterClassA(Window.ClassName.c_str(), Window.hInstance);
    }

    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg)
        {
        case WM_CREATE:
        {
            MARGINS     Margin = { -1 };
            DwmExtendFrameIntoClientArea(hWnd, &Margin);
            break;
        }
        case WM_SIZE:
            if (Device.g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
            {
                Device.CleanupRenderTarget();
                Device.g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                Device.CreateRenderTarget();
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU)
                return 0;
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
        }
        return ::DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    std::wstring OSImGui_Base::StringToWstring(std::string& str)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.from_bytes(str);
    }
}

// OS-ImGui Draw 绘制功能
namespace OSImGui
{
    void OSImGui::Text(std::string Text, Vec2 Pos, ImColor Color, float FontSize, bool KeepCenter)
    {
        if (!KeepCenter)
        {
            ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), FontSize, Pos.ToImVec2(), Color, Text.c_str());
        }
        else
        {
            float TextWidth = ImGui::GetFont()->CalcTextSizeA(FontSize, FLT_MAX, 0.f, Text.c_str()).x;
            ImVec2 Pos_ = { Pos.x - TextWidth / 2,Pos.y };
            ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), FontSize, Pos_, Color, Text.c_str());
        }
    }

    void OSImGui::StrokeText(std::string Text, Vec2 Pos, ImColor Color, float FontSize, bool KeepCenter)
    {
        this->Text(Text, Vec2(Pos.x - 1, Pos.y + 1), ImColor(0, 0, 0), FontSize, KeepCenter);
        this->Text(Text, Vec2(Pos.x - 1, Pos.y - 1), ImColor(0, 0, 0), FontSize, KeepCenter);
        this->Text(Text, Vec2(Pos.x + 1, Pos.y + 1), ImColor(0, 0, 0), FontSize, KeepCenter);
        this->Text(Text, Vec2(Pos.x + 1, Pos.y - 1), ImColor(0, 0, 0), FontSize, KeepCenter);
        this->Text(Text, Pos, Color, FontSize, KeepCenter);
    }

    void OSImGui::Rectangle(Vec2 Pos, Vec2 Size, ImColor Color, float Thickness, float Rounding)
    {
        ImGui::GetForegroundDrawList()->AddRect(Pos.ToImVec2(), ImVec2(Pos.x + Size.x, Pos.y + Size.y), Color, Rounding, 0, Thickness);
    }

    void OSImGui::RectangleFilled(Vec2 Pos, Vec2 Size, ImColor Color, float Rounding, float Nums)
    {
        ImDrawList* DrawList = ImGui::GetForegroundDrawList();
        ImDrawCornerFlags rounding_corners = ImDrawCornerFlags_All;
        ImVec2 a = Pos.ToImVec2();
        ImVec2 b = { Pos.x + Size.x,Pos.y + Size.y };
        Rounding = ImMin<float>(Rounding, fabsf(Size.x) * (((rounding_corners & ImDrawCornerFlags_Top) == ImDrawCornerFlags_Top) || ((rounding_corners & ImDrawCornerFlags_Bot) == ImDrawCornerFlags_Bot) ? 0.5f : 1.0f) - 1.0f);
        Rounding = ImMin<float>(Rounding, fabsf(Size.y) * (((rounding_corners & ImDrawCornerFlags_Left) == ImDrawCornerFlags_Left) || ((rounding_corners & ImDrawCornerFlags_Right) == ImDrawCornerFlags_Right) ? 0.5f : 1.0f) - 1.0f);

        if (Rounding <= 0.0f || rounding_corners == 0)
        {
            DrawList->PathLineTo(a);
            DrawList->PathLineTo(ImVec2(b.x, a.y));
            DrawList->PathLineTo(b);
            DrawList->PathLineTo(ImVec2(a.x, b.y));
        }
        else
        {
            DrawList->PathArcTo(ImVec2(a.x + Rounding, a.y + Rounding), Rounding, IM_PI, IM_PI / 2 * 3, Nums);
            DrawList->PathArcTo(ImVec2(b.x - Rounding, a.y + Rounding), Rounding, IM_PI / 2 * 3, IM_PI * 2, Nums);
            DrawList->PathArcTo(ImVec2(b.x - Rounding, b.y - Rounding), Rounding, 0, IM_PI / 2, Nums);
            DrawList->PathArcTo(ImVec2(a.x + Rounding, b.y - Rounding), Rounding, IM_PI / 2, IM_PI, Nums);
        }
        DrawList->PathFillConvex(Color);
    }

    void OSImGui::Line(Vec2 From, Vec2 To, ImColor Color, float Thickness)
    {
        ImGui::GetForegroundDrawList()->AddLine(From.ToImVec2(), To.ToImVec2(), Color, Thickness);
    }

    void OSImGui::Circle(Vec2 Center, float Radius, ImColor Color, float Thickness, int Num)
    {
        ImGui::GetForegroundDrawList()->AddCircle(Center.ToImVec2(), Radius, Color, Num, Thickness);
    }

    void OSImGui::CircleFilled(Vec2 Center, float Radius, ImColor Color, int Num)
    {
        ImGui::GetForegroundDrawList()->AddCircleFilled(Center.ToImVec2(), Radius, Color, Num);
    }

    void OSImGui::Arc(Vec2 Pos, float Radius, ImColor Color, float Angel, float Proportion, float Thickness)
    {
        float Count = 30;
        float Angle = 360 * Proportion;
        float IncreaseAngle = Angle / Count;
        float Alpha;

        Alpha = -Angel - Angle / 2;

        float TempAngle = Alpha;
        Vec2 Previous, Current;
        for (int i = 0; i < Count; i++, TempAngle += IncreaseAngle)
        {
            Current.x = Pos.x + Radius * cos(TempAngle * 3.1415926 / 180);
            Current.y = Pos.y + Radius * sin(TempAngle * 3.1415926 / 180);
            if (i)
                this->Line(Previous, Current, Color, Thickness);
            Previous = Current;
        }
    }

    void OSImGui::ConnectPoints(std::vector<Vec2> Points, ImColor Color, float Thickness)
    {
        if (Points.size() <= 0)
            return;
        for (int i = 0; i < Points.size() - 1; i++)
        {
            Line(Points[i], Points[i + 1], Color, Thickness);
            if (i == Points.size() - 2)
                Line(Points[i + 1], Points[0], Color, Thickness);
        }
    }

    void OSImGui::Arc(ImVec2 Center, float Radius, ImColor Color, float Thickness, float Angle_begin, float Angle_end, float Nums)
    {
        ImDrawList* DrawList = ImGui::GetForegroundDrawList();
        float angle = (Angle_end - Angle_begin) / Nums;
        for (int i = 0; i < Nums; i++)
        {
            float angle_ = i * angle + Angle_begin - IM_PI / 2;
            DrawList->PathLineTo({ Center.x - Radius * cos(angle_),Center.y - Radius * sin(angle_) });
        }
        DrawList->PathStroke(Color, false, Thickness);
    }

    void OSImGui::MyCheckBox(const char* str_id, bool* v)
    {
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* DrawList = ImGui::GetWindowDrawList();
        float Height = ImGui::GetFrameHeight();
        float Width = Height * 1.7;
        float Radius = Height / 2 - 2;

        ImGui::InvisibleButton(str_id, ImVec2(Width, Height));
        if (ImGui::IsItemClicked())
            *v = !(*v);
        // 组件移动动画
        float t = *v ? 1.0f : 0.f;
        ImGuiContext& g = *GImGui;
        float AnimationSpeed = 0.08f;
        if (g.LastActiveId == g.CurrentWindow->GetID(str_id))
        {
            float T_Animation = ImSaturate(g.LastActiveIdTimer / AnimationSpeed);
            t = *v ? (T_Animation) : (1.0f - T_Animation);
        }
        // 鼠标悬停颜色
        ImU32 Color;
        if (ImGui::IsItemHovered())
            Color = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.24f, 0.15f, 1.0f), ImVec4(0.55f, 0.85f, 0.13, 1.000f), t));
        else
            Color = ImGui::GetColorU32(ImLerp(ImVec4(0.90f, 0.29f, 0.20f, 1.0f), ImVec4(0.60f, 0.90f, 0.18, 1.000f), t));
        // 组件绘制
        DrawList->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + Width, p.y + Height), Color, Height);
        DrawList->AddCircleFilled(ImVec2(p.x + Radius + t * (Width - Radius * 2) + (t == 0 ? 2 : -2), p.y + Radius + 2), Radius, IM_COL32(255, 255, 255, 255), 360);
        DrawList->AddCircle(ImVec2(p.x + Radius + t * (Width - Radius * 2) + (t == 0 ? 2 : -2), p.y + Radius + 2), Radius, IM_COL32(20, 20, 20, 80), 360, 1);

        ImGui::SameLine();
        ImGui::Text(str_id);
    }

    void OSImGui::MyCheckBox2(const char* str_id, bool* v)
    {
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* DrawList = ImGui::GetWindowDrawList();
        float Height = ImGui::GetFrameHeight();
        float Width = Height * 1.7;
        float Radius = Height / 2 - 2;

        ImGui::InvisibleButton(str_id, ImVec2(Width, Height));
        if (ImGui::IsItemClicked())
            *v = !(*v);
        // 组件移动动画
        float t = *v ? 1.0f : 0.f;
        ImGuiContext& g = *GImGui;
        float AnimationSpeed = 0.15f;
        if (g.LastActiveId == g.CurrentWindow->GetID(str_id))
        {
            float T_Animation = ImSaturate(g.LastActiveIdTimer / AnimationSpeed);
            t = *v ? (T_Animation) : (1.0f - T_Animation);
        }
        // 鼠标悬停颜色
        ImU32 Color;
        if (ImGui::IsItemHovered())
            Color = ImGui::GetColorU32(ImLerp(ImVec4(0.08f, 0.18f, 0.21f, 1.0f), ImVec4(0.10f, 0.48f, 0.68f, 1.000f), t));
        else
            Color = ImGui::GetColorU32(ImLerp(ImVec4(0.12f, 0.22f, 0.25f, 1.0f), ImVec4(0.14f, 0.52f, 0.72f, 1.000f), t));
        // 组件绘制
        DrawList->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + Width, p.y + Height), Color, 360);
        DrawList->AddCircleFilled(ImVec2(p.x + Radius + 2 + t * (Width - (Radius + 2) * 2), p.y + Radius + 2), Radius + 2, IM_COL32(255, 255, 255, 255), 360);
        DrawList->AddCircleFilled(ImVec2(p.x + Radius + t * (Width - Radius * 2) + (t == 0 ? 2 : -2), p.y + Radius + 2), Radius, IM_COL32(230, 230, 230, 255), 360);
        if (*v)
            DrawList->AddText(ImVec2(p.x + 45, p.y + 2), ImColor{ 255,255,255,255 }, str_id);
        else
            DrawList->AddText(ImVec2(p.x + 45, p.y + 2), ImColor{ 185,185,185,255 }, str_id);

    }

    void OSImGui::MyCheckBox3(const char* str_id, bool* v)
    {
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* DrawList = ImGui::GetWindowDrawList();
        float Height = ImGui::GetFrameHeight();
        float Width = Height;
        float Left = 8;
        float Right = Left * 1.5;
        ImGui::InvisibleButton(str_id, ImVec2(Width, Height));

        if (ImGui::IsItemClicked())
            *v = !(*v);
        // 组件移动动画
        float t = *v ? 1.0f : 0.f;
        ImGuiContext& g = *GImGui;
        float AnimationSpeed = 0.12f;
        if (g.LastActiveId == g.CurrentWindow->GetID(str_id))
        {
            float T_Animation = ImSaturate(g.LastActiveIdTimer / AnimationSpeed);
            t = *v ? (T_Animation) : (1.0f - T_Animation);
        }
        // 鼠标悬停颜色
        ImU32 Color;
        ImU32 TickColor1, TickColor2;
        if (ImGui::IsItemHovered())
            Color = ImGui::GetColorU32(ImLerp(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), ImVec4(0.05f, 0.85f, 0.25f, 1.000f), t));
        else
            Color = ImGui::GetColorU32(ImLerp(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), ImVec4(0.1f, 0.9f, 0.3f, 1.000f), t));

        TickColor1 = IM_COL32(255, 255, 255, 255 * t);
        TickColor2 = IM_COL32(180, 180, 180, 255 * (1 - t));

        float Size = Width;
        float Scale = (float)(Size) / 20.0f;
        // 底色
        DrawList->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + Width, p.y + Height), Color, 5, 15);
        // 选中勾
        DrawList->AddLine(ImVec2(p.x + 3 * Scale, p.y + Size / 2 - 2 * Scale), ImVec2(p.x + Size / 2 - 1 * Scale, p.y + Size - 5 * Scale), TickColor1, 3 * Scale);
        DrawList->AddLine(ImVec2(p.x + Size - 3 * Scale - 1, p.y + 3 * Scale + 1), ImVec2(p.x + Size / 2 - 1 * Scale, p.y + Size - 5 * Scale), TickColor1, 3 * Scale);
        // 未选中勾
        DrawList->AddLine(ImVec2(p.x + 3 * Scale, p.y + Size / 2 - 2 * Scale), ImVec2(p.x + Size / 2 - 1 * Scale, p.y + Size - 5 * Scale), TickColor2, 3 * Scale);
        DrawList->AddLine(ImVec2(p.x + Size - 3 * Scale - 1, p.y + 3 * Scale + 1), ImVec2(p.x + Size / 2 - 1 * Scale, p.y + Size - 5 * Scale), TickColor2, 3 * Scale);
        ImGui::SameLine();
        ImGui::Text(str_id);
    }

    void OSImGui::MyCheckBox4(const char* str_id, bool* v)
    {
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* DrawList = ImGui::GetWindowDrawList();
        float Height = ImGui::GetFrameHeight();
        float Width = Height;
        ImGui::InvisibleButton(str_id, ImVec2(Width, Height));

        if (ImGui::IsItemClicked())
            *v = !(*v);
        // 组件动画
        float t = *v ? 1.0f : 0.f;
        ImGuiContext& g = *GImGui;
        float AnimationSpeed = 0.12f;
        if (g.LastActiveId == g.CurrentWindow->GetID(str_id))
        {
            float T_Animation = ImSaturate(g.LastActiveIdTimer / AnimationSpeed);
            t = *v ? (T_Animation) : (1.0f - T_Animation);
        }
        // bg 0.74 0.72 0.81-> 0.69 0.77 0.76
        ImU32 BgColor;
        if (ImGui::IsItemHovered())
            BgColor = ImGui::GetColorU32(ImVec4(0.69f, 0.69f, 0.69f, 1.0f));
        else
            BgColor = ImGui::GetColorU32(ImVec4(0.74f, 0.74f, 0.74f, 1.0f));
        DrawList->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + Width, p.y + Width), BgColor);

        ImU32 FrColor;
        FrColor = ImGui::GetColorU32(ImVec4(0.f, 0.f, 0.f, 0.5f * t));
        DrawList->AddRectFilled(ImVec2(p.x + Width / 5, p.y + Width / 5), ImVec2(p.x + Width - Width / 5, p.y + Width - Width / 5), FrColor);

        ImGui::SameLine();
        ImGui::Text(str_id);
    }
}