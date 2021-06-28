#pragma once

namespace ZE::GFX
{
	// Sending commands to GPU
	class Context
	{
	protected:
		Context() = default;

	public:
		Context(Context&&) = delete;
		Context(const Context&) = delete;
		Context& operator=(Context&&) = delete;
		Context& operator=(const Context&) = delete;
		virtual ~Context() = default;
	};
}