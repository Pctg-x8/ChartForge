#pragma once

// Debugger Library

#include <windows.h>
#include <exception>
#include <string>
#include <fstream>

class DebuggerInitializationException : public std::exception
{
	std::wstring reason;
public:
	DebuggerInitializationException(const std::wstring& r) : reason(r){}
	virtual ~DebuggerInitializationException() = default;
};

class Debugger
{
	std::wofstream fp;

	template<typename T>
	class AtomicWriter
	{
	public:
		static void call(Debugger* pd, const T& val)
		{
			pd->fp << val;
		}
	};
	template<typename Tp>
	class AtomicWriter<Tp*>
	{
	public:
		static void call(Debugger* pd, const Tp*& val)
		{
			wchar_t temp[16] = {0};
			wsprintf(temp, L"0x%08x", val);
			pd->fp << std::wstring(temp);
		}
	};

	template<typename FirstT, typename... RestT>
	void writeImpl(uint32_t d, const FirstT& first, const RestT&... rest)
	{
		AtomicWriter<FirstT>::call(this, first);
		this->writeImpl(0, rest...);
	}
	void writeImpl(uint32_t d){}
public:
	Debugger();
	~Debugger();

	template<typename... Args>
	void write(const Args&... args)
	{
		this->writeImpl(0, args...);
	}
	template<typename... Args>
	void writeln(const Args&... args)
	{
		this->writeImpl(0, args..., L"\n");
	}
};

Debugger* getActiveDebugger();
