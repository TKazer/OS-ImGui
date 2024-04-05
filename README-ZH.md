
# OS-ImGui
一个简单的imgui库，基于[Dear-ImGui](https://github.com/ocornut/imgui).

旨在让人简单的上手imgui，仅需几分钟。

English -> [README-EN](https://github.com/TKazer/OS-ImGui/blob/master/README.md)

## 特点
* 同时兼容内外部使用，切换方便快捷
> 如果你想使用内部版本, 只需要定义 "OSIMGUI_INTERNAL" 在预处理代码中.

* 易于使用
> Only one line of code is needed to call.

* 多样绘制
><img src = "https://github.com/TKazer/OS-ImGui/blob/master/Image/WindowImage.png" width = 400/>

## 示例

1. 外部模式
~~~ c++
int main()
{
	try {
		/*
		    新建窗口。
		*/
		Gui.NewWindow("WindowName", Vec2(500, 500), DrawCallBack);
		/*
		    通过指定窗口名或类名附加到窗口。
		*/
		Gui.AttachAnotherWindow("Title","", DrawCallBack);
	}
	catch (OSImGui::OSException& e)
	{
		std::cout << e.what() << std::endl;
	}

	system("pause");
	return 0;
}
~~~

2. 内部模式
~~~c++
// 使用前请定义"OSIMGUI_INTERNAL" 在预处理代码中。

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		// 入口
		// 自动识别DirectX类型。
		Gui.Start(hModule, DrawCallBack, OSImGui::DirectXType::AUTO);
	}
	return TRUE;
}
~~~

