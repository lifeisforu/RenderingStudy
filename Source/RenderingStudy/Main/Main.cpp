#include <iostream>

#include "Debug/DXDebugLayer.h"
#include "Debug/Log.h"

#include "Renderer/RendererD3D12.h"

int main()
{
	#if defined(RS_ENABLE_DX_DEBUG)
	const bool enableDebugLayer = true;
	#else // NOT RS_ENABLE_DX_DEBUG
	const bool enableDebugLayer = false;
	#endif // RS_ENABLE_DX_DEBUG

	RendererD3D12 renderer;

	if (enableDebugLayer)
	{
		if (renderer.CreateDebugLayer() == false)
		{
			return -1;
		}
	}

	if (renderer.CreateDXGIFactory() == false)
	{
		return -1;
	}

	renderer.EnumerateAdapters();

	if (renderer.SelectAdapterByPreference(DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE) == false)
	{
		return -1;
	}

	TArray<D3D_FEATURE_LEVEL> featureLevels = renderer.EnumerateFeatureLevels();
	auto lastFeatureLevel = *featureLevels.rbegin();
	if (renderer.CreateDevice(lastFeatureLevel) == false)
	{
		return -1;
	}

	return 0;
}