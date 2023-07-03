#pragma once
#include <string>

enum class MsgBoxType {
	NOTHING,
	OK,
	OKCANCEL,
	YESNO,
	YESNOCANCEL
};

namespace MsgBox {
	int GetResult(std::string Id);
	void Draw();

	// Provide a way to do ImGui MessageBox
	void Show(std::string Id, std::string Title, std::string fmt);
	void Show(std::string Id, std::string Title, std::string fmt, MsgBoxType type);

	// Provide a way to do OS MessageBox
	int ShowOut(std::string title, std::string fmt);
	int ShowOut(std::string title, std::string fmt, MsgBoxType type);
}
