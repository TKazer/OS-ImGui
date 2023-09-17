#include <iostream>
#include "OS-ImGui.h"

void DrawCallBack()
{
	ImGui::Begin("Menu");
	{
		ImGui::Text("This is a text.");
		if (ImGui::Button("Button"))
		{
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