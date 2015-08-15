#pragma once

#include <comdef.h>

// HRESULT Wrapper
struct ComResult
{
	HRESULT hr;

	ComResult() { this->hr = S_OK; }
	ComResult(HRESULT hr)
	{
		this->hr = hr;
		if (FAILED(hr)) throw _com_error(hr);
	}
	
	ComResult& operator=(HRESULT hr)
	{
		this->hr = hr;
		if (FAILED(hr)) throw _com_error(hr);
		return *this;
	}
};