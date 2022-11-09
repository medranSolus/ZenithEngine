#pragma once
#include "GFX/CommandType.h"

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
	class CommandList final
	{
	public:
		CommandList() = default;
		CommandList(GFX::Device& dev);
		CommandList(GFX::Device& dev, CommandType type);
		ZE_CLASS_MOVE(CommandList);
		~CommandList() = default;

		constexpr void Open(GFX::Device& dev) {}

#if _ZE_GFX_MARKERS
		void TagBegin(GFX::Device& dev, const wchar_t* tag, Pixel color) const noexcept {}
		void TagEnd(GFX::Device& dev) const noexcept {}
#endif
		void Reset(GFX::Device& dev) {}

		void Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso);
		void Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso);
		void Close(GFX::Device& dev);

		void Draw(GFX::Device& dev, U32 vertexCount) const noexcept(!_ZE_DEBUG_GFX_API);
		void DrawIndexed(GFX::Device& dev, U32 indexCount) const noexcept(!_ZE_DEBUG_GFX_API);
		void DrawFullscreen(GFX::Device& dev) const noexcept(!_ZE_DEBUG_GFX_API);
		void Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(!_ZE_DEBUG_GFX_API);
	};
}