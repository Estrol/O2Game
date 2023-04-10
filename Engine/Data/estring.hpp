#pragma once
#include <iostream>

class estring : public std::u32string {
public:
	estring(const char*);
	estring(const wchar_t*);
	estring(const char32_t*);
	estring(const std::string&);
	estring(const std::u32string&);
	
	estring(const estring&) = default;
	estring() = default;

	friend std::wostream& operator<<(std::wostream& o, const estring& e);
};