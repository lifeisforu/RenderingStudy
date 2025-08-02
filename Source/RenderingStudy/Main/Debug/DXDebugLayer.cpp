#include "DXDebugLayer.h"

//-----------------------------------------------------------------------------
// DXDebugLayer
//-----------------------------------------------------------------------------

bool DXDebugLayer::Init()
{
	#if defined(RS_ENABLE_DX_DEBUG)
	// init d3d12 debug layer.
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&_DebugD3D12))))
	{
		_DebugD3D12->EnableDebugLayer();

		// init dxgi debug.
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&_DebugDxgi))))
		{
			_DebugDxgi->EnableLeakTrackingForThread();
			return true;
		}
	}
	#endif // RS_ENABLE_DX_DEBUG

	return false;
}

void DXDebugLayer::Deinit()
{
	#if defined(RS_ENABLE_DX_DEBUG)
	if (_DebugDxgi != nullptr)
	{
		OutputDebugString(L"===== Begin reporting DXGI live objects:\n");
		_DebugDxgi->ReportLiveObjects(DXGI_DEBUG_ALL, 
			DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		OutputDebugString(L"===== End reporting DXGI live objects\n");
	}

	_DebugDxgi.Release();
	_DebugD3D12.Release();
	#endif // RS_ENABLE_DX_DEBUG
}