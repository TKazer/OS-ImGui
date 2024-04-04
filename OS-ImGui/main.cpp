#include <iostream>
#include "OS-ImGui.h"

void DrawCallBack()
{
	ImGui::Begin("Menu");
	{
		ImGui::Text("This is a text.");
		if (ImGui::Button("Quit"))
		{
			Gui.Quit();
			//...
		}
		static bool a = false, b = false, c = false, d = false;
		static float Value = 0;
		float min = 0, max = 100;
		Gui.MyCheckBox("CheckBox1", &a);
		Gui.MyCheckBox2("CheckBox2", &b);
		Gui.MyCheckBox3("CheckBox3", &c);
		Gui.MyCheckBox4("CheckBox4", &d);
		Gui.SliderScalarEx1("[Slider]", ImGuiDataType_Float, &Value, &min, &max, "%.1f", ImGuiSliderFlags_None);
	}ImGui::End();

	Gui.ShadowRectFilled({ 50,50 }, { 100,100 }, ImColor(220, 190, 99, 255), ImColor(50, 50, 50, 255), 9, { 0,0 }, 10);
	Gui.ShadowCircle({ 200,200 }, 30, ImColor(220, 190, 99, 255), ImColor(50, 50, 50, 255), 9, { 0,0 });

	//...
}

/*
	NOTICE:
		If need change to internal mode, please define "OSIMGUI_INTERNAL" in Preprocessing, and change the project to DLL.
		Only surport for DirectX11 now.
	提示：
		如果需要使用internal版本，请在预处理器中定义 "OSIMGUI_INTERNAL"，并且将项目切换为DLL项目。
		目前只支持DirectX11。
*/

#ifdef OSIMGUI_INTERNAL

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		// Entry
		Gui.Start(hModule, DrawCallBack);
	}
	return TRUE;
}

#endif

#ifndef OSIMGUI_INTERNAL

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

#endif