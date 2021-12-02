#pragma once
#include "GFX/API/DX11/Device.h"
#include "GFX/API/DX12/Device.h"
#include "GFX/API/Backend.h"

namespace ZE::GFX
{
	// Resource allocation
	class Device final
	{
		ZE_API_BACKEND(Device) backend;

	public:
		Device() = default;
		Device(Device&&) = default;
		Device(const Device&) = delete;
		Device& operator=(Device&&) = default;
		Device& operator=(const Device&) = delete;
		~Device() { WaitMain(SetMainFence()); }

		constexpr void Init(U32 descriptorCount, U32 scratchDescriptorCount) { backend.Init(descriptorCount, scratchDescriptorCount); }
		constexpr ZE_API_BACKEND(Device)& Get() noexcept { return backend; }
		constexpr void SwitchApi(GfxApiType nextApi);

		// Main Gfx API

		constexpr U64 GetMainFence() const noexcept { U64 val; ZE_API_BACKEND_CALL_RET(backend, val, GetMainFence); return val; }
		constexpr U64 GetComputeFence() const noexcept { U64 val; ZE_API_BACKEND_CALL_RET(backend, val, GetComputeFence); return val; }
		constexpr U64 GetCopyFence() const noexcept { U64 val; ZE_API_BACKEND_CALL_RET(backend, val, GetCopyFence); return val; }

		// CPU side wait for main queue
		constexpr void WaitMain(U64 val) { ZE_API_BACKEND_CALL(backend, WaitMain, val); }
		// CPU side wait for compute queue
		constexpr void WaitCompute(U64 val) { ZE_API_BACKEND_CALL(backend, WaitCompute, val); }
		// CPU side wait for copy queue
		constexpr void WaitCopy(U64 val) { ZE_API_BACKEND_CALL(backend, WaitCopy, val); }

		// Set fence for main queue from CPU
		constexpr U64 SetMainFenceCPU() { U64 val; ZE_API_BACKEND_CALL_RET(backend, val, SetMainFenceCPU); return val; }
		// Set fence for compute queue from CPU
		constexpr U64 SetComputeFenceCPU() { U64 val; ZE_API_BACKEND_CALL_RET(backend, val, SetComputeFenceCPU); return val; }
		// Set fence for copy queue from CPU
		constexpr U64 SetCopyFenceCPU() { U64 val; ZE_API_BACKEND_CALL_RET(backend, val, SetCopyFenceCPU); return val; }

		// GPU side wait for main queue till compute queue will reach fence value
		constexpr void WaitMainFromCompute(U64 val) { ZE_API_BACKEND_CALL(backend, WaitMainFromCompute, val); }
		// GPU side wait for main queue till copy queue will reach fence value
		constexpr void WaitMainFromCopy(U64 val) { ZE_API_BACKEND_CALL(backend, WaitMainFromCopy, val); }
		// GPU side wait for compute queue till main queue will reach fence value
		constexpr void WaitComputeFromMain(U64 val) { ZE_API_BACKEND_CALL(backend, WaitComputeFromMain, val); }
		// GPU side wait for compute queue till copy queue will reach fence value
		constexpr void WaitComputeFromCopy(U64 val) { ZE_API_BACKEND_CALL(backend, WaitComputeFromCopy, val); }
		// GPU side wait for copy queue till main queue will reach fence value
		constexpr void WaitCopyFromMain(U64 val) { ZE_API_BACKEND_CALL(backend, WaitCopyFromMain, val); }
		// GPU side wait for copy queue till compute queue will reach fence value
		constexpr void WaitCopyFromCompute(U64 val) { ZE_API_BACKEND_CALL(backend, WaitCopyFromCompute, val); }

		// GPU side signal from main queue for it's fence
		constexpr U64 SetMainFence() { U64 val; ZE_API_BACKEND_CALL_RET(backend, val, SetMainFence); return val; }
		// GPU side signal from compute queue for it's fence
		constexpr U64 SetComputeFence() { U64 val; ZE_API_BACKEND_CALL_RET(backend, val, SetComputeFence); return val; }
		// GPU side signal from copy queue for it's fence
		constexpr U64 SetCopyFence() { U64 val; ZE_API_BACKEND_CALL_RET(backend, val, SetCopyFence); return val; }

		// Set max size of command lists to execute in single call to Execute()
		constexpr void SetCommandBufferSize(U32 count) noexcept { ZE_API_BACKEND_CALL(backend, SetCommandBufferSize, count); }
		constexpr void FinishUpload() { ZE_API_BACKEND_CALL(backend, FinishUpload); }

		constexpr void Execute(CommandList* cls, U32 count) noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(backend, Execute, cls, count); }
		constexpr void ExecuteMain(CommandList& cl) noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(backend, ExecuteMain, cl); }
		constexpr void ExecuteCompute(CommandList& cl) noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(backend, ExecuteCompute, cl); }
		constexpr void ExecuteCopy(CommandList& cl) noexcept(ZE_NO_DEBUG) { ZE_API_BACKEND_CALL(backend, ExecuteCopy, cl); }
	};

#pragma region Functions
	constexpr void Device::SwitchApi(GfxApiType nextApi)
	{
		std::pair<U32, U32> data;
		U32 commandCount;
		ZE_API_BACKEND_CALL_RET(backend, data, GetData);
		ZE_API_BACKEND_CALL_RET(backend, commandCount, GetCommandBufferSize);
		backend.Switch(nextApi, data.first, data.second);
		SetCommandBufferSize(commandCount);
	}
#pragma endregion
}