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

	void Show(std::string Id, std::string Title, std::string fmt);
	void Show(std::string Id, std::string Title, std::string fmt, MsgBoxType type);
}
