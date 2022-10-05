#pragma once
#include "VK.h"

namespace ZE::GFX
{
	class CommandList;
}
namespace ZE::GFX::API::VK
{
	class Device final
	{
	public:
		Device() = default;
		Device(U32 descriptorCount, U32 scratchDescriptorCount);
		ZE_CLASS_DELETE(Device);
		~Device() = default;

		constexpr std::pair<U32, U32> GetData() const noexcept { return { 0, 0 }; }

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

#ifdef _ZE_MODE_DEBUG
		void TagBeginMain(const wchar_t* tag, Pixel color) const noexcept {  }
		void TagBeginCompute(const wchar_t* tag, Pixel color) const noexcept {  }
		void TagBeginCopy(const wchar_t* tag, Pixel color) const noexcept {  }

		void TagEndMain() const noexcept {  }
		void TagEndCompute() const noexcept {  }
		void TagEndCopy() const noexcept {  }
#endif

		constexpr void BeginUploadRegion() {}
		constexpr void StartUpload() {}
		constexpr void EndUploadRegion() {}

		void ExecuteMain(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG) {}
		void ExecuteCompute(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG) {  }
		void ExecuteCopy(GFX::CommandList& cl) noexcept(ZE_NO_DEBUG) {  }

		void Execute(GFX::CommandList* cls, U32 count) noexcept(ZE_NO_DEBUG);
	};
}