#pragma once
#include "GFX/Binding/SchemaDesc.h"
#include "GFX/Device.h"

namespace ZE::GFX::API::VK::Binding
{
	class Schema final
	{
		VkPipelineLayout layout = VK_NULL_HANDLE;

	public:
		Schema() = default;
		Schema(GFX::Device& dev, const GFX::Binding::SchemaDesc& desc);
		ZE_CLASS_MOVE(Schema);
		~Schema();

		constexpr U32 GetCount() const noexcept { return 0; }
		void SetCompute(GFX::CommandList& cl) const noexcept;
		void SetGraphics(GFX::CommandList& cl) const noexcept;

		// Gfx API Internal

		constexpr VkPipelineLayout GetLayout() const noexcept { return layout; }
	};
}