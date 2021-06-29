#pragma once
#include "IContext.h"
#include "CommandList.h"

namespace ZE::GFX
{
	// Recording commands for MainContext
	class DeferredContext : public IContext
	{
	protected:
		DeferredContext() = default;

	public:
		virtual ~DeferredContext() = default;

		virtual void SetCommandList(CommandList& cl) noexcept = 0;
		virtual void FinishList(Device& dev) = 0;
	};
}