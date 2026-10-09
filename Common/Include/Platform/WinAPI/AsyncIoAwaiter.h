#pragma once
#include "IO/EofResult.h"
#include "IocpOverlapped.h"

namespace ZE::Platform::WinAPI
{
    // Custom awaiter for non-blocking I/O
    template<bool IS_READ, typename BuffBtr>
    struct AsyncIoAwaiter final
    {
        HANDLE FileHandle = nullptr;
        BuffBtr Buffer = nullptr;
        U32 Size = 0;
        IocpOverlapped Overlapped = {};

        constexpr AsyncIoAwaiter(HANDLE fileHandle, BuffBtr buffer, U32 size, U64 offset) noexcept;
        ZE_CLASS_DEFAULT(AsyncIoAwaiter);
        ~AsyncIoAwaiter() = default;

        // Always attempt suspension to let the I/O run asynchronously
        constexpr bool await_ready() const noexcept { return false; }

        // Returning a coroutine_handle allows for symmetric transfer
        std::coroutine_handle<> await_suspend(std::coroutine_handle<> awaitingHandle) noexcept;
        constexpr Status await_resume() const noexcept;
    };

#pragma region Functions
    template<bool IS_READ, typename BuffBtr>
    constexpr AsyncIoAwaiter<IS_READ, BuffBtr>::AsyncIoAwaiter(HANDLE fileHandle, BuffBtr buffer, U32 size, U64 offset) noexcept
        : FileHandle(fileHandle), Buffer(buffer), Size(size)
    {
        Overlapped.Offset = static_cast<U32>(offset & UINT32_MAX);
        Overlapped.OffsetHigh = static_cast<U32>((offset >> 32));
    }

    template<bool IS_READ, typename BuffBtr>
    std::coroutine_handle<> AsyncIoAwaiter<IS_READ, BuffBtr>::await_suspend(std::coroutine_handle<> awaitingHandle) noexcept
    {
        Overlapped.Continuation = awaitingHandle;

        BOOL result = false;
        if constexpr (IS_READ)
            result = ReadFile(FileHandle, Buffer, Size, nullptr, &Overlapped);
        else
            result = WriteFile(FileHandle, Buffer, Size, nullptr, &Overlapped);

        if (result)
        {
            // Data available immediately
            if (GetOverlappedResult(FileHandle, &Overlapped, &Overlapped.BytesTransferred, FALSE) != 0)
                Overlapped.Error = ZE_WIN_LAST_ERROR();
            return awaitingHandle;
        }

        DWORD lastError = GetLastError();
        if (lastError != ERROR_IO_PENDING)
        {
            // I/O failed to queue
            Overlapped.Error = ZE_WIN_ERROR(lastError);
            return awaitingHandle;
        }

        // I/O successfully queued, cleanly suspend
        return std::noop_coroutine();
    }

    template<bool IS_READ, typename BuffBtr>
    constexpr Status AsyncIoAwaiter<IS_READ, BuffBtr>::await_resume() const noexcept
    {
        if (Overlapped.Error)
            return Overlapped.Error;

        if (Size != Overlapped.BytesTransferred)
            return IO::EofResult::Make(Overlapped.BytesTransferred);
        return {};
    }
#pragma endregion
}