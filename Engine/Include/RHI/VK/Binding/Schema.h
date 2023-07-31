#pragma once
#include "GFX/Binding/SchemaDesc.h"
#include "GFX/CommandList.h"

namespace ZE::RHI::VK::Binding
{
	class Schema final
	{
	public:
		enum class BindType : U8 { Constant, CBV, SRV, UAV, Table };

		struct Binding
		{
			BindType Type;
			U8 DescSetIndex;
		};

	private:
		bool isCompute;
		U32 count;
		Ptr<Binding> bindings;
		U32 samplersCount;
		Ptr<VkSampler> samplers;
		VkPipelineLayout layout = VK_NULL_HANDLE;

	public:
		Schema() = default;
		Schema(GFX::Device& dev, const GFX::Binding::SchemaDesc& desc);
		ZE_CLASS_MOVE(Schema);
		~Schema() { ZE_ASSERT(bindings == nullptr && samplers == nullptr, "Resource not freed before deletion!"); }

		constexpr U32 GetCount() const noexcept { return count; }

		void Free(GFX::Device& dev) noexcept;
		void SetCompute(GFX::CommandList& cl) const noexcept;
		void SetGraphics(GFX::CommandList& cl) const noexcept;

		// Gfx API Internal

		constexpr bool IsCompute() const noexcept { return isCompute; }
		constexpr VkPipelineLayout GetLayout() const noexcept { return layout; }

		const Binding& GetCurrentType(U32 index) const noexcept { ZE_ASSERT(index < count, "Access out of range!"); return bindings[index]; }
	};
}