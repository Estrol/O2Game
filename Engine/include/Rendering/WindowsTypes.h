#pragma once

// Windows type replacement for 'DWORD'
typedef unsigned long DWORD;

// Windows type replacement for 'BOOL'
typedef int BOOL;

#ifndef NULL
#define NULL 0
#endif

#include <windows.h>

struct RectF {
	float left;
	float top;
	float right;
	float bottom;
};

struct Rect {
	int left;
	int top;
	int right;
	int bottom;
};

constexpr RectF ToRectF(Rect rc) {
	return RectF{ (float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom };
}

constexpr Rect RectZero = { 0, 0, 0, 0 };
constexpr RectF RectFZero = { 0.0f, 0.0f, 0.0f, 0.0f };