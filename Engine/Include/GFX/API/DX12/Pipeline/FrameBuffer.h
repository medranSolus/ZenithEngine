#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/Pipeline/FrameBufferDesc.h"
#include "GFX/Pipeline/SyncType.h"
#include "GFX/Graphics.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12::Pipeline
{
	class FrameBuffer final
	{
		struct ResourceInfo
		{
			RID Handle;
			U32 Offset;
			U32 Chunks;
			U64 StartLevel;
			U64 LastLevel;
			D3D12_RESOURCE_DESC Desc;
			D3D12_CLEAR_VALUE ClearVal;
			DX::ComPtr<ID3D12Resource> Resource;
			std::bitset<6> Flags;

			constexpr bool IsRTV() const noexcept { return Flags[0]; }
			constexpr bool IsDSV() const noexcept { return Flags[1]; }
			constexpr bool IsSRV() const noexcept { return Flags[2]; }
			constexpr bool IsUAV() const noexcept { return Flags[3]; }
			constexpr bool IsCube() const noexcept { return Flags[4]; }
			constexpr bool IsAliasing() const noexcept { return Flags[5]; }
			void SetAliasing() noexcept { Flags[5] = true; }
		};
		struct LevelTransition
		{
			U32 BarrierCount = 0;
			Ptr<D3D12_RESOURCE_BARRIER> EntryBarriers;
			Ptr<std::pair<Ptr<D3D12_RESOURCE_BARRIER>, U32>> ExitBarriers;
		};
		struct BufferData
		{
			DX::ComPtr<ID3D12Resource> Resource;
			UInt2 Size;
		};

		U16 transitionLevels = 0;
		Ptr<LevelTransition> gfxTransitions;
		Ptr<LevelTransition> computeTransitions;
		Ptr<std::pair<Ptr<D3D12_RESOURCE_BARRIER>, U32>> syncedEntryTransitions;
		Ptr<std::pair<Ptr<D3D12_RESOURCE_BARRIER>, U32>> syncedExitTransitions; // No last transition

		U32 backbufferBarriersCount = 0;
		Ptr<D3D12_RESOURCE_BARRIER*> backbufferBarriers;
		Ptr<bool> aliasingResources;
		Ptr<BufferData> resources;

		RID resourceCount;
		Ptr<D3D12_CPU_DESCRIPTOR_HANDLE> rtvDsv;
		Ptr<Ptr<D3D12_CPU_DESCRIPTOR_HANDLE>> rtvDsvMips; // No backbuffer
		Ptr<D3D12_GPU_DESCRIPTOR_HANDLE> srv;
		Ptr<std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE>> uav; // No backbuffer
		Ptr<Ptr<std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE>>> uavMips; // No backbuffer

		DX::ComPtr<ID3D12DescriptorHeap> rtvDescHeap;
		DX::ComPtr<ID3D12DescriptorHeap> dsvDescHeap;
		DX::ComPtr<ID3D12DescriptorHeap> uavDescHeap;
		DX::ComPtr<ID3D12Heap> mainHeap;
		DX::ComPtr<ID3D12Heap> uavHeap;
#define _ZE_DEBUG_FRAME_NO_ALIASING_MEMORY
#ifdef _ZE_DEBUG_FRAME_MEMORY_PRINT
		static void PrintMemory(std::string&& memID, U32 maxChunks, U64 levelCount,
			RID invalidID, const std::vector<RID>& memory, U64 heapSize);
#endif
#ifndef _ZE_DEBUG_FRAME_NO_ALIASING_MEMORY
		static U64 FindHeapSize(U32 maxChunks, U64 levelCount, RID invalidID, const std::vector<RID>& memory) noexcept;
		static bool CheckResourceAliasing(U32 offset, U32 chunks, U64 startLevel, U64 lastLevel,
			U32 maxChunks, U64 levelCount, RID invalidID, const std::vector<RID>& memory) noexcept;
#endif
		static U32 AllocResource(RID id, U32 chunks, U64 startLevel, U64 lastLevel,
			U32 maxChunks, U64 levelCount, RID invalidID, std::vector<RID>& memory);
		static U16 GetRenderLevel(U64 passLevel, GFX::Pipeline::FrameBufferDesc& desc) noexcept;
		static D3D12_RESOURCE_BARRIER CreateResource(ResourceInfo& res, ID3D12Heap* heap,
			Device& dev, GFX::Pipeline::FrameBufferDesc& desc);

		bool FillBarrier(const GFX::Pipeline::TransitionDesc& transition, D3D12_RESOURCE_BARRIER& barrier) const noexcept;
		bool FillBarrierLevelGfx(U32& barrierCount, U64 levelIndex,
			D3D12_RESOURCE_BARRIER*& barrierBuffer, GFX::Pipeline::FrameBufferDesc& desc) noexcept;
		void FillBarrierLevelCompute(U32& barrierCount, U64 levelIndex,
			D3D12_RESOURCE_BARRIER*& barrierBuffer, GFX::Pipeline::FrameBufferDesc& desc) noexcept;
		void InitResource(CommandList& cl, RID rid) const noexcept;
		void SetupViewport(D3D12_VIEWPORT& viewport, D3D12_RECT& scissorRect, RID rid) const noexcept;
		void SetViewport(CommandList& cl, RID rid) const noexcept;

	public:
		FrameBuffer() = default;
		FrameBuffer(GFX::Graphics& gfx, GFX::Pipeline::FrameBufferDesc& desc);
		ZE_CLASS_DELETE(FrameBuffer);
		~FrameBuffer();

		constexpr UInt2 GetDimmensions(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Size; }

		void InitRTV(GFX::CommandList& cl, RID rid) const noexcept { InitResource(cl.Get().dx12, rid); }
		void InitDSV(GFX::CommandList& cl, RID rid) const noexcept { InitResource(cl.Get().dx12, rid); }

		void Copy(GFX::CommandList& cl, RID src, RID dest) const noexcept;

		void SetRTV(GFX::CommandList& cl, RID rid) const noexcept;
		void SetRTV(GFX::CommandList& cl, RID rid, U16 mipLevel) const noexcept;
		void SetDSV(GFX::CommandList& cl, RID rid) const noexcept;
		void SetDSV(GFX::CommandList& cl, RID rid, U16 mipLevel) const noexcept;
		void SetOutput(GFX::CommandList& cl, RID rtv, RID dsv) const noexcept;

		template<U32 RTVCount>
		void SetRTV(GFX::CommandList& cl, const RID* rid, bool adjacent) const noexcept;
		template<U32 RTVCount>
		void SetOutput(GFX::CommandList& cl, const RID* rtv, RID dsv, bool adjacent) const noexcept;

		void SetSRV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid) const noexcept;
		void SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid) const noexcept;
		void SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid, U16 mipLevel) const noexcept;

		void BarrierUAV(GFX::CommandList& cl, RID rid) const noexcept;
		void BarrierTransition(GFX::CommandList& cl, RID rid, GFX::Resource::State before, GFX::Resource::State after) const noexcept;
		template<U32 BarrierCount>
		void BarrierTransition(GFX::CommandList& cl, const std::array<GFX::Pipeline::TransitionInfo, BarrierCount>& barriers) const noexcept;

		void ClearRTV(GFX::CommandList& cl, RID rid, const ColorF4& color) const noexcept;
		void ClearDSV(GFX::CommandList& cl, RID rid, float depth, U8 stencil) const noexcept;
		void ClearUAV(GFX::CommandList& cl, RID rid, const ColorF4& color) const noexcept;
		void ClearUAV(GFX::CommandList& cl, RID rid, const Pixel colors[4]) const noexcept;

		void SwapBackbuffer(GFX::Device& dev, GFX::SwapChain& swapChain) noexcept;
		void SyncedEntryTransitions(GFX::CommandList& cl, U16 renderLevel) const;
		void SyncedExitTransitions(GFX::CommandList& cl, U16 renderLevel) const;
		void EntryTransitions(GFX::CommandList& cl, QueueType queue, U16 renderLevel) const;
		void ExitTransitions(GFX::CommandList& cl, QueueType queue, U16 renderLevel, U16 passlevel) const;
	};

