#include "appContext.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	return getCurrentContext().runApplication();
}