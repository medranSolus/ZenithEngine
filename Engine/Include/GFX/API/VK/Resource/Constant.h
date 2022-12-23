#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::VK::Resource
{
	template<typename T>
	class Constant final
	{

	public:
		Constant() = default;
		Constant(GFX::Device& dev, const T& value) {}
		ZE_CLASS_MOVE(Constant);
		~Constant() = default;

		constexpr void Set(GFX::Device& dev, const T& value) const {}
		constexpr T GetData(GFX::Device& dev) const { return {}; }

		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept {}
	};
}