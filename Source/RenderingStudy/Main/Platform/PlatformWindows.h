#pragma once

#define NOMINMAX

#include <Windows.h>
#include <tchar.h>

#include <d3d12.h>
#include <dxgi1_6.h>

#define RS_ENABLE_DX_DEBUG
#if defined(RS_ENABLE_DX_DEBUG)
#include <d3d12sdklayers.h>
#include <dxgidebug.h>
#endif // RS_ENABLE_DX_DEBUG

#include <type_traits>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <format>

#include <stdint.h>

#define BLOCK(desc)

#define FORMAT(InFormat, ...) std::format(InFormat, __VA_ARGS__).c_str()

#define TUniquePtr std::unique_ptr
#define MakeUniquePtr std::make_unique
#define TSharedPtr std::shared_ptr
#define MakeSharedPtr std::make_shared
#define TWeakPtr std::weak_ptr
#define MakeWeakPtr std::make_weak
#define TArray std::vector
#define TMap std::unordered_map

typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;