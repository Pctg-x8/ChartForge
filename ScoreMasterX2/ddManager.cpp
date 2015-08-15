#include "ddManager.h"

#include "comutils.h"
#include "appContext.h"

CDropTarget::CDropTarget()
{
	this->refCount = 0;
}
CDropTarget::~CDropTarget() = default;

HRESULT CDropTarget::QueryInterface(const IID& iid, void** ppv)
{
	if (IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IDropTarget))
	{
		*ppv = static_cast<IDropTarget*>(this);
	}
	else
	{
		*ppv = nullptr;
		return E_NOINTERFACE;
	}
	this->AddRef();
	return S_OK;
}
ULONG CDropTarget::AddRef()
{
	this->refCount++;
	return this->refCount;
}
ULONG CDropTarget::Release()
{
	this->refCount--;
	if (this->refCount == 0) delete this;
	return this->refCount;
}
HRESULT CDropTarget::createInstance(IDropTarget** pp)
{
	if (pp != nullptr)
	{
		auto p = new CDropTarget();
		return p->QueryInterface(IID_IDropTarget, (void**)pp);
	}
	return E_INVALIDARG;
}

HRESULT CDropTarget::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	HRESULT hr;
	FORMATETC format = {};

	format.cfFormat = CF_HDROP;
	format.ptd = nullptr;
	format.dwAspect = DVASPECT_CONTENT;
	format.lindex = -1;
	format.tymed = TYMED_HGLOBAL;
	hr = pDataObj->QueryGetData(&format);
	if (FAILED(hr))
	{
		*pdwEffect = DROPEFFECT_NONE;
	}
	else
	{
		*pdwEffect = DROPEFFECT_LINK;
		getCurrentContext().getDragScreenOverlay()->raiseEffect();
	}

	OutputDebugString(L"DragEnter\n");
	return *pdwEffect == DROPEFFECT_NONE ? DRAGDROP_S_CANCEL : S_OK;
}
HRESULT CDropTarget::DragOver(DWORD grfkeyState, POINTL pt, DWORD* pdwEffect)
{
	OutputDebugString(L"DragOver\n");
	return S_OK;
}
HRESULT CDropTarget::DragLeave()
{
	OutputDebugString(L"DragLeave\n");
	getCurrentContext().getDragScreenOverlay()->falloffEffect();
	return S_OK;
}
HRESULT CDropTarget::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	HRESULT hr;
	FORMATETC format = {};
	STGMEDIUM medium = {};

	format.cfFormat = CF_HDROP;
	format.ptd = nullptr;
	format.dwAspect = DVASPECT_CONTENT;
	format.lindex = -1;
	format.tymed = TYMED_HGLOBAL;
	hr = pDataObj->QueryGetData(&format);
	if (FAILED(hr))
	{
		OutputDebugString(L"Format Error.\n");
		return hr;
	}
	hr = pDataObj->GetData(&format, &medium);
	if (FAILED(hr))
	{
		OutputDebugString(L"GetData Error.\n");
		return hr;
	}

	auto hDrop = static_cast<HDROP>(GlobalLock(medium.hGlobal));
	auto nDrops = DragQueryFile(hDrop, 0xffffffff, nullptr, 0);
	for (uint32_t i = 0; i < nDrops; i++)
	{
		auto fnSize = DragQueryFile(hDrop, i, nullptr, 0);
		std::unique_ptr<wchar_t[]> pFileName(new wchar_t[fnSize + 1]);
		DragQueryFile(hDrop, i, pFileName.get(), fnSize + 1);
		
		OutputDebugString(L"  Dropped File: ");
		OutputDebugString(pFileName.get());
		OutputDebugString(L"\n");
	}

	GlobalUnlock(medium.hGlobal);
	ReleaseStgMedium(&medium);

	OutputDebugString(L"Drop\n");
	getCurrentContext().getDragScreenOverlay()->falloffEffect();
	return S_OK;
}