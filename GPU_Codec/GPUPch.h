#pragma once

constexpr unsigned int NUM_BACKBUFFERS = 3;

#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <d2d1_3.h>
#include <d3d11.h>
#include <d3d11on12.h>
#include <dwrite.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")


#include <wrl.h>
using namespace Microsoft::WRL;

#include <comdef.h>
#include <stdexcept>
#include <vector>
#include <memory>
#include <stdio.h>

inline void TIF(HRESULT hr)
{
#ifdef _DEBUG
	if(FAILED(hr))
	{
		_com_error err(hr);
		CHAR s_str[256] = {};
		wprintf(L"%ls", err.ErrorMessage());

		throw std::runtime_error(s_str);
	}
#endif
}

// Assign a name to the object to aid with debugging.
#if defined(_DEBUG) || defined(DBG)
	inline void
	SetName(ID3D12Object* pObject, LPCWSTR name)
{
	pObject->SetName(name);
}
inline void SetNameIndexed(ID3D12Object* pObject, LPCWSTR name, UINT index)
{
	WCHAR fullName[50];
	if(swprintf_s(fullName, L"%s[%u]", name, index) > 0)
	{
		pObject->SetName(fullName);
	}
}
#else
	inline void
	SetName(ID3D12Object*, LPCWSTR)
{}
inline void SetNameIndexed(ID3D12Object*, LPCWSTR, UINT) {}
#endif

// Naming helper for ComPtr<T>.
// Assigns the name of the variable as the name of the object.
// The indexed variant will include the index in the name of the object.
#define NAME_D3D12_OBJECT(x) SetName((x).Get(), L#x)
#define NAME_D3D12_OBJECT_INDEXED(x, n) SetNameIndexed((x)[n].Get(), L#x, n)