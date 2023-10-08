#pragma once
#include "GFX/Resource/GenericResourceBarrier.h"
#include "GFX/QueueType.h"
#include "DX11.h"

namespace ZE::GFX
{
	class Device;
	namespace Resource
	{
		class PipelineStateCompute;
		class PipelineStateGfx;
	}
}
namespace ZE::RHI::DX11
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
		CommandList(GFX::Device& dev, GFX::QueueType type);
		ZE_CLASS_MOVE(CommandList);
		~CommandList() { ZE_ASSERT(tagManager == nullptr && commands == nullptr && context == nullptr, "Command list not freed before deletion!"); }

		constexpr void Open(GFX::Device& dev) {}

		void Reset(GFX::Device& dev) { commands = nullptr; }
#if _ZE_GFX_MARKERS
		void TagBegin(GFX::Device& dev, std::string_view tag, Pixel color) const noexcept { tagManager->BeginEvent(Utils::ToUTF16(tag).c_str()); }
		void TagEnd(GFX::Device& dev) const noexcept { tagManager->EndEvent(); }
#endif

		void Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso);
		void Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso);
		void Close(GFX::Device& dev);

		constexpr void Barrier(GFX::Device& dev, GFX::Resource::GenericResourceBarrier* barriers, U32 count) const noexcept(!_ZE_DEBUG_GFX_API) {}
		void DrawFullscreen(GFX::Device& dev) const noexcept(!_ZE_DEBUG_GFX_API);
		void Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept(!_ZE_DEBUG_GFX_API);
		void Free(GFX::Device& dev) noexcept;

		// Gfx API Internal

		constexpr bool IsDeferred() const noexcept { return deferred; }
		IDeviceContext* GetContext() const noexcept { return context.Get(); }
		ICommandList* GetList() const noexcept { return commands.Get(); }
	};
}