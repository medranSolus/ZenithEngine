#pragma once

namespace ZE::GFX
{
	// Resource allocation
	class Device
	{
	protected:
		Device() = default;

	public:
		Device(Device&&) = delete;
		Device(const Device&) = delete;
		Device& operator=(Device&&) = delete;
		Device& operator=(const Device&) = delete;
		virtual ~Device() = default;
	};
}