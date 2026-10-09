#pragma once
#include "WinAPI.h"

namespace ZE::Platform::WinAPI
{
    // Custom overlapped structure that allows for keeping more data inside 
    struct IocpOverlapped : public OVERLAPPED
    {
        std::coroutine_handle<> Continuation = {};
        DWORD BytesTransferred = 0;
        Status Error = {};

        constexpr IocpOverlapped() noexcept;
        ZE_CLASS_DEFAULT(IocpOverlapped);
        virtual ~IocpOverlapped() = default;
    };


#pragma region Functions
    constexpr IocpOverlapped::IocpOverlapped() noexcept
    {
        Internal = 0;
        InternalHigh = 0;
        Offset = 0;
        OffsetHigh = 0;
        hEvent = nullptr;
    }
#pragma endregion
}