#pragma once
#if _ZE_FFX_API_ENABLED
#	include "GFX/External/FfxApiFunctions.h"
#	include "GFX/Pipeline/FrameBuffer.h"

namespace ZE::RHI::DX12::External
{
	class FfxApiInterface final
	{
		struct ResourceAllocation
		{
			DX::ComPtr<IHeap> aliasableHeap;
			U64 startOffset = 0;
			U64 availableSize = 0;
			U64 nextOffset = 0;
		};

		HMODULE ffxApiDll = nullptr;
		PfnFfxCreateContext ffxCreateContext = nullptr;
		GFX::External::FfxApiFunctions ffxFunctions = {};

		DX::ComPtr<IDevice> device;
		std::set<U64> aliasableResources;
		std::unordered_map<U32, ResourceAllocation> effectAllocs;
		ResourceAllocation newAliasableRegion = {};

		static ffxReturnCode_t ResourceAllocCallback(U32 effectId, D3D12_RESOURCE_STATES initialState,
			const D3D12_HEAP_PROPERTIES* heapProps, const D3D12_RESOURCE_DESC* desc, const struct FfxApiResourceDescription* ffxDesc,
			const D3D12_CLEAR_VALUE* clearVal, ID3D12Resource** resource) noexcept;
		static ffxReturnCode_t ResourceDeallocCallback(U32 effectId, ID3D12Resource* resource) noexcept;
		static ffxReturnCode_t HeapAllocCallback(U32 effectId, const D3D12_HEAP_DESC* heapDesc, bool aliasable, ID3D12Heap** heap, U64* startOffset) noexcept;
		static ffxReturnCode_t HeapDeallocCallback(U32 effectId, ID3D12Heap* heap, U64 startOffset, U64 heapSize) noexcept;

		ResourceAllocation* AcquireRegion(U32 effectId) noexcept;
		bool RemoveAllocation(U32 effectId, U64 bytes) noexcept;
		void Destroy() noexcept;
		void MoveFrom(FfxApiInterface&& ffxInt) noexcept;

	public:
		FfxApiInterface() = default;
		ZE_CLASS_NO_COPY(FfxApiInterface);
		FfxApiInterface(FfxApiInterface&& ffxInt) noexcept { MoveFrom(std::move(ffxInt)); }
		FfxApiInterface& operator=(FfxApiInterface&& ffxInt) noexcept { Destroy(); MoveFrom(std::move(ffxInt)); return *this; }
		~FfxApiInterface() { Destroy(); }

		static Expected<FfxApiInterface> Create(GFX::Device& dev) noexcept;

		constexpr bool IsInitialized() const noexcept { return ffxApiDll; }
		constexpr const GFX::External::FfxApiFunctions* GetFunctions() const noexcept { return &ffxFunctions; }

		ffxReturnCode_t CreateFfxCtx(GFX::Device& dev, GFX::Pipeline::FrameBuffer& buffers, ffxContext* ctx, ffxCreateContextDescHeader& ctxHeader, RID aliasableRegion) noexcept;
	};
}
#endif