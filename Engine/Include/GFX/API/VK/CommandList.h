#pragma once
#include "GFX/CommandType.h"
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

		void Open(Device& dev, VkPipeline state, VkPipelineBindPoint bindPoint);

	public:
		CommandList() = default;
		CommandList(GFX::Device& dev) : CommandList(dev, CommandType::All) {}
		CommandList(GFX::Device& dev, CommandType type);
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
		void Open(Device& dev) { Open(dev, VK_NULL_HANDLE, VK_PIPELINE_BIND_POINT_GRAPHICS); }

		void Init(Device& dev, CommandType type);
		void Close();
		void Reset(Device& dev);
	};
}