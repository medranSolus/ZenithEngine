#pragma once
#include "GFX/QueueType.h"
#include "D3D11.h"

namespace ZE::GFX
{
	class Device;
	namespace Resource
	{
		class PipelineStateCompute;
		class PipelineStateGfx;
	}
}
namespace ZE::GFX::API::DX11
{
	class CommandList final
	{
		bool deferred;
		DX::ComPtr<IDeviceContext> context;
		DX::ComPtr<ICommandList> commands;
#if _ZE_GFX_MARKERS
		DX::ComPtr<ID3DUserDefinedAnnotation> tagManager;
#endif

	public:
		CommandList() = default;
		CommandList(GFX::Device& dev);
		CommandList(GFX::Device& dev, QueueType type);
		ZE_CLASS_MOVE(CommandList);
		~CommandList() { ZE_ASSERT(tagManager == nullptr && commands == nullptr && context == nullptr, "Command list not freed before deletion!"); }

		constexpr void Open(GFX::Device& dev) {}

		void Reset(GFX::Device& dev) { commands = nullptr; }
#if _ZE_GFX_MARKERS
		void TagBegin(GFX::Device& dev, std::string_view tag, Pixel color) const noexcept { std::wstring label = Utils::ToUtf8(tag); tagManager->BeginEvent(label.c_str()); }
		void TagEnd(GFX::Device& dev) const noexcept { tagManager->EndEvent(); }
#endif
		void Free(GFX::Device& dev) noexcept { tagManager = nullptr; commands = nullptr; context = nullptr; }

		void Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso);
		void Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso);
		void Close(GFX::Device& dev);

		void Draw(GFX::Device& dev, U32 vertexCount) const noexcept(!_ZE_DEBUG_GFX_API);
		void DrawIndexed(GFX::Device& dev, U32 indexCount) const noexcept(!_ZE_DEBUG_GFX_API);
		void DrawFullscreen(GFX::Device& dev) const noexcept(!_ZE_DEBUG_GFX_API);
		void Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(!_ZE_DEBUG_GFX_API);

		// Gfx API Internal

		constexpr bool IsDeferred() const noexcept { return deferred; }
		IDeviceContext* GetContext() const noexcept { return context.Get(); }
		ICommandList* GetList() const noexcept { return commands.Get(); }
	};
}