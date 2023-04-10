#include "Win32ErrorHandling.h"
#include <system_error>

Win32Exception::Win32Exception(HRESULT hr) : std::runtime_error("Win32Exception::CLASS") {
	LPSTR errMsg = nullptr;
	DWORD len = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		hr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&errMsg,
		0,
		nullptr
	);

	if (len == 0) {
		message = "Unsuspicied!";
	}
	else {
		message = errMsg;
	}

	LocalFree(errMsg);
}

Win32Exception::Win32Exception(HRESULT hr, std::string msg) : Win32Exception(hr) {
	message = msg + "\nHRESULT: " + message;
}

const char* Win32Exception::what() const noexcept {
	return message.c_str();
}