#include <iostream>

#include "Platform/PlatformWindows.h"
#include "Platform/ComPointer.h"
#include "Debug/DXDebugLayer.h"

int main()
{
	const bool enableDebugLayer = true;

	if (enableDebugLayer)
	{
		DXDebugLayer::Get().Init();
	}

	ComPointer<ID3D12Device14> deviceD3D12;


	if (enableDebugLayer)
	{
		DXDebugLayer::Get().Deinit();
	}
}