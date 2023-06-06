#include "OS-ImGui_Struct.h"
#include "OS-ImGui_Exception.hpp"
#include <iostream>
#include <string>
#include <functional>
#include <codecvt>
#include <vector>
#include <dwmapi.h>
#pragma comment(lib,"dwmapi.lib")

/****************************************************
* Copyright (C)	: Liv
* @file			: OS-ImGui.h
* @author		: Liv
* @email		: 1319923129@qq.com
* @version		: 1.0
* @date			: 2023/4/2	13:16
****************************************************/

namespace OSImGui
{
	class D3DDevice
	{
	public:
		ID3D11Device* g_pd3dDevice = nullptr;
		ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
		IDXGISwapChain* g_pSwapChain = nullptr;
		ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
		bool CreateDeviceD3D(HWND hWnd);
		void CleanupDeviceD3D();
		void CreateRenderTarget();
		void CleanupRenderTarget();
	};

	static D3DDevice Device;

	enum WindowType
	{
		NEW,
		ATTACH
	};

	class WindowData
	{
	public:
		HWND  hWnd = NULL;
		HINSTANCE hInstance;
		std::string Name;
		std::wstring wName;
		std::string ClassName;
		std::wstring wClassName;
		Vec2 Pos;
		Vec2 Size;
		ImColor BgColor{ 255, 255, 255 };
	};

	class OSImGui_Base
	{
	private:
		// 回调函数
		std::function<void()> CallBackFn = nullptr;
		// 退出标识
		bool EndFlag = false;
		// 启动类型
		WindowType Type = NEW;
	protected:
		// 窗口数据
		WindowData Window;
		// 目标窗口数据
		WindowData DestWindow;
	public:
		// 创建一个新窗口
		void NewWindow(std::string WindowName, Vec2 WindowSize, std::function<void()> CallBack);
		// 附加到另一个窗口上
		void AttachAnotherWindow(std::string DestWindowName, std::string DestWindowClassName, std::function<void()> CallBack);
		// 退出
		void Quit();
	private:
		void MainLoop();
		bool CreateMyWindow();
		bool UpdateWindowData();
		bool InitImGui();
		void CleanImGui();
		bool PeekEndMessage();
		std::wstring StringToWstring(std::string& str);
	};

	class OSImGui : public OSImGui_Base, public Singleton<OSImGui>
	{
	public:
		// 文本
		void Text(std::string Text, Vec2 Pos, ImColor Color, float FontSize = 15, bool KeepCenter = false);
		// 描边文本
		void StrokeText(std::string Text, Vec2 Pos, ImColor Color, float FontSize = 15, bool KeepCenter = false);
		// 矩形
		void Rectangle(Vec2 Pos, Vec2 Size, ImColor Color, float Thickness, float Rounding = 0);
		void RectangleFilled(Vec2 Pos, Vec2 Size, ImColor Color, float Rounding = 0, float Nums = 15);
		// 线
		void Line(Vec2 From, Vec2 To, ImColor Color, float Thickness);
		// 圆形
		void Circle(Vec2 Center, float Radius, ImColor Color,float Thickness, int Num = 50);
		void CircleFilled(Vec2 Center, float Radius, ImColor Color, int Num = 50);
		// 连接点
		void ConnectPoints(std::vector<Vec2> Points, ImColor Color, float Thickness);
		// 圆弧
		void Arc(ImVec2 Center, float Radius, ImColor Color, float Thickness, float Aangle_begin, float Angle_end, float Nums = 15);
		// 勾选框
		void MyCheckBox(const char* str_id, bool* v);
		void MyCheckBox2(const char* str_id, bool* v);
		void MyCheckBox3(const char* str_id, bool* v);
		void MyCheckBox4(const char* str_id, bool* v);
		// 阴影矩形
		void ShadowRectFilled(Vec2 Pos, Vec2 Size, ImColor RectColor, ImColor ShadowColor, float ShadowThickness, Vec2 ShadowOffset, float Rounding = 0);
		// 阴影圆形
		void ShadowCircle(Vec2 Pos, float Radius, ImColor CircleColor, ImColor ShadowColor, float ShadowThickness, Vec2 ShadowOffset, float Num = 30);
	};
}

