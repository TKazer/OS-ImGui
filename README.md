# OS-ImGui
A simple imgui library, develop based on [Dear-ImGui](https://github.com/ocornut/imgui).

Aim to make it easy for people to use imgui, which only takes few minutes.

Chinese -> [README-ZH](https://github.com/TKazer/OS-ImGui/blob/master/README-ZH.md)

## Feature
* Compatible for internal and external use, and easy to switch.
> If you want to use internal mode, just need define "OSIMGUI_INTERNAL" in preprocessing.

* Easy to use.
> Only one line of code is needed to call.

* MutipleDrawing
><img src = "https://github.com/TKazer/OS-ImGui/blob/master/Image/WindowImage.png" width = 400/>

## Example

1. External Mode
~~~ c++
int main()
{
	try {
		Gui.NewWindow("WindowName", Vec2(500, 500), DrawCallBack);
		//Gui.AttachAnotherWindow("Title","", DrawCallBack);
	}
	catch (OSImGui::OSException& e)
	{
		std::cout << e.what() << std::endl;
	}

	system("pause");
	return 0;
}
~~~

2. Internal Mode
~~~c++
// Define "OSIMGUI_INTERNAL" in preprocessing before use.

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		// Entry
		Gui.Start(hModule, DrawCallBack);
	}
	return TRUE;
}
~~~

