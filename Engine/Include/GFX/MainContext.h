#pragma once
#include "IContext.h"
#include "CommandList.h"

namespace ZE::GFX
{
	// Sending direct commands to GPU
	class MainContext : public IContext
	{
	protected:
		MainContext() = default;

	public:
		virtual ~MainContext() = default;

		virtual void Execute(Device& dev, CommandList& cl) const = 0;
	};
}