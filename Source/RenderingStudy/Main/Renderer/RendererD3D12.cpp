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
		ASSERT_MSG(_FactoryDXGI != nullptr, TEXT("You must create dxgi factory first. Call RendererD3D12::CreateDXGIFactory()."));
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
		ASSERT_HRESULT(hr, TEXT("Failed to select adapter index : {0}."), InAdapterIndex);
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
		ASSERT_MSG(_AdapterDXGI != nullptr, TEXT("You must select an adapter first. See SelectAdapterXXX()."));
		return featureLevels;
	}

	LOG(Info, TEXT("enumerate D3D feature levels..."));

	featureLevels.reserve(gNumKnownFeatureLevels);
	for (int32 i = 0; i < gNumKnownFeatureLevels; ++i)
	{
		ComPointer<ID3D12Device10> deviceD3D12Ptr;
		D3D_FEATURE_LEVEL featureLevel = gKnownFeatureLevels[i];
		HRESULT hr = D3D12CreateDevice(_AdapterDXGI, featureLevel,
			IID_ID3D12Device10, reinterpret_cast<void**>(&deviceD3D12Ptr));
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
		IID_ID3D12Device10, reinterpret_cast<void**>(&_DeviceD3D12));
	if (FAILED(hr))
	{
		ASSERT_HRESULT(hr, TEXT("Failed to create device with minimum feature level {0}"), ConvertFeatureLevelToString(InFeatureLevel));
		return false;
	}

	LOG(Info, TEXT("D3D12 Device is created with minimum feature level {0}"), ConvertFeatureLevelToString(InFeatureLevel));

	_CheckFeatures();

	return true;
}

