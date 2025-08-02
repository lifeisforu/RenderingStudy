#pragma once

#include "Platform/PlatformWindows.h"
#include "Platform/ComPointer.h"

//-----------------------------------------------------------------------------
// Foward Declaration
//-----------------------------------------------------------------------------

class DXDebugLayer;

//-----------------------------------------------------------------------------
// RendererD3D12
//-----------------------------------------------------------------------------

/// render using D3D12.
class RendererD3D12
{
public:

	/// default constructor.
	RendererD3D12();

	/// destructor.
	~RendererD3D12();

	/// create debug layer.
	/// @return true if succeeded.
	bool CreateDebugLayer();

	/// create dxgi factory.
	/// @return true if succeeded.
	bool CreateDXGIFactory();

	/// enumerate dxgi adapters.
	/// @return adapters.
	TArray<DXGI_ADAPTER_DESC> EnumerateAdapters();

	/// select adapter.
	/// @return true if succeeded.
	bool SelectAdapter(const uint32 InAdapterIndex);

	/// create adapter by preference.
	/// @param InFeatureLevel feature level.
	/// @return true if succeeded.
	bool SelectAdapterByPreference(const DXGI_GPU_PREFERENCE InPreference);

	/// enumerate feature levels.
	/// @return feature levels.
	TArray<D3D_FEATURE_LEVEL> EnumerateFeatureLevels();

	/// create device.
	/// @param InFeatureLevel feature level.
	/// @return true if succeeded.
	bool CreateDevice(const D3D_FEATURE_LEVEL InFeatureLevel);

private:

	/// feature level.
	D3D_FEATURE_LEVEL _FeatureLevelD3D = D3D_FEATURE_LEVEL_1_0_GENERIC;

	/// D3D12 device.
	ComPointer<ID3D12Device14> _DeviceD3D12;

	/// DXGI adapter.
	ComPointer<IDXGIAdapter4> _AdapterDXGI;

	/// DXGI factory.
	ComPointer<IDXGIFactory7> _FactoryDXGI;

	/// debug layer.
	TUniquePtr<DXDebugLayer> _DebugLayerPtr;
};