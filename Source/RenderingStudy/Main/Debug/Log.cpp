#include "Log.h"

#include <iostream>

#if _UNICODE
#define CONSOLE_OUTPUT std::wcout
#define PRINT OutputDebugStringW
#else // NOT _UNICODE
#define CONSOLE_OUTPUT std::cout
#define PRINT OutputDebugString
#endif // _UNICODE

void OutputLog(const ELogCategory InCategory, const TCHAR* InCategoryName, const TCHAR* InText)
{
	#if _DEBUG
	#else // NOT _DEBUG
	if (InCategory == ELogCategory::Debug)
	{
		return;
	}
	#endif // _DEBUG

	CONSOLE_OUTPUT << TEXT("[") << InCategoryName << TEXT("] : ") << InText << std::endl;

	PRINT(FORMAT(TEXT("[{0}] {1}\n"), InCategoryName, InText));
}

void OutputHRESULT(const HRESULT InHR, const TCHAR* InText)
{
	#if _UNICODE
	LPWSTR messageBuffer;
	#else // NOT _UNICODE
	LPSTR messageBuffer;
	#endif // _UNICODE
	DWORD numChars = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		InHR,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		#if _UNICODE
		(LPWSTR)&messageBuffer,
		#else // NOT _UNICODE
		(LPSTR)&messageBuffer,
		#endif // _UNICODE
		0,
		nullptr);

	if (numChars > 0) 
	{
		CONSOLE_OUTPUT << TEXT("[") << messageBuffer << TEXT("]") << InText << std::endl;

		PRINT(FORMAT(TEXT("[{0}] {1}\n"), messageBuffer, InText));

		LocalFree(messageBuffer);
	}
	else 
	{
		CONSOLE_OUTPUT << TEXT("[Error : ") << GetLastError() << TEXT("]") << InText << std::endl;

		PRINT(FORMAT(TEXT("[Error : {0}] {1}\n"), GetLastError(), InText));
	}
}

void OutputAssertLog(const TCHAR* InExpression, const TCHAR* InFile, const int32 InLine, const TCHAR* InText)
{
	CONSOLE_OUTPUT << TEXT("[Assertion failed] expression : ") << InExpression << std::endl
		<< TEXT("\tat ") << InFile << TEXT("(") << InLine << TEXT(")") << std::endl
		<< TEXT("\t") << InText << std::endl;

	PRINT(FORMAT(TEXT("[Assertion failed] expression : {0}\n\t{1}({2})\n\t{3}\n"), InExpression, InFile, InLine, InText));
}