void RendererD3D12::_CheckFeatures()
{
	if (_DeviceD3D12 == nullptr)
	{
		ASSERT_MSG(_DeviceD3D12 != nullptr, TEXT("Create a device first. See CreateDevice()."));
		return;
	}

	HRESULT hr;
	_DeviceFeatures = {};

	#define CHECK_FEATURE(InName, OutSucceeded) \
		hr = _DeviceD3D12->CheckFeatureSupport(D3D12_FEATURE_ ##InName, &_DeviceFeatures._ ##InName, sizeof(D3D12_FEATURE_DATA_ ##InName));\
		if (FAILED(hr)) { /*LOG_HRESULT(hr, TEXT("Failed to get feature : {0}."), TEXT(#InName));*/ OutSucceeded = false; } else { OutSucceeded = true; }

	#define CHECK_FEATURE_FOR(InName, InVar, InText, OutSucceeded) \
		hr = _DeviceD3D12->CheckFeatureSupport(D3D12_FEATURE_ ##InName, &InVar, sizeof(D3D12_FEATURE_DATA_ ##InName));\
		if (FAILED(hr)) { /*LOG_HRESULT(hr, TEXT("Failed to get feature : {0} for {1}."), TEXT(#InName), InText);*/ OutSucceeded = false; } else { OutSucceeded = true; }

	bool result;
	CHECK_FEATURE(D3D12_OPTIONS, result);
	CHECK_FEATURE(D3D12_OPTIONS1, result);
	CHECK_FEATURE(D3D12_OPTIONS2, result);
	CHECK_FEATURE(D3D12_OPTIONS3, result);
	CHECK_FEATURE(D3D12_OPTIONS4, result);
	CHECK_FEATURE(D3D12_OPTIONS5, result);
	CHECK_FEATURE(D3D12_OPTIONS6, result);
	CHECK_FEATURE(D3D12_OPTIONS7, result);
	CHECK_FEATURE(D3D12_OPTIONS8, result);
	CHECK_FEATURE(D3D12_OPTIONS9, result);
	CHECK_FEATURE(D3D12_OPTIONS10, result);
	CHECK_FEATURE(D3D12_OPTIONS11, result);
	CHECK_FEATURE(D3D12_OPTIONS12, result);
	CHECK_FEATURE(D3D12_OPTIONS13, result);
	CHECK_FEATURE(D3D12_OPTIONS19, result);
	CHECK_FEATURE(D3D12_OPTIONS20, result);
	CHECK_FEATURE(ARCHITECTURE, result);
	CHECK_FEATURE(ARCHITECTURE1, result);
	_DeviceFeatures._FEATURE_LEVELS.pFeatureLevelsRequested = gKnownFeatureLevels;
	_DeviceFeatures._FEATURE_LEVELS.NumFeatureLevels = gNumKnownFeatureLevels;
	CHECK_FEATURE(FEATURE_LEVELS, result);
	for (int32 i = 0; i < gNumKnownDXGIFormats; ++i)
	{
		D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupportData = {};
		DXGI_FORMAT formatDXGI = gKnownDXGIFormats[i];
		const TCHAR* formatDXGIName = gKnownDXGIFormatNames[i];
		formatSupportData.Format = formatDXGI;
		CHECK_FEATURE_FOR(FORMAT_SUPPORT, formatSupportData, formatDXGIName, result);
		if (result)
		{
			_DeviceFeatures._FormatSupportMap[formatDXGI] = formatSupportData;
		}

		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msData = {};
		msData.Format = formatDXGI;
		msData.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msData.SampleCount = 1;
		CHECK_FEATURE_FOR(MULTISAMPLE_QUALITY_LEVELS, msData, formatDXGIName, result);
		if (result)
		{
			_DeviceFeatures._MultiSampleQualityLevelMap[formatDXGI] = msData;
		}

		D3D12_FEATURE_DATA_FORMAT_INFO infoData = {};
		infoData.Format = formatDXGI;
		CHECK_FEATURE_FOR(FORMAT_INFO, infoData, formatDXGIName, result);
		if (result)
		{
			_DeviceFeatures._FormatInfoMap[formatDXGI] = infoData;
		}
	}
	CHECK_FEATURE(GPU_VIRTUAL_ADDRESS_SUPPORT, result);
	for (int32 i = 0; i < gNumKnownShaderModels; ++i)
	{
		D3D12_FEATURE_DATA_SHADER_MODEL data = {};
		D3D_SHADER_MODEL shaderModelD3D = gKnownShaderModels[i];
		data.HighestShaderModel = shaderModelD3D;
		CHECK_FEATURE_FOR(SHADER_MODEL, data, gKnownShaderModelNames[i], result);
		_DeviceFeatures._ShaderModelSupportMap[shaderModelD3D] = result;
	}
	CHECK_FEATURE(PROTECTED_RESOURCE_SESSION_SUPPORT, result);
	_DeviceFeatures._ROOT_SIGNATURE.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	CHECK_FEATURE(ROOT_SIGNATURE, result);
	CHECK_FEATURE(SHADER_CACHE, result);
	for (int32 i = 0; i < gNumKnownCommandListTypes; ++i)
	{
		D3D12_COMMAND_LIST_TYPE typeD3D12 = gKnownCommandListTypes[i];
		D3D12_FEATURE_DATA_COMMAND_QUEUE_PRIORITY data = {};
		data.CommandListType = typeD3D12;
		data.Priority = 0;
		CHECK_FEATURE_FOR(COMMAND_QUEUE_PRIORITY, data, gKnownCommandListTypeNames[i], result);
		_DeviceFeatures._CommandQueuePriority0SupportMap[typeD3D12] = (data.PriorityForTypeIsSupported == TRUE);
	}
	CHECK_FEATURE(EXISTING_HEAPS, result);
	CHECK_FEATURE(SERIALIZATION, result);
	CHECK_FEATURE(CROSS_NODE, result);
	CHECK_FEATURE(DISPLAYABLE, result);
	//CHECK_FEATURE(QUERY_META_COMMAND, result);
	CHECK_FEATURE(PROTECTED_RESOURCE_SESSION_TYPE_COUNT, result);
	_DeviceFeatures._ProtectedSessionTypeGUIDs.resize(_DeviceFeatures._PROTECTED_RESOURCE_SESSION_TYPE_COUNT.Count);
	_DeviceFeatures._PROTECTED_RESOURCE_SESSION_TYPES.NodeIndex = _DeviceFeatures._PROTECTED_RESOURCE_SESSION_TYPE_COUNT.NodeIndex;
	_DeviceFeatures._PROTECTED_RESOURCE_SESSION_TYPES.Count = _DeviceFeatures._PROTECTED_RESOURCE_SESSION_TYPE_COUNT.Count;
	_DeviceFeatures._PROTECTED_RESOURCE_SESSION_TYPES.pTypes = _DeviceFeatures._ProtectedSessionTypeGUIDs.data();
	CHECK_FEATURE(PROTECTED_RESOURCE_SESSION_TYPES, result);

	#undef CHECK_FEATURE_FOR
	#undef CHECK_FEATURE
}