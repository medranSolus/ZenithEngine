#pragma once
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
namespace ZE::GFX::API::VK
{
	class Device;

	class CommandList final
	{
		VkCommandPool pool = VK_NULL_HANDLE;
		VkCommandBuffer commands = VK_NULL_HANDLE;
		U32 familyIndex = UINT32_MAX;

		void Open(Device& dev, VkPipeline state, VkPipelineBindPoint bindPoint);

	public:
		CommandList() = default;
		CommandList(GFX::Device& dev) : CommandList(dev, QueueType::Main) {}
		CommandList(GFX::Device& dev, QueueType type);
		ZE_CLASS_MOVE(CommandList);
		~CommandList() { ZE_ASSERT(commands == nullptr && pool == nullptr, "Command list not freed before deletion!"); }

		void Open(GFX::Device& dev);
		void Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso);
		void Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso);

		void Close(GFX::Device& dev) { Close(); }
		void Reset(GFX::Device& dev);

		void Draw(GFX::Device& dev, U32 vertexCount) const noexcept(!_ZE_DEBUG_GFX_API);
		void DrawIndexed(GFX::Device& dev, U32 indexCount) const noexcept(!_ZE_DEBUG_GFX_API);
		void DrawFullscreen(GFX::Device& dev) const noexcept(!_ZE_DEBUG_GFX_API);
		void Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(!_ZE_DEBUG_GFX_API);

#if _ZE_GFX_MARKERS
		void TagBegin(GFX::Device& dev, std::string_view tag, Pixel color) const noexcept;
		void TagEnd(GFX::Device& dev) const noexcept;
#endif
		void Free(GFX::Device& dev) noexcept;

		// Gfx API Internal

		constexpr VkCommandBuffer GetBuffer() const noexcept { return commands; }
		constexpr U32 GetFamily() const noexcept { return familyIndex; }
		void Open(Device& dev) { Open(dev, VK_NULL_HANDLE, VK_PIPELINE_BIND_POINT_GRAPHICS); }

		void Init(Device& dev, QueueType commandType);
		void Close();
		void Reset(Device& dev);
		void Free(Device& dev) noexcept;
		void TransferOwnership(VkBufferMemoryBarrier2& barrier) noexcept;
	};
}