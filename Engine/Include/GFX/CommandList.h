#pragma once

namespace ZE::GFX
{
	// Storing commands for GPU
	class CommandList
	{
	protected:
		CommandList() = default;

	public:
		CommandList(CommandList&&) = delete;
		CommandList(const CommandList&) = delete;
		CommandList& operator=(CommandList&&) = delete;
		CommandList& operator=(const CommandList&) = delete;
		virtual ~CommandList() = default;
	};
}