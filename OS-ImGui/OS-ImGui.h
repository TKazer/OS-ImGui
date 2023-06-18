#pragma once
#include "OS-ImGui_External.h"

/****************************************************
* Copyright (C)	: Liv
* @file			: OS-ImGui.h
* @author		: Liv
* @email		: 1319923129@qq.com
* @version		: 1.0
* @date			: 2023/6/18	11:21
****************************************************/

namespace OSImGui
{
#ifdef _CONSOLE
	class OSImGui : public OSImGui_External, public Singleton<OSImGui>
#else
	class OSImGui : public OSImGui_Internal, public Singleton<OSImGui>
#endif
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

