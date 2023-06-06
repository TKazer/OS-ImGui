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
		OSImGui::OSImGui::get().MyCheckBox("CheckBox1", &a);
		OSImGui::OSImGui::get().MyCheckBox2("CheckBox2", &b);
		OSImGui::OSImGui::get().MyCheckBox3("CheckBox3", &c);
		OSImGui::OSImGui::get().MyCheckBox4("CheckBox4", &d);
	}ImGui::End();

	OSImGui::OSImGui::get().ShadowRectFilled({ 50,50 }, { 100,100 }, ImColor(220, 190, 99, 255), ImColor(50, 50, 50, 255), 9, { 0,0 }, 10);
	OSImGui::OSImGui::get().ShadowCircle({ 200,200 }, 30, ImColor(220, 190, 99, 255), ImColor(50, 50, 50, 255), 9, { 0,0 });
	//...
}

int main()
{
	try {
		OSImGui::OSImGui::get().NewWindow("WindowName", Vec2(500, 500), DrawCallBack);
		//OSImGui::OSImGui::get().AttachAnotherWindow("Title","", DrawCallBack);
	}
	catch (OSImGui::OSException& e)
	{
		std::cout << e.what() << std::endl;
	}

	system("pause");
	return 0;
}