#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/Pipeline/FrameBufferDesc.h"
#include "GFX/SwapChain.h"

namespace ZE::RHI::DX11::Pipeline
{
	class FrameBuffer final
	{
		struct BufferData
		{
			DX::ComPtr<IResource> Resource;
			UInt2 Size;
			U16 Array;
			U16 Mips;
			PixelFormat Format;
			std::bitset<2> Flags;

			constexpr bool IsCube() const noexcept { return Flags[0]; }
			constexpr void SetCube() noexcept { Flags[0] = true; }
			constexpr bool IsArrayView() const noexcept { return Flags[1]; }
			constexpr void SetArrayView() noexcept { Flags[1] = true; }
		};

		RID resourceCount;
		Ptr<BufferData> resources;
		// Is SRV | correct binding slots
		mutable std::vector<std::pair<bool, Binding::Schema::SlotData>> currentSlots;

		Ptr<DX::ComPtr<IRenderTargetView>> rtvs;
		Ptr<DX::ComPtr<IDepthStencilView>> dsvs; // No backbuffer
		Ptr<DX::ComPtr<IShaderResourceView>> srvs;
		Ptr<DX::ComPtr<IUnorderedAccessView>> uavs; // No backbuffer

		Ptr<Ptr<DX::ComPtr<IRenderTargetView>>> rtvMips; // No backbuffer
		Ptr<Ptr<DX::ComPtr<IDepthStencilView>>> dsvMips; // No backbuffer
		Ptr<Ptr<DX::ComPtr<IUnorderedAccessView>>> uavMips; // No backbuffer

		void SetupViewport(D3D11_VIEWPORT& viewport, RID rid) const noexcept;
		void SetViewport(CommandList& cl, RID rid) const noexcept;

	public:
		FrameBuffer() = default;
		FrameBuffer(GFX::Device& dev, GFX::CommandList& mainList,
			const GFX::Pipeline::FrameBufferDesc& desc);
		ZE_CLASS_DELETE(FrameBuffer);
		~FrameBuffer();

		constexpr UInt2 GetDimmensions(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Size; }
		constexpr U16 GetArraySize(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Array; }
		constexpr U16 GetMipCount(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Mips; }
		constexpr bool IsCubeTexture(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].IsCube(); }
		constexpr bool IsArrayView(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].IsArrayView(); }
		constexpr bool IsUAV(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); if (rid == 0) return false; return uavs[rid - 1] != nullptr; }
		constexpr PixelFormat GetFormat(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Format; }

		constexpr void InitRTV(GFX::CommandList& cl, RID rid) const noexcept {}
		constexpr void InitDSV(GFX::CommandList& cl, RID rid) const noexcept {}

		void Copy(GFX::CommandList& cl, RID src, RID dest) const noexcept;

		void SetRTV(GFX::CommandList& cl, RID rid) const noexcept;
		void SetRTV(GFX::CommandList& cl, RID rid, U16 mipLevel) const noexcept;
		void SetDSV(GFX::CommandList& cl, RID rid) const noexcept;
		void SetDSV(GFX::CommandList& cl, RID rid, U16 mipLevel) const noexcept;
		void SetOutput(GFX::CommandList& cl, RID rtv, RID dsv) const noexcept;

		void SetRTVSparse(GFX::CommandList& cl, const RID* rid, U8 count) const noexcept;
		void SetOutputSparse(GFX::CommandList& cl, const RID* rtv, RID dsv, U8 count) const noexcept;

		template<U32 RTVCount>
		void SetRTV(GFX::CommandList& cl, const RID* rid, bool adjacent) const noexcept;
		template<U32 RTVCount>
		void SetOutput(GFX::CommandList& cl, const RID* rtv, RID dsv, bool adjacent) const noexcept;

		void SetSRV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid) const noexcept;
		void SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid) const noexcept;
		void SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid, U16 mipLevel) const noexcept;

		constexpr void BarrierUAV(GFX::CommandList& cl, RID rid) const noexcept {}
		void BarrierTransition(GFX::CommandList& cl, RID rid, GFX::Resource::State before, GFX::Resource::State after) const noexcept;
		template<U32 BarrierCount>
		void BarrierTransition(GFX::CommandList& cl, const std::array<GFX::Pipeline::TransitionInfo, BarrierCount>& barriers) const noexcept;

		void ClearRTV(GFX::CommandList& cl, RID rid, const ColorF4& color) const noexcept;
		void ClearDSV(GFX::CommandList& cl, RID rid, float depth, U8 stencil) const noexcept;
		void ClearUAV(GFX::CommandList& cl, RID rid, const ColorF4& color) const noexcept;
		void ClearUAV(GFX::CommandList& cl, RID rid, const Pixel colors[4]) const noexcept;

		void SwapBackbuffer(GFX::Device& dev, GFX::SwapChain& swapChain) noexcept;
		constexpr void InitTransitions(GFX::Device& dev, GFX::CommandList& cl) const noexcept {}
		void ExitTransitions(GFX::Device& dev, GFX::CommandList& cl, U64 level) const noexcept;

		constexpr void Free(GFX::Device& dev) noexcept {}
	};

#pragma region Functions
	template<U32 RTVCount>
	void FrameBuffer::SetRTV(GFX::CommandList& cl, const RID* rid, bool adjacent) const noexcept
	{
		static_assert(RTVCount > 1, "For performance reasons FrameBuffer::SetRTV() should be only used for multiple render targets!");

		ID3D11RenderTargetView* handles[RTVCount];
		D3D11_VIEWPORT vieports[RTVCount];
		for (U32 i = 0; i < RTVCount; ++i)
		{
			RID id = rid[i];
			ZE_ASSERT(id < resourceCount, "Resource ID outside available range!");

			handles[i] = static_cast<ID3D11RenderTargetView*>(rtvs[id].Get());
			ZE_ASSERT(handles[i], "Current resource is not suitable for being render target!");
			SetupViewport(vieports[i], id);
		}
		cl.Get().dx11.GetContext()->RSSetViewports(RTVCount, vieports);
		cl.Get().dx11.GetContext()->OMSetRenderTargets(RTVCount, handles, nullptr);
	}

	template<U32 RTVCount>
	void FrameBuffer::SetOutput(GFX::CommandList& cl, const RID* rtv, RID dsv, bool adjacent) const noexcept
	{
		static_assert(RTVCount > 1, "For performance reasons FrameBuffer::SetOutput() should be only used for multiple render targets!");
		ZE_ASSERT(dsv != 0, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(dsvs[dsv - 1], "Current resource is not suitable for being depth stencil!");

		ID3D11RenderTargetView* handles[RTVCount];
		D3D11_VIEWPORT vieports[RTVCount];
		for (U32 i = 0; i < RTVCount; ++i)
		{
			RID id = rtv[i];
			ZE_ASSERT(id < resourceCount, "Resource ID outside available range!");

			handles[i] = static_cast<ID3D11RenderTargetView*>(rtvs[id].Get());
			ZE_ASSERT(handles[i], "Current resource is not suitable for being render target!");
			SetupViewport(vieports[i], id);
		}
		cl.Get().dx11.GetContext()->RSSetViewports(RTVCount, vieports);
		cl.Get().dx11.GetContext()->OMSetRenderTargets(RTVCount, handles, dsvs[dsv - 1].Get());
	}

	template<U32 BarrierCount>
	void FrameBuffer::BarrierTransition(GFX::CommandList& cl, const std::array<GFX::Pipeline::TransitionInfo, BarrierCount>& barriers) const noexcept
	{
		for (const auto& barrier : barriers)
			BarrierTransition(cl, barrier.RID, barrier.BeforeState, barrier.AfterState);
	}
#pragma endregion
}