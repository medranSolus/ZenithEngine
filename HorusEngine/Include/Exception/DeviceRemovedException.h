#pragma once
#include "GraphicsException.h"

namespace Exception
{
	// Exception getting info from DXGI_ERROR_DEVICE_REMOVED error (driver crash, device hung, overheat, etc.)
	class DeviceRemovedException : public GraphicsException
	{
	public:
#ifdef _MODE_DEBUG
		DeviceRemovedException(U32 line, const char* file, HRESULT hResult,
			std::vector<std::string>&& info = std::vector<std::string>()) noexcept
			: BasicException(line, file), GraphicsException(line, file, hResult, std::forward<std::vector<std::string>>(info)) {}
#else
		DeviceRemovedException(U32 line, const char* file, HRESULT hResult) noexcept
			: BasicException(line, file), GraphicsException(line, file, hResult) {}
#endif
		DeviceRemovedException(DeviceRemovedException&&) = default;
		DeviceRemovedException(const DeviceRemovedException&) = default;
		DeviceRemovedException& operator=(DeviceRemovedException&&) = default;
		DeviceRemovedException& operator=(const DeviceRemovedException&) = default;
		virtual ~DeviceRemovedException() = default;

		constexpr const char* GetType() const noexcept override { return "Graphics Removed Exception"; }
	};
}