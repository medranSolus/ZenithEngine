#pragma once
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
		ZE_CLASS_MOVE(CommandList);
		~CommandList() = default;

		static Expected<CommandList> Create(GFX::Device& dev) noexcept;
		static Expected<CommandList> Create(GFX::Device& dev, GFX::QueueType type) noexcept;

		constexpr void RestoreExternalState(GFX::Device& dev) const noexcept {}
		constexpr void WriteBreadcrumbs(GFX::Device& dev, U32 value, U64 location, void* breadcrumbsBuffer, bool isBegin) const noexcept {}

		bool IsInitialized() const noexcept { return context != nullptr; }
		void* GetHandle() const noexcept { return GetContext(); }
		Status Open(GFX::Device& dev) const noexcept { return {}; }
		Status Reset(GFX::Device& dev) noexcept { commands = nullptr; return {}; }

#if _ZE_GFX_MARKERS
		void TagBegin(GFX::Device& dev, std::string_view tag, Pixel color) const noexcept { tagManager->BeginEvent(Utils::ToUTF16(tag).c_str()); }
		void TagEnd(GFX::Device& dev) const noexcept { tagManager->EndEvent(); }
#endif

		Status Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso) const noexcept;
		Status Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso) const noexcept;
		Status Close(GFX::Device& dev) noexcept;

		void DrawFullscreen(GFX::Device& dev) const noexcept;
		void Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept;

		// Gfx API Internal

		constexpr bool IsDeferred() const noexcept { return deferred; }
		IDeviceContext* GetContext() const noexcept { return context.Get(); }
		ICommandList* GetList() const noexcept { return commands.Get(); }
	};
}