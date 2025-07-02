#pragma once

#define NOMINMAX

#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>

#define RS_ENABLE_DX_DEBUG
#if defined(RS_ENABLE_DX_DEBUG)
#include <d3d12sdklayers.h>
#include <dxgidebug.h>
#endif // RS_ENABLE_DX_DEBUG