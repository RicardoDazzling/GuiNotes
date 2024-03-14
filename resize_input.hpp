#include "imgui.h"


class ResizableInputTextMultiline {
private:
	ImVector<char>* str;
public:
	ResizableInputTextMultiline(ImVector<char>* str);
	bool Draw(const char* label, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0);
	int ResizeCallback(ImGuiInputTextCallbackData* data);
};
