/*
	This is free and unencumbered software released into the public domain.

	Anyone is free to copy, modify, publish, use, compile, sell, or
	distribute this software, either in source code form or as a compiled
	binary, for any purpose, commercial or non-commercial, and by any
	means.

	In jurisdictions that recognize copyright laws, the author or authors
	of this software dedicate any and all copyright interest in the
	software to the public domain. We make this dedication for the benefit
	of the public at large and to the detriment of our heirs and
	successors. We intend this dedication to be an overt act of
	relinquishment in perpetuity of all present and future rights to this
	software under copyright law.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
	OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
	ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.

	For more information, please refer to <http://unlicense.org/>

	https://gist.github.com/Ohjurot/e17a5f04e9719a44866b4f38a4f2f680
	Created by Ludwig Fuechsl
	Last Change: 07.04.2023
*/
#pragma once

#include <type_traits>

/// <summary>
/// A template class for Microsoft com pointer (I like this one more the the WRL pointer)
/// </summary>
/// <typeparam name="T">Com pointer Type</typeparam>
template<typename CT, typename = std::enable_if_t<std::is_base_of_v<IUnknown, CT>>>
class ComPointer
{
public:
	// Default empty constructor
	ComPointer() = default;

	// Construct by raw pointer (add ref)
	ComPointer(CT* pointer)
	{
		SetPointerAndAddRef(pointer);
	}

	ComPointer(const ComPointer<CT>& other)
	{
		SetPointerAndAddRef(other._Pointer);
	}
	ComPointer(ComPointer<CT>&& other) noexcept
	{
		_Pointer = other._Pointer;
		other._Pointer = nullptr;
	}

	~ComPointer()
	{
		ClearPointer();
	}

	ComPointer<CT>& operator=(const ComPointer<CT>& other)
	{
		ClearPointer();
		SetPointerAndAddRef(other._Pointer);
		return *this;
	}
	ComPointer<CT>& operator=(ComPointer<CT>&& other)
	{
		ClearPointer();

		_Pointer = other._Pointer;
		other._Pointer = nullptr;
		return *this;
	}
	ComPointer<CT>& operator=(CT* other)
	{
		ClearPointer();
		SetPointerAndAddRef(other);
		return *this;
	}

	ULONG Release()
	{
		return ClearPointer();
	}

	CT* GetRef()
	{
		if (_Pointer)
		{
			_Pointer->AddRef();
			return _Pointer;
		}
		return nullptr;
	}
	CT* Get()
	{
		return _Pointer;
	}

	template<typename T>
	bool QueryInterface(ComPointer<T>& other, HRESULT* errorCode = nullptr)
	{
		if (_Pointer)
		{
			HRESULT result = _Pointer->QueryInterface(IID_PPV_ARGS(&other));
			if (errorCode) *errorCode = result;
			return result == S_OK;
		}
		return false;
	}

	bool operator==(const ComPointer<CT>& other)
	{
		return _Pointer == other._Pointer;
	}
	bool operator==(const CT* other)
	{
		return _Pointer == other;
	}

	CT* operator->()
	{
		return _Pointer;
	}
	CT** operator&()
	{
		return &_Pointer;
	}

	operator bool()
	{
		return _Pointer != nullptr;
	}
	operator CT* ()
	{
		return _Pointer;
	}

private:
	ULONG ClearPointer()
	{
		ULONG newRef = 0;
		if (_Pointer)
		{
			newRef = _Pointer->Release();
			_Pointer = nullptr;
		}

		return newRef;
	}

	void SetPointerAndAddRef(CT* pointer)
	{
		_Pointer = pointer;
		if (_Pointer)
		{
			_Pointer->AddRef();
		}
	}

private:
	CT* _Pointer = nullptr;
};