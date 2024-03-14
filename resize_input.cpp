#include "resize_input.hpp"


int ResizableInputTextMultiline::ResizeCallback(ImGuiInputTextCallbackData* data)
{
	this->str = (ImVector<char>*)data->UserData;
	IM_ASSERT(data->EventFlag == ImGuiInputTextFlags_CallbackResize);
	ImVector<char>* text = this->str;
	if (data->BufTextLen == 0)
	{
		text->clear();
		text->push_back('\0');
	}
	else
	{
		text->resize(data->BufTextLen);
	}
	data->Buf = text->begin();
	return 0;
}

ResizableInputTextMultiline::ResizableInputTextMultiline(ImVector<char>* str)
{
	this->str = str;
}

bool ResizableInputTextMultiline::Draw(const char* label, const ImVec2& size, ImGuiInputTextFlags flags)
{
	IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
	return ImGui::InputTextMultiline(label, this->str->begin(), (size_t)this->str->size(), size, flags | ImGuiInputTextFlags_CallbackResize,
		[](ImGuiInputTextCallbackData* data) {
			return static_cast<ResizableInputTextMultiline*>(data->UserData)->ResizeCallback(data);
		}, this);

}
