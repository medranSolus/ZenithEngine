#pragma once
#include "D3D11.h"

namespace ZE::GFX
{
	class CommandList;
}
namespace ZE::GFX::API::DX11
{
	class Device final
	{
#ifdef _ZE_MODE_DEBUG
		DX::DebugInfoManager debugManager;
#endif
		DX::ComPtr<ID3D11Device5> device;
		DX::ComPtr<ID3D11DeviceContext4> context;

		U32 commandListsCount = 0;
		U32 descriptorCount;
		U32 scratchDescriptorCount;

		void Execute(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG);

	public:
		Device(U32 descriptorCount, U32 scratchDescriptorCount);
		ZE_CLASS_DELETE(Device);
		~Device();

		constexpr std::pair<U32, U32> GetData() const noexcept { return { descriptorCount, scratchDescriptorCount }; }

		constexpr U64 GetMainFence() const noexcept { return 0; }
		constexpr U64 GetComputeFence() const noexcept { return 0; }
		constexpr U64 GetCopyFence() const noexcept { return 0; }

		constexpr void WaitMain(U64 val) {}
		constexpr void WaitCompute(U64 val) {}
		constexpr void WaitCopy(U64 val) {}

		constexpr U64 SetMainFenceCPU() { return 0; }
		constexpr U64 SetComputeFenceCPU() { return 0; }
		constexpr U64 SetCopyFenceCPU() { return 0; }

		constexpr void WaitMainFromCompute(U64 val) {}
		constexpr void WaitMainFromCopy(U64 val) {}
		constexpr void WaitComputeFromMain(U64 val) {}
		constexpr void WaitComputeFromCopy(U64 val) {}
		constexpr void WaitCopyFromMain(U64 val) {}
		constexpr void WaitCopyFromCompute(U64 val) {}

		constexpr U64 SetMainFence() { return 0; }
		constexpr U64 SetComputeFence() { return 0; }
		constexpr U64 SetCopyFence() { return 0; }

		constexpr U32 GetCommandBufferSize() const noexcept { return commandListsCount; }
		constexpr void SetCommandBufferSize(U32 count) noexcept { commandListsCount = count; }
		constexpr void FinishUpload();

		void ExecuteMain(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG) { Execute(cl); }
		void ExecuteCompute(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG) { Execute(cl); }
		void ExecuteCopy(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG) { Execute(cl); }

		void Execute(GFX::CommandList* cls, U32 count) noexcept(ZE_NO_DEBUG);

		// Gfx API Internal

#ifdef _ZE_MODE_DEBUG
		constexpr DX::DebugInfoManager& GetInfoManager() noexcept { return debugManager; }
#endif
		ID3D11Device5* GetDevice() const noexcept { return device.Get(); }
		ID3D11DeviceContext4* GetMainContext() const noexcept { return context.Get(); }
	};
}