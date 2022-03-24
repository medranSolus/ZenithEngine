#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/Pipeline/FrameBufferDesc.h"
#include "GFX/Pipeline/SyncType.h"
#include "GFX/CommandList.h"
#include "GFX/SwapChain.h"
#include "D3D12.h"

namespace ZE::GFX::API::DX12::Pipeline
{
	class FrameBuffer final
	{
		struct TransitionPoint
		{
			U32 BarrierCount = 0;
			Ptr<D3D12_RESOURCE_BARRIER> Barriers;
			GFX::Pipeline::SyncType AfterSync = GFX::Pipeline::SyncType::None;
		};
		struct BufferData
		{
			DX::ComPtr<ID3D12Resource> Resource;
			U32 Width;
			U32 Height;
		};

		TransitionPoint initTransitions;
		Ptr<GFX::Pipeline::SyncType> transitionSyncs;
		Ptr<TransitionPoint> transitions;

		U64 backbufferBarriersLocationsCount = 0;
		Ptr<U64> backbufferBarriersLocations;
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

#ifdef _ZE_DEBUG_FRAME_MEMORY_PRINT
		static void PrintMemory(std::string&& memID, U32 maxChunks, U64 levelCount,
			RID invalidID, const std::vector<RID>& memory, U64 heapSize);
#endif
		static U64 FindHeapSize(U32 maxChunks, U64 levelCount, RID invalidID, const std::vector<RID>& memory) noexcept;
		static bool CheckResourceAliasing(U32 offset, U32 chunks, U64 startLevel, U64 lastLevel,
			U32 maxChunks, U64 levelCount, RID invalidID, const std::vector<RID>& memory) noexcept;
		static U32 AllocResource(RID id, U32 chunks, U64 startLevel, U64 lastLevel,
			U32 maxChunks, U64 levelCount, RID invalidID, std::vector<RID>& memory);

		void InitResource(CommandList& cl, RID rid) const noexcept;
		void SetupViewport(D3D12_VIEWPORT& viewport, D3D12_RECT& scissorRect, RID rid) const noexcept;
		void SetViewport(CommandList& cl, RID rid) const noexcept;

	public:
		FrameBuffer() = default;
		FrameBuffer(GFX::Device& dev, GFX::CommandList& mainList, GFX::Pipeline::FrameBufferDesc& desc);
		ZE_CLASS_DELETE(FrameBuffer);
		~FrameBuffer();

		UInt2 GetDimmensions(RID rid) const noexcept { const auto& data = resources[rid]; return { data.Width, data.Height }; }

		void InitRTV(GFX::CommandList& cl, RID rid) const noexcept { InitResource(cl.Get().dx12, rid); }
		void InitDSV(GFX::CommandList& cl, RID rid) const noexcept { InitResource(cl.Get().dx12, rid); }

		void SetRTV(GFX::CommandList& cl, RID rid) const;
		void SetRTV(GFX::CommandList& cl, RID rid, U16 mipLevel) const;
		void SetDSV(GFX::CommandList& cl, RID rid) const;
		void SetDSV(GFX::CommandList& cl, RID rid, U16 mipLevel) const;
		void SetOutput(GFX::CommandList& cl, RID rtv, RID dsv) const;

		template<U32 RTVCount>
		void SetRTV(GFX::CommandList& cl, const RID* rid, bool adjacent) const;
		template<U32 RTVCount>
		void SetOutput(GFX::CommandList& cl, const RID* rtv, RID dsv, bool adjacent) const;

		void SetSRV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid) const;
		void SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid) const;
		void SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid, U16 mipLevel) const;

		void BarrierUAV(GFX::CommandList& cl, RID rid) const;
		void BarrierTransition(GFX::CommandList& cl, RID rid, GFX::Resource::State before, GFX::Resource::State after) const;
		template<U32 BarrierCount>
		void BarrierTransition(GFX::CommandList& cl, const std::array<GFX::Pipeline::TransitionInfo, BarrierCount>& barriers) const;

		void ClearRTV(GFX::CommandList& cl, RID rid, const ColorF4& color) const;
		void ClearDSV(GFX::CommandList& cl, RID rid, float depth, U8 stencil) const;
		void ClearUAV(GFX::CommandList& cl, RID rid, const ColorF4& color) const;
		void ClearUAV(GFX::CommandList& cl, RID rid, const Pixel colors[4]) const;

		void SwapBackbuffer(GFX::Device& dev, GFX::SwapChain& swapChain);
		void InitTransitions(GFX::Device& dev, GFX::CommandList& cl) const;
		void ExitTransitions(GFX::Device& dev, GFX::CommandList& cl, U64 level) const noexcept;
	};

#pragma region Functions
	template<U32 RTVCount>
	void FrameBuffer::SetRTV(GFX::CommandList& cl, const RID* rid, bool adjacent) const
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
	void FrameBuffer::SetOutput(GFX::CommandList& cl, const RID* rtv, RID dsv, bool adjacent) const
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
	void FrameBuffer::BarrierTransition(GFX::CommandList& cl, const std::array<GFX::Pipeline::TransitionInfo, BarrierCount>& barriers) const
	{
		static_assert(BarrierCount > 1, "For performance reasons FrameBuffer::BarrierTransition() should be only used for multiple barriers!");

		D3D12_RESOURCE_BARRIER transitions[BarrierCount];
		for (U32 i = 0; i < BarrierCount; ++i)
		{
			const GFX::Pipeline::TransitionInfo& info = barriers.at(i);
			ZE_ASSERT(info.RID < resourceCount, "Resource ID outside available range!");

			ZE_ASSERT(info.BeforeState != GFX::Resource::State::UnorderedAccess ||
				info.BeforeState == GFX::Resource::State::UnorderedAccess && uav[info.RID - 1].first.ptr != -1,
				"Current resource is not suitable for being unnordered access!");

			ZE_ASSERT(info.BeforeState != GFX::Resource::State::RenderTarget ||
				info.BeforeState == GFX::Resource::State::RenderTarget && rtvDsv[info.RID].ptr != -1,
				"Current resource is not suitable for being render target!");

			ZE_ASSERT(info.BeforeState != GFX::Resource::State::DepthRead ||
				info.BeforeState == GFX::Resource::State::DepthRead && rtvDsv[info.RID].ptr != -1,
				"Current resource is not suitable for being depth stencil!");
			ZE_ASSERT(info.BeforeState != GFX::Resource::State::DepthWrite ||
				info.BeforeState == GFX::Resource::State::DepthWrite && rtvDsv[info.RID].ptr != -1,
				"Current resource is not suitable for being depth stencil!");

			ZE_ASSERT(info.BeforeState != GFX::Resource::State::ShaderResourceAll ||
				info.BeforeState == GFX::Resource::State::ShaderResourceAll && srv[info.RID].ptr != -1,
				"Current resource is not suitable for being shader resource!");
			ZE_ASSERT(info.BeforeState != GFX::Resource::State::ShaderResourcePS ||
				info.BeforeState == GFX::Resource::State::ShaderResourcePS && srv[info.RID].ptr != -1,
				"Current resource is not suitable for being shader resource!");
			ZE_ASSERT(info.BeforeState != GFX::Resource::State::ShaderResourceNonPS ||
				info.BeforeState == GFX::Resource::State::ShaderResourceNonPS && srv[info.RID].ptr != -1,
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