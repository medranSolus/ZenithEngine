#pragma once
#include "Exception/BasicException.h"
#include <vector>

namespace Exception
{
#ifdef _MODE_DEBUG
	// Exception using DXGIDebugInfoManager
	class GfxDebugException : public virtual BasicException
	{
		std::vector<std::string> debugInfo;

	public:
		GfxDebugException(U32 line, const char* file, std::vector<std::string>&& info) noexcept
			: BasicException(line, file), debugInfo(std::move(info)) {}
		GfxDebugException(GfxDebugException&&) = default;
		GfxDebugException(const GfxDebugException&) = default;
		GfxDebugException& operator=(GfxDebugException&&) = default;
		GfxDebugException& operator=(const GfxDebugException&) = default;
		virtual ~GfxDebugException() = default;

		constexpr const char* GetType() const noexcept override { return "DirectX Debug Exception"; }

		std::string GetDebugInfo() const noexcept;
		const char* what() const noexcept override;
	};
#endif
}