#include "RendererD3D12.h"

#include "Debug/Log.h"
#include "Debug/DXDebugLayer.h"

//-----------------------------------------------------------------------------
// static
//-----------------------------------------------------------------------------

static const TCHAR* ConvertFeatureLevelToString(const D3D_FEATURE_LEVEL InFeatureLevel)
{
	switch (InFeatureLevel)
	{
	case D3D_FEATURE_LEVEL_1_0_GENERIC:
		return TEXT("D3D_FEATURE_LEVEL_1_0_GENERIC");
	case D3D_FEATURE_LEVEL_1_0_CORE:
		return TEXT("D3D_FEATURE_LEVEL_1_0_CORE");
	case D3D_FEATURE_LEVEL_9_1:
		return TEXT("D3D_FEATURE_LEVEL_9_1");
	case D3D_FEATURE_LEVEL_9_2:
		return TEXT("D3D_FEATURE_LEVEL_9_2");
	case D3D_FEATURE_LEVEL_9_3:
		return TEXT("D3D_FEATURE_LEVEL_9_3");
	case D3D_FEATURE_LEVEL_10_0:
		return TEXT("D3D_FEATURE_LEVEL_10_0");
	case D3D_FEATURE_LEVEL_10_1:
		return TEXT("D3D_FEATURE_LEVEL_10_1");
	case D3D_FEATURE_LEVEL_11_0:
		return TEXT("D3D_FEATURE_LEVEL_11_0");
	case D3D_FEATURE_LEVEL_11_1:
		return TEXT("D3D_FEATURE_LEVEL_11_1");
	case D3D_FEATURE_LEVEL_12_0:
		return TEXT("D3D_FEATURE_LEVEL_12_0");
	case D3D_FEATURE_LEVEL_12_1:
		return TEXT("D3D_FEATURE_LEVEL_12_1");
	case D3D_FEATURE_LEVEL_12_2:
		return TEXT("D3D_FEATURE_LEVEL_12_2");
	default:
		break;
	}

	return TEXT("UNKNOWN_FEATURE_LEVEL");
}

//-----------------------------------------------------------------------------
// RendererD3D12
//-----------------------------------------------------------------------------

RendererD3D12::RendererD3D12()
{

}

RendererD3D12::~RendererD3D12()
{
	// If you properly control declaration orders of smart pointers, bellow logic is unnessesary.
	// But in most cases, the orders are incorrect, so I manually relese objects.

	_DeviceD3D12.Release();
	_AdapterDXGI.Release();
	_FactoryDXGI.Release();
	_DebugLayerPtr.reset(nullptr);
}

bool RendererD3D12::CreateDebugLayer()
{
	if (_DebugLayerPtr != nullptr)
	{
		LOG(Warning, TEXT("DebugLayer is already created."));
		return true;
	}

	_DebugLayerPtr = MakeUniquePtr<DXDebugLayer>();
	return _DebugLayerPtr->Init();
}

bool RendererD3D12::CreateDXGIFactory()
{
	HRESULT hr = CreateDXGIFactory2(
		(_DebugLayerPtr != nullptr) ? DXGI_CREATE_FACTORY_DEBUG : 0,
		IID_IDXGIFactory7,
		reinterpret_cast<void**>(&_FactoryDXGI));
	if (FAILED(hr))
	{
		ASSERT_HRESULT(hr, TEXT("Failed to create dxgi factory."));
	}
	return (hr == S_OK);
}

TArray<DXGI_ADAPTER_DESC> RendererD3D12::EnumerateAdapters()
{
	TArray<DXGI_ADAPTER_DESC> adapterDescs;

	if (_FactoryDXGI == nullptr)
	{
		ASSERT(_FactoryDXGI != nullptr, TEXT("You must create dxgi factory first. Call RendererD3D12::CreateDXGIFactory()."));
		return adapterDescs;
	}

	LOG(Info, TEXT("enumerate DXGI adapters..."));

	UINT adapterIndex = 0;
	ComPointer<IDXGIAdapter> adapterDXGI;
	while (_FactoryDXGI->EnumAdapters(adapterIndex++, &adapterDXGI) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC adapterDesc;
		if (SUCCEEDED(adapterDXGI->GetDesc(&adapterDesc)))
		{
			LOG(Info, TEXT("\t[{0}] {1}."), adapterIndex - 1, adapterDesc.Description);
		}
		else
		{
			LOG(Error, TEXT("Failed to get description from adapter"));
		}
		adapterDescs.push_back(adapterDesc);
		adapterDXGI.Release();
	}

	return adapterDescs;
}

bool RendererD3D12::SelectAdapter(const uint32 InAdapterIndex)
{
	ComPointer<IDXGIAdapter> adapterDXGI;
	HRESULT hr = _FactoryDXGI->EnumAdapters(InAdapterIndex, &adapterDXGI);
	if (FAILED(hr))
	{
		ASSERT_HRESULT(hr, FORMAT(TEXT("Failed to select adapter index : {0}."), InAdapterIndex));
		return false;
	}

	hr = adapterDXGI->QueryInterface(IID_IDXGIAdapter4, reinterpret_cast<void**>(&_AdapterDXGI));
	if (FAILED(hr))
	{
		ASSERT_HRESULT(hr, TEXT("Failed to select DXGIAdapter4 interface."));
		return false;
	}
	_AdapterDXGI->AddRef();

	DXGI_ADAPTER_DESC adapterDesc;
	if (SUCCEEDED(_AdapterDXGI->GetDesc(&adapterDesc)))
	{
		LOG(Info, TEXT("DXGI Adapter is selected : {0}."), adapterDesc.Description);
	}
	else
	{
		LOG(Error, TEXT("Failed to get description from adapter"));
	}

	return true;
}

bool RendererD3D12::SelectAdapterByPreference(const DXGI_GPU_PREFERENCE InPreference)
{
	HRESULT hr = _FactoryDXGI->EnumAdapterByGpuPreference(0, InPreference,
		IID_IDXGIAdapter4,
		reinterpret_cast<void**>(&_AdapterDXGI));
	if (FAILED(hr))
	{
		ASSERT_HRESULT(hr, TEXT("Failed to enumerate adapter by preference."));
		return false;
	}

	DXGI_ADAPTER_DESC adapterDesc;
	if (SUCCEEDED(_AdapterDXGI->GetDesc(&adapterDesc)))
	{
		LOG(Info, TEXT("DXGI Adapter is selected : {0}."), adapterDesc.Description);
	}
	else
	{
		LOG(Error, TEXT("Failed to get description from adapter"));
	}

	return true;
}


TArray<D3D_FEATURE_LEVEL> RendererD3D12::EnumerateFeatureLevels()
{
	TArray<D3D_FEATURE_LEVEL> featureLevels;
	if (_AdapterDXGI == nullptr)
	{
		ASSERT(_AdapterDXGI != nullptr, TEXT("You must select an adapter first. See SelectAdapterXXX()."));
		return featureLevels;
	}

	D3D_FEATURE_LEVEL availableLevels[] =
	{
		D3D_FEATURE_LEVEL_1_0_GENERIC,
		D3D_FEATURE_LEVEL_1_0_CORE,
		D3D_FEATURE_LEVEL_9_1,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_2
	};

	LOG(Info, TEXT("enumerate D3D feature levels..."));

	const int32 numAvailableLevels = sizeof(availableLevels) / sizeof(D3D_FEATURE_LEVEL);
	featureLevels.reserve(numAvailableLevels);
	for (int32 i = 0; i < numAvailableLevels; ++i)
	{
		ComPointer<ID3D12Device14> deviceD3D12Ptr;
		D3D_FEATURE_LEVEL featureLevel = availableLevels[i];
		HRESULT hr = D3D12CreateDevice(_AdapterDXGI, featureLevel,
			IID_ID3D12Device14, reinterpret_cast<void**>(&deviceD3D12Ptr));
		if (SUCCEEDED(hr))
		{
			featureLevels.push_back(featureLevel);
			deviceD3D12Ptr.Release();

			LOG(Info, TEXT("\t{0}."), ConvertFeatureLevelToString(featureLevel));
		}
	}

	return featureLevels;
}


bool RendererD3D12::CreateDevice(const D3D_FEATURE_LEVEL InFeatureLevel)
{
	_FeatureLevelD3D = InFeatureLevel;
	HRESULT hr = D3D12CreateDevice(_AdapterDXGI, _FeatureLevelD3D,
		IID_ID3D12Device14, reinterpret_cast<void**>(&_DeviceD3D12));
	if (FAILED(hr))
	{
		ASSERT_HRESULT(hr, FORMAT(TEXT("Failed to create device with {0}"), ConvertFeatureLevelToString(InFeatureLevel)));
		return false;
	}

	LOG(Info, TEXT("Create D3D12 Device with {0}"), ConvertFeatureLevelToString(InFeatureLevel));

	return true;
}