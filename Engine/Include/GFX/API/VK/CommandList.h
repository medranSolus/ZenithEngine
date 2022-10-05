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

#ifdef _ZE_MODE_DEBUG
		void TagBegin(GFX::Device& dev, const wchar_t* tag, Pixel color) const noexcept {}
		void TagEnd(GFX::Device& dev) const noexcept {}
#endif
		void Reset(GFX::Device& dev) {}

		void Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso);
		void Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso);
		void Close(GFX::Device& dev);

		void Draw(GFX::Device& dev, U32 vertexCount) const noexcept(ZE_NO_DEBUG);
		void DrawIndexed(GFX::Device& dev, U32 indexCount) const noexcept(ZE_NO_DEBUG);
		void DrawFullscreen(GFX::Device& dev) const noexcept(ZE_NO_DEBUG);
		void Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(ZE_NO_DEBUG);
	};
}