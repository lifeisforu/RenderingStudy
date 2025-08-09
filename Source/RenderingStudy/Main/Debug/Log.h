#pragma once

#include "Platform/PlatformWindows.h"

//-----------------------------------------------------------------------------
// ELogCategory
//-----------------------------------------------------------------------------

/// log category.
enum class ELogCategory : uint8
{
	Debug, ///< for _DEBUG
	Info, ///< information.
	Warning, ///< warning.
	Error, ///< error.
	Fatal ///< fatal and exit program.
};

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

/// output log.
/// @param InCategory category.
/// @param InCategoryName category name string.
/// @param InText string to be logged.
extern void OutputLog(const ELogCategory InCategory, const TCHAR* InCategoryName, const TCHAR* InText);

/// output HRESULT.
/// @param InHR HRESULT.
/// @param InText text to be logged.
extern void OutputHRESULT(const HRESULT InHR, const TCHAR* InText);

/// output assert log.
/// @param InExpression expression.
/// @param InFile file name.
/// @param InLine line number.
/// @param InText text.
extern void OutputAssertLog(const TCHAR* InExpression, const TCHAR* InFile, const int32 InLine, const TCHAR* InText);

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

/// macro for calling OutputLog().
#define LOG(InCategory, InFormat, ...) \
	OutputLog(ELogCategory:: ##InCategory, TEXT(#InCategory), FORMAT(InFormat, __VA_ARGS__));\
	if (ELogCategory::InCategory >= ELogCategory::Error) DebugBreak()

/// macro for calling LogHRESULT()
#define LOG_HRESULT(InHR, InFormat, ...) OutputHRESULT(InHR, FORMAT(InFormat, __VA_ARGS__))

/// macro for calling LogHRESULT() with assert.
#define ASSERT_HRESULT(InHR, InFormat, ...) OutputHRESULT(InHR, FORMAT(InFormat, __VA_ARGS__)); DebugBreak()

/// macro for calling Assert().
#define ASSERT(InExpression, InFormat, ...) if(!(InExpression))\
	{ OutputAssertLog(TEXT(#InExpression), TEXT(__FILE__), __LINE__, FORMAT(InFormat, __VA_ARGS__)); DebugBreak(); }