#pragma region Functions
	template<U32 RTVCount>
	void FrameBuffer::SetRTV(GFX::CommandList& cl, const RID* rid, bool adjacent) const noexcept
	{
		static_assert(RTVCount > 1, "For performance reasons FrameBuffer::SetRTV() should be only used for multiple render targets!");

		D3D12_CPU_DESCRIPTOR_HANDLE handles[RTVCount];
		D3D12_VIEWPORT vieports[RTVCount];
		D3D12_RECT scissorRects[RTVCount];
		for (U32 i = 0; i < RTVCount; ++i)
		{
			RID id = rid[i];
			ZE_ASSERT(id < resourceCount, "Resource ID outside available range!");

			handles[i] = rtvDsv[id];
			ZE_ASSERT(handles[i].ptr != -1, "Current resource is not suitable for being render target!");
			SetupViewport(vieports[i], scissorRects[i], id);
		}
		cl.Get().dx12.GetList()->RSSetViewports(RTVCount, vieports);
		cl.Get().dx12.GetList()->RSSetScissorRects(RTVCount, scissorRects);
		cl.Get().dx12.GetList()->OMSetRenderTargets(RTVCount, handles, adjacent, nullptr);
	}

	template<U32 RTVCount>
	void FrameBuffer::SetOutput(GFX::CommandList& cl, const RID* rtv, RID dsv, bool adjacent) const noexcept
	{
		static_assert(RTVCount > 1, "For performance reasons FrameBuffer::SetOutput() should be only used for multiple render targets!");
		ZE_ASSERT(dsv != 0, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(rtvDsv[dsv].ptr != -1, "Current resource is not suitable for being depth stencil!");

		D3D12_CPU_DESCRIPTOR_HANDLE handles[RTVCount];
		D3D12_VIEWPORT vieports[RTVCount];
		D3D12_RECT scissorRects[RTVCount];
		for (U32 i = 0; i < RTVCount; ++i)
		{
			RID id = rtv[i];
			ZE_ASSERT(id < resourceCount, "Resource ID outside available range!");

			handles[i] = rtvDsv[id];
			ZE_ASSERT(handles[i].ptr != -1, "Current resource is not suitable for being render target!");
			SetupViewport(vieports[i], scissorRects[i], id);
		}
		cl.Get().dx12.GetList()->RSSetViewports(RTVCount, vieports);
		cl.Get().dx12.GetList()->RSSetScissorRects(RTVCount, scissorRects);
		cl.Get().dx12.GetList()->OMSetRenderTargets(RTVCount, handles, adjacent, rtvDsv + dsv);
	}

	template<U32 BarrierCount>
	void FrameBuffer::BarrierTransition(GFX::CommandList& cl, const std::array<GFX::Pipeline::TransitionInfo, BarrierCount>& barriers) const noexcept
	{
		static_assert(BarrierCount > 1, "For performance reasons FrameBuffer::BarrierTransition() should be only used for multiple barriers!");

		D3D12_RESOURCE_BARRIER transitions[BarrierCount];
		for (U32 i = 0; i < BarrierCount; ++i)
		{
			const GFX::Pipeline::TransitionInfo& info = barriers.at(i);
			ZE_ASSERT(info.RID < resourceCount, "Resource ID outside available range!");

			ZE_ASSERT(info.BeforeState != GFX::Resource::StateUnorderedAccess ||
				info.BeforeState == GFX::Resource::StateUnorderedAccess && uav[info.RID - 1].first.ptr != -1,
				"Current resource is not suitable for being unnordered access!");

			ZE_ASSERT(info.BeforeState != GFX::Resource::StateRenderTarget ||
				info.BeforeState == GFX::Resource::StateRenderTarget && rtvDsv[info.RID].ptr != -1,
				"Current resource is not suitable for being render target!");

			ZE_ASSERT(info.BeforeState != GFX::Resource::StateDepthRead ||
				info.BeforeState == GFX::Resource::StateDepthRead && rtvDsv[info.RID].ptr != -1,
				"Current resource is not suitable for being depth stencil!");
			ZE_ASSERT(info.BeforeState != GFX::Resource::StateDepthWrite ||
				info.BeforeState == GFX::Resource::StateDepthWrite && rtvDsv[info.RID].ptr != -1,
				"Current resource is not suitable for being depth stencil!");

			ZE_ASSERT(info.BeforeState != GFX::Resource::StateShaderResourceAll ||
				info.BeforeState == GFX::Resource::StateShaderResourceAll && srv[info.RID].ptr != -1,
				"Current resource is not suitable for being shader resource!");
			ZE_ASSERT(info.BeforeState != GFX::Resource::StateShaderResourcePS ||
				info.BeforeState == GFX::Resource::StateShaderResourcePS && srv[info.RID].ptr != -1,
				"Current resource is not suitable for being shader resource!");
			ZE_ASSERT(info.BeforeState != GFX::Resource::StateShaderResourceNonPS ||
				info.BeforeState == GFX::Resource::StateShaderResourceNonPS && srv[info.RID].ptr != -1,
				"Current resource is not suitable for being shader resource!");

			D3D12_RESOURCE_BARRIER& barrier = transitions[i];
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = resources[info.RID].Resource.Get();
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrier.Transition.StateBefore = GetResourceState(info.BeforeState);
			barrier.Transition.StateAfter = GetResourceState(info.AfterState);
		}
		cl.Get().dx12.GetList()->ResourceBarrier(BarrierCount, transitions);
	}
#pragma endregion
}