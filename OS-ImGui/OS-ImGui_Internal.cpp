#include "OS-ImGui_Internal.h"
#include "OS-ImGui.h"

/****************************************************
* Copyright (C)	: Liv
* @file			: OS-ImGui_Internal.cpp
* @author		: Liv
* @email		: 1319923129@qq.com
* @version		: 1.1
* @date			: 2024/4/5 13:00
****************************************************/

#ifdef OSIMGUI_INTERNAL

typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
typedef HRESULT(__stdcall* Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef HRESULT(__stdcall* Resize)(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
typedef HRESULT(__stdcall* EndScene)(LPDIRECT3DDEVICE9);
typedef HRESULT(__stdcall* Reset)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

// DirectX 9
inline EndScene oEndScene;
inline Reset oReSet;
// DirectX 11
inline Present oPresent;
inline Resize oResize;

inline WNDPROC oWndProc = NULL;
inline bool IsDx11Init = false, IsDx9Init = false;

namespace OSImGui
{
	void OSImGui_Internal::Start(HMODULE hLibModule, std::function<void()> CallBack, DirectXType DxType)
	{
		this->DxType = DxType;

		this->CallBackFn = CallBack;

		this->hLibModule = hLibModule;

		MH_Initialize();

		std::thread(&OSImGui_Internal::InitThread, this).detach();
	}

	void OSImGui_Internal::ReleaseHook()
	{
		if (this->HookData.HookList.size() > 0)
			MH_DisableHook(MH_ALL_HOOKS);

		for (auto Target : this->HookData.HookList)
		{
			MH_RemoveHook(Target);
		}
		this->HookData.HookList.clear();

		MH_Uninitialize();
	}

	bool OSImGui_Internal::CreateMyWindow()
	{
		WNDCLASSEXA wc = { sizeof(wc), CS_HREDRAW | CS_VREDRAW, DefWindowProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "OS-ImGui_Internal", NULL };
		RegisterClassExA(&wc);

		this->Window.hInstance = wc.hInstance;

		this->Window.hWnd = ::CreateWindowA(wc.lpszClassName, "OS-ImGui_Overlay", WS_OVERLAPPEDWINDOW, 0, 0, 50, 50, NULL, NULL, wc.hInstance, NULL);

		this->Window.ClassName = wc.lpszClassName;

		return this->Window.hWnd != NULL;
	}

	void OSImGui_Internal::DestroyMyWindow()
	{
		if (this->Window.hWnd)
		{
			DestroyWindow(this->Window.hWnd);
			UnregisterClassA(this->Window.ClassName.c_str(), this->Window.hInstance);
			this->Window.hWnd = NULL;
		}
	}

	void OSImGui_Internal::InitThread()
	{
		if (this->DxType == DirectXType::AUTO)
		{
			if (GetModuleHandleA("d3d11.dll"))
				this->DxType = DirectXType::DX11;
			else if (GetModuleHandleA("d3d9.dll"))
				this->DxType = DirectXType::DX9;
			else
				goto END;
		}

		do
		{
			if (this->DxType == DirectXType::DX11)
			{
				if (this->InitDx11Hook())
				{
					break;
				}
			}
			else if(this->DxType == DirectXType::DX9)
			{
				if (this->InitDx9Hook())
				{
					break;
				}
			}
			else
			{
				goto END;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(100));

		} while (true);

		while (true)
		{
			if (this->FreeDLL)
			{
			END:
				FreeLibrary(this->hLibModule);
				return;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
			return true;

		return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
	}

	HRESULT __stdcall hkResize(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
	{
		if (g_Device.g_mainRenderTargetView)
		{
			g_Device.g_pd3dDeviceContext->OMSetRenderTargets(0, 0, 0);
			g_Device.g_mainRenderTargetView->Release();
		}

		HRESULT Result = oResize(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

		ID3D11Texture2D* pBackBuffer = nullptr;

		pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
		g_Device.g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_Device.g_mainRenderTargetView);
		pBackBuffer->Release();

		g_Device.g_pd3dDeviceContext->OMSetRenderTargets(1, &g_Device.g_mainRenderTargetView, NULL);

		D3D11_VIEWPORT ViewPort{ 0,0,Width,Height,0.0f,1.0f };

		g_Device.g_pd3dDeviceContext->RSSetViewports(1, &ViewPort);

		return Result;
	}

	HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
	{
		if (!IsDx11Init)
		{
			if (OSImGui::get().InitDx11(pSwapChain))
				IsDx11Init = true;
			else
				return oPresent(pSwapChain, SyncInterval, Flags);

			oWndProc = (WNDPROC)SetWindowLongPtrA(OSImGui::get().Window.hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
			
			try {
				OSImGui::get().InitImGui(g_Device.g_pd3dDevice, g_Device.g_pd3dDeviceContext);
			}
			catch (OSException& e)
			{
				OSImGui::get().CleanDx11();
				IsDx11Init = false;

				std::cout << e.what() << std::endl;
				return oPresent(pSwapChain, SyncInterval, Flags);
			}
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		OSImGui::get().CallBackFn();

		ImGui::Render();

		g_Device.g_pd3dDeviceContext->OMSetRenderTargets(1, &g_Device.g_mainRenderTargetView, NULL);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		if (OSImGui::get().EndFlag)
		{
			oPresent(pSwapChain, SyncInterval, Flags);

			OSImGui::get().ReleaseHook();
			OSImGui::get().CleanImGui();
			OSImGui::get().CleanDx11();

			oWndProc = (WNDPROC)SetWindowLongPtrA(OSImGui::get().Window.hWnd, GWLP_WNDPROC, (LONG_PTR)(oWndProc));

			OSImGui::get().FreeDLL = true;

			return NULL;
		}

		return oPresent(pSwapChain, SyncInterval, Flags);
	}

	void OSImGui_Internal::CleanDx11()
	{
		if (g_Device.g_pd3dDevice)
			g_Device.g_pd3dDevice->Release();
		if (g_Device.g_pd3dDeviceContext)
			g_Device.g_pd3dDeviceContext->Release();
		if (g_Device.g_mainRenderTargetView)
			g_Device.g_mainRenderTargetView->Release();
	}

	bool OSImGui_Internal::InitDx11(IDXGISwapChain* pSwapChain)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&g_Device.g_pd3dDevice))))
		{
			g_Device.g_pd3dDevice->GetImmediateContext(&g_Device.g_pd3dDeviceContext);

			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);

			this->Window.hWnd = sd.OutputWindow;

			this->Window.Size = Vec2((float)sd.BufferDesc.Width, (float)sd.BufferDesc.Height);

			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			g_Device.g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_Device.g_mainRenderTargetView);

			pBackBuffer->Release();

			return true;
		}
		return false;
	}

	bool OSImGui_Internal::InitDx11Hook()
	{
		if (!this->CreateMyWindow())
			return false;

		HMODULE hModule = NULL;

		void* D3D11CreateDeviceAndSwapChain;
		D3D_FEATURE_LEVEL FeatureLevel;
		D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_11_0 };
		DXGI_RATIONAL RefreshRate;
		DXGI_MODE_DESC ModeDesc;
		DXGI_SAMPLE_DESC SampleDesc;
		DXGI_SWAP_CHAIN_DESC SwapChainDesc;
		IDXGISwapChain* SwapChain;
		ID3D11Device* Device;
		ID3D11DeviceContext* Context;

		hModule = GetModuleHandleA("d3d11.dll");

		if (hModule == NULL)
		{
			this->DestroyMyWindow();
			return false;
		}

		D3D11CreateDeviceAndSwapChain = GetProcAddress(hModule, "D3D11CreateDeviceAndSwapChain");

		if (D3D11CreateDeviceAndSwapChain == 0)
		{
			this->DestroyMyWindow();
			return false;
		}
		// Datas init.
		RefreshRate.Numerator = 60;
		RefreshRate.Denominator = 1;
		SampleDesc.Count = 1;
		SampleDesc.Quality = 0;

		ModeDesc.Width = 100;
		ModeDesc.Height = 100;
		ModeDesc.RefreshRate = RefreshRate;
		ModeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		ModeDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		ModeDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		SwapChainDesc.BufferDesc = ModeDesc;
		SwapChainDesc.SampleDesc = SampleDesc;
		SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapChainDesc.BufferCount = 1;
		SwapChainDesc.OutputWindow = this->Window.hWnd;
		SwapChainDesc.Windowed = 1;
		SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		// Create D3D11 device and swapchain.
		if (((long(__stdcall*)(
			IDXGIAdapter*,
			D3D_DRIVER_TYPE,
			HMODULE,
			UINT,
			const D3D_FEATURE_LEVEL*,
			UINT,
			UINT,
			const DXGI_SWAP_CHAIN_DESC*,
			IDXGISwapChain**,
			ID3D11Device**,
			D3D_FEATURE_LEVEL*,
			ID3D11DeviceContext**))(D3D11CreateDeviceAndSwapChain))(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, FeatureLevels, 1, D3D11_SDK_VERSION, &SwapChainDesc, &SwapChain, &Device, &FeatureLevel, &Context) < 0)
		{
			this->DestroyMyWindow();
			return false;
		}

		Address* Vtable = reinterpret_cast<Address*>(calloc(205, sizeof(Address)));

		memcpy(Vtable, *reinterpret_cast<Address**>(SwapChain), sizeof(Address) * 18);
		memcpy(Vtable + 18, *reinterpret_cast<Address**>(Device), sizeof(Address) * 43);
		memcpy(Vtable + 61, *reinterpret_cast<Address**>(Context), sizeof(Address) * 144);

		SwapChain->Release();
		Device->Release();
		Context->Release();

		this->DestroyMyWindow();

		Address* pTarget = nullptr;

		pTarget = reinterpret_cast<Address*>(Vtable[8]);
		if (MH_CreateHook(pTarget, hkPresent, (void**)(&oPresent)) == MH_OK)
			this->HookData.HookList.push_back(pTarget);

		pTarget = reinterpret_cast<Address*>(Vtable[13]);
		if (MH_CreateHook(pTarget, hkResize, (void**)(&oResize)) == MH_OK)
			this->HookData.HookList.push_back(pTarget);

		free(Vtable);

		if (MH_EnableHook(MH_ALL_HOOKS) == MH_OK)
			return true;

		return false;
	}

	HRESULT __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice)
	{
		if (!IsDx9Init)
		{
			if(OSImGui::get().InitDx9(pDevice))
				IsDx9Init = true;
			else
				return oEndScene(pDevice);
		}

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		OSImGui::get().CallBackFn();

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		if (OSImGui::get().EndFlag)
		{
			oEndScene(pDevice);

			OSImGui::get().ReleaseHook();
			OSImGui::get().CleanDx9();

			oWndProc = (WNDPROC)SetWindowLongPtrA(OSImGui::get().Window.hWnd, GWLP_WNDPROC, (LONG_PTR)(oWndProc));

			OSImGui::get().FreeDLL = true;

			return NULL;
		}

		return oEndScene(pDevice);
	}

	HRESULT __stdcall hkReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPrams)
	{
		if (IsDx9Init)
		{
			OSImGui::get().CleanDx9();
			IsDx9Init = false;
		}

		return oReSet(pDevice, pPrams);
	}

	BOOL CALLBACK EnumWindowsCallback(HWND hWnd, LPARAM lParam)
	{
		DWORD wndProcId;
		GetWindowThreadProcessId(hWnd, &wndProcId);

		if (GetCurrentProcessId() != wndProcId)
			return TRUE;

		OSImGui::get().Window.hWnd = hWnd;
		return FALSE;
	}

	HWND GetProcessWindow()
	{
		OSImGui::get().Window.hWnd = NULL;
		EnumWindows(EnumWindowsCallback, NULL);
		return OSImGui::get().Window.hWnd;
	}

	bool OSImGui_Internal::InitDx9(LPDIRECT3DDEVICE9 pDevice)
	{
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImGui::StyleColorsDark();
		io.LogFilename = nullptr;

		if (!ImGui_ImplWin32_Init(Window.hWnd))
			throw OSException("ImGui_ImplWin32_Init() call failed.");
		if (!ImGui_ImplDX9_Init(pDevice))
			throw OSException("ImGui_ImplDX9_Init() call failed.");

		return true;
	}

	void OSImGui_Internal::CleanDx9()
	{
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	bool OSImGui_Internal::InitDx9Hook()
	{
		if (!this->CreateMyWindow())
			return false;

		HMODULE hModule = NULL;

		void* PDirect3DCreate9;
		LPDIRECT3DDEVICE9 Device;
		IDirect3D9* PDirect3D9;
		D3DDISPLAYMODE DisplayMode;
		D3DPRESENT_PARAMETERS D3dParam;

		// Get d3d module address
		hModule = GetModuleHandleA("d3d9.dll");
		if (hModule == NULL)
		{
			this->DestroyMyWindow();
			return false;
		}

		PDirect3DCreate9 = GetProcAddress(hModule, "Direct3DCreate9");
		if (PDirect3DCreate9 == NULL)
		{
			this->DestroyMyWindow();
			return false;
		}

		PDirect3D9 = ((LPDIRECT3D9(__stdcall*)(DWORD))(PDirect3DCreate9))(D3D_SDK_VERSION);
		if (PDirect3D9 == NULL)
		{
			this->DestroyMyWindow();
			return false;
		}

		if (PDirect3D9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &DisplayMode) < 0)
		{
			this->DestroyMyWindow();
			return false;
		}

		D3dParam = { 0,0,DisplayMode.Format,0,D3DMULTISAMPLE_NONE,NULL,D3DSWAPEFFECT_DISCARD,this->Window.hWnd,1,0,D3DFMT_UNKNOWN,NULL,0,0 };

		// Create d3d device
		if (PDirect3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, this->Window.hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT, &D3dParam, &Device) < 0)
		{
			this->DestroyMyWindow();
			PDirect3D9->Release();
			return false;
		}

		Address* Vtable = reinterpret_cast<Address*>(calloc(119, sizeof(Address)));
		memcpy(Vtable, *reinterpret_cast<Address**>(Device), 119 * sizeof(Address));

		PDirect3D9->Release();
		Device->Release();
		this->DestroyMyWindow();

		Address* pTarget = nullptr;

		pTarget = reinterpret_cast<Address*>(Vtable[42]);

		if (MH_CreateHook(pTarget, hkEndScene, (void**)(&oEndScene)) == MH_OK)
			this->HookData.HookList.push_back(pTarget);

		pTarget = reinterpret_cast<Address*>(Vtable[16]);

		if (MH_CreateHook(pTarget, hkReset, (void**)(&oReSet)) == MH_OK)
			this->HookData.HookList.push_back(pTarget);

		free(Vtable);

		if (GetProcessWindow() == NULL)
		{
			if (this->HookData.HookList.size() > 0)
			{
				MH_RemoveHook(this->HookData.HookList.at(0));
			}
			return false;
		}

		if (MH_EnableHook(MH_ALL_HOOKS) == MH_OK)
			return true;

		return false;
	}
}

#endif