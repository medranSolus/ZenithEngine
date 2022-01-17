#pragma once
#include "GFX/Binding/SchemaDesc.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::DX11::Binding
{
	class Schema final
	{
	public:
		Schema() = default;
		Schema(GFX::Device& dev, const GFX::Binding::SchemaDesc& desc) {}
		ZE_CLASS_MOVE(Schema);
		~Schema() = default;

		constexpr void SetCompute(GFX::CommandList& cl) const noexcept {}
		constexpr void SetGraphics(GFX::CommandList& cl) const noexcept {}
	};
}