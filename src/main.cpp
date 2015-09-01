#include "appContext.hpp"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	return getCurrentContext()->runApplication();
}