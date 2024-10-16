#pragma once
#include "GFX/Resource/GenericResourceBarrier.h"
#include "GFX/QueueType.h"
#include "VK.h"

namespace ZE::GFX
{
	class Device;
	namespace Resource
	{
		class PipelineStateCompute;
		class PipelineStateGfx;
	}
}
namespace ZE::RHI::VK
{
	class Device;

	class CommandList final
	{
		VkCommandPool pool = VK_NULL_HANDLE;
		VkCommandBuffer commands = VK_NULL_HANDLE;
		U32 familyIndex = UINT32_MAX;

	public:
		CommandList() = default;
		CommandList(GFX::Device& dev) : CommandList(dev, GFX::QueueType::Main) {}
		CommandList(GFX::Device& dev, GFX::QueueType type);
		ZE_CLASS_MOVE(CommandList);
		~CommandList() { ZE_ASSERT(commands == nullptr && pool == nullptr, "Command list not freed before deletion!"); }

		void Open(GFX::Device& dev) { Open(); }
		void Close(GFX::Device& dev) { Close(); }

		void Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso);
		void Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso);
		void Reset(GFX::Device& dev);

		constexpr void Barrier(GFX::Device& dev, GFX::Resource::GenericResourceBarrier* barriers, U32 count) const noexcept(!_ZE_DEBUG_GFX_API) {}
		void DrawFullscreen(GFX::Device& dev) const noexcept(!_ZE_DEBUG_GFX_API);
		void Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(!_ZE_DEBUG_GFX_API);

		void Free(GFX::Device& dev) noexcept;
#if _ZE_GFX_MARKERS
		void TagBegin(GFX::Device& dev, std::string_view tag, Pixel color) const noexcept;
		void TagEnd(GFX::Device& dev) const noexcept;
#endif

		// Gfx API Internal

		constexpr VkCommandBuffer GetBuffer() const noexcept { return commands; }
		constexpr U32 GetFamily() const noexcept { return familyIndex; }

		void Init(Device& dev, GFX::QueueType commandType);
		void Open();
		void Close();
		void Reset(Device& dev);
		void Free(Device& dev) noexcept;
		void TransferOwnership(VkBufferMemoryBarrier2& barrier) noexcept;
	};
}