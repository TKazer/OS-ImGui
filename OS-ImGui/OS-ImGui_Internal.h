#pragma once
#include "OS-ImGui_Base.h"
#include "Minhook/MinHook.h"
#include <thread>

/****************************************************
* Copyright (C)	: Liv
* @file			: OS-ImGui_Internal.h
* @author		: Liv
* @email		: 1319923129@qq.com
* @version		: 1.1
* @date			: 2024/4/4 13:59
****************************************************/

namespace OSImGui
{
#ifdef OSIMGUI_INTERNAL

	class OSImGui_Internal : public OSImGui_Base
	{
	public:
		WindowType Type = INTERNAL;
		DirectXType DxType = DirectXType::AUTO;
		bool FreeDLL = false;
		HMODULE hLibModule = NULL;
		struct HookData_
		{
			std::vector<Address*> HookList;
		}HookData;
	public:
		void Start(HMODULE hLibModule ,std::function<void()> CallBack, DirectXType DxType = DirectXType::AUTO);
		void ReleaseHook();
	public:
		bool InitDx11(IDXGISwapChain* pSwapChain);
		void CleanDx11();
		bool InitDx11Hook();
	private:
		void InitThread();
		bool CreateMyWindow();
		void DestroyMyWindow();
	};

#endif 
}