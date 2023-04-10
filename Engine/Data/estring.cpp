#include "estring.hpp"
#include <codecvt>

std::u32string convert(const char* str) {
	std::string w = str;
	return std::u32string(w.begin(), w.end());
}

std::u32string convert(const wchar_t* str) {
	std::wstring w = str;
	return std::u32string(w.begin(), w.end());
}

estring::estring(const char* e) : std::u32string(convert(e)) {
	
}

estring::estring(const wchar_t* e) : std::u32string(convert(e)) {
	
}
 
estring::estring(const char32_t* e) : std::u32string(e) {

}

estring::estring(const std::string& e) : std::u32string(e.begin(), e.end()) {

}

estring::estring(const std::u32string& e) : std::u32string(e) {

}

std::wostream& operator<<(std::wostream& o, const estring& e) {
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
	o << converter.to_bytes(e);
	return o;
}
