#pragma once
#include <windows.h>
#include <string>
#include <stdexcept>
 
// If necessary, use this to free raw pointers. 
// Preferably, use Microsoft::WRL::ComPtr from client.h (http://msdn.microsoft.com/en-us/library/br244983.aspx)
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) \
   if(x != NULL)        \
   {                    \
      x->Release();     \
      x = NULL;         \
   }
#endif

class Win32Exception : public std::runtime_error {
public:
    Win32Exception(HRESULT hr);
    Win32Exception(HRESULT hr, std::string msg);

    const char* what() const noexcept override;

    static inline void ThrowIfError(HRESULT hr) {
        if (FAILED(hr)) {
            throw new Win32Exception(hr);
        }
    }
	
    static inline void ThrowIfError(HRESULT hr, std::string msg) {
        if (FAILED(hr)) {
            throw new Win32Exception(hr, msg);
        }
    }

private:
    std::string message;
};