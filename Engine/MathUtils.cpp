#include "MathUtils.hpp"

bool MathUtil::ClipRect(const RECT& rect, RECT& clip) {
	if (clip.left < rect.left) {
		clip.left = rect.left;
	}
	if (clip.right > rect.right) {
		clip.right = rect.right;
	}
	if (clip.top < rect.top) {
		clip.top = rect.top;
	}
	if (clip.bottom > rect.bottom) {
		clip.bottom = rect.bottom;
	}

	return true;
}
