#pragma once
#include "Types.h"
#include "Device.h"

namespace ZE::GFX
{
	// Sending commands to GPU
	class IContext
	{
	protected:
		IContext() = default;

	public:
		IContext(IContext&&) = delete;
		IContext(const IContext&) = delete;
		IContext& operator=(IContext&&) = delete;
		IContext& operator=(const IContext&) = delete;
		virtual ~IContext() = default;

		virtual void DrawIndexed(Device& dev, U32 count) const noexcept(ZE_NO_DEBUG) = 0;
		virtual void Compute(Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG) = 0;
	};
}