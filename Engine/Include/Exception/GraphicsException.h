#pragma once
#include "Platform/WinAPI/WinApiException.h"
#include "GfxDebugException.h"

namespace ZE::Exception
{
#ifdef _ZE_MODE_DEBUG
	class GraphicsException : public WinAPI::WinApiException, public GfxDebugException
	{
	public:
		GraphicsException(U32 line, const char* file, HRESULT hResult,
			std::vector<std::string>&& info = std::vector<std::string>()) noexcept
			: BasicException(line, file), WinApiException(line, file, hResult),
			GfxDebugException(line, file, std::forward<std::vector<std::string>>(info)) {}
#else
	class GraphicsException : public WinAPI::WinApiException
	{
	public:
		GraphicsException(U32 line, const char* file, HRESULT hResult) noexcept
			: BasicException(line, file), WinApiException(line, file, hResult) {}
#endif
		GraphicsException(GraphicsException&&) = default;
		GraphicsException(const GraphicsException&) = default;
		GraphicsException& operator=(GraphicsException&&) = default;
		GraphicsException& operator=(const GraphicsException&) = default;
		virtual ~GraphicsException() = default;

		constexpr const char* GetType() const noexcept override { return "Graphics Exception"; }

		const char* what() const noexcept override;
	};
}