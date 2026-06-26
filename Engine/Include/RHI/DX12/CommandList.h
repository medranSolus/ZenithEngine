#pragma once
#include "DX12.h"
ZE_WARNING_PUSH
#include "WinPixEventRuntime/pix3.h"
ZE_WARNING_POP

namespace ZE::GFX
{
	class Device;
	namespace Resource
	{
		class PipelineStateCompute;
		class PipelineStateGfx;
	}
}
namespace ZE::RHI::DX12
{
	class Device;

	class CommandList final
	{
		DX::ComPtr<IGraphicsCommandList> commands;
		DX::ComPtr<ICommandAllocator> allocator;

		Status Open(Device& dev, IPipelineState* state) const noexcept;
		void RestoreExternalState(Device& dev) const noexcept;

	public:
		CommandList() = default;
		ZE_CLASS_MOVE(CommandList);
		~CommandList() = default;

		static Expected<CommandList> Create(GFX::Device& dev) noexcept { return Create(dev, GFX::QueueType::Main); }
		static Expected<CommandList> Create(GFX::Device& dev, GFX::QueueType type) noexcept;

		bool IsInitialized() const noexcept { return allocator != nullptr; }
		void* GetHandle() const noexcept { return GetList(); }

		Status Open(GFX::Device& dev) const noexcept;
		Status Open(GFX::Device& dev, GFX::Resource::PipelineStateCompute& pso) const noexcept;
		Status Open(GFX::Device& dev, GFX::Resource::PipelineStateGfx& pso) const noexcept;

		void RestoreExternalState(GFX::Device& dev) const noexcept;

		Status Close(GFX::Device& dev) noexcept;
		Status Reset(GFX::Device& dev) noexcept;

		void DrawFullscreen(GFX::Device& dev) const noexcept;
		void Compute(GFX::Device& dev, U32 groupX, U32 groupY, U32 groupZ) const noexcept;

		void WriteBreadcrumbs(GFX::Device& dev, U32 value, U64 location, void* breadcrumbsBuffer, bool isBegin) const noexcept;

#if _ZE_GFX_MARKERS
		void TagBegin(GFX::Device& dev, std::string_view tag, Pixel color) const noexcept;
		void TagEnd(GFX::Device& dev) const noexcept;
#endif

		// Gfx API Internal

		static Expected<CommandList> Create(Device& dev, GFX::QueueType type) noexcept;

		IGraphicsCommandList* GetList() const noexcept { return commands.Get(); }
		Status Open(Device& dev) const noexcept { return Open(dev, nullptr); }

		Status Close(Device& dev) noexcept;
		Status Reset(Device& dev) const noexcept;
	};
}