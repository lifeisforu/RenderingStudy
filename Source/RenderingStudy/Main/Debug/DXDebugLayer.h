#pragma once

#include "Platform/PlatformWindows.h"
#include "Platform/ComPointer.h"

//-----------------------------------------------------------------------------
// DXDebugLayer
//-----------------------------------------------------------------------------

class DXDebugLayer
{
public:

	// default constructor.
	DXDebugLayer() = default;

	// copy constructor.
	DXDebugLayer(const DXDebugLayer&) = delete;

	// copy operator.
	DXDebugLayer& operator=(const DXDebugLayer&) = delete;

	// get singletone instance.
	inline static DXDebugLayer& Get()
	{
		static DXDebugLayer sInst;
		return sInst;
	}

public:

	// initialize.
	bool Init();

	// deinitialize.
	void Deinit();

private:

	#if defined(RS_ENABLE_DX_DEBUG)
	// d3d12 debug.
	ComPointer<ID3D12Debug6> DebugD3D12;

	// dxgi debug.
	ComPointer<IDXGIDebug1> DebugDxgi;
	#endif // RS_ENABLE_DX_DEBUG
};