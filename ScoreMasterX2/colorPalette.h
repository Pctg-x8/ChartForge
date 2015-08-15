#pragma once

#include <d2d1.h>

namespace ColorPalette
{
	const auto AppBackground = D2D1::ColorF(D2D1::ColorF::White);
	const auto ButtonSelectedFill = D2D1::ColorF(0.125f, 0.5f, 1.0f, 0.25f);
	const auto ButtonClickableBorder = D2D1::ColorF(0.0625f, 0.25f, 0.75f, 0.75f);

	const auto ScrollingThumbBack = D2D1::ColorF(0xc0c0c0);
	const auto ScrollUnitButtonBase = D2D1::ColorF(0xe0e0e0);
}