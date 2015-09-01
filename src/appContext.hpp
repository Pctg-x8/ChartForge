#pragma once

#include <windows.h>
#include <memory>
#include <exception>
#include <string>

class InitializeException : public std::exception
{
	std::wstring source;
public:
	InitializeException(const std::wstring& s) : source(s){}
	virtual ~InitializeException() = default;
};

class AppContext
{
	friend AppContext* getCurrentContext();

	AppContext() = default;
	~AppContext() = default;

	HWND nativePointer;

	void init();
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
public:
	int runApplication();

	auto getNativePointer(){ return this->nativePointer; }
};

AppContext* getCurrentContext();