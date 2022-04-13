#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/Pipeline/FrameBufferDesc.h"
#include "GFX/SwapChain.h"
#include "D3D11.h"

namespace ZE::GFX::API::DX11::Pipeline
{
	class FrameBuffer final
	{
		const DX::ComPtr<ID3D11UnorderedAccessView> nullUAV = nullptr;
		const DX::ComPtr<ID3D11ShaderResourceView> nullSRV = nullptr;

		RID resourceCount;
		Ptr<UInt2> dimmensions;

		Ptr<DX::ComPtr<ID3D11RenderTargetView1>> rtvs;
		Ptr<DX::ComPtr<ID3D11DepthStencilView>> dsvs; // No backbuffer
		Ptr<DX::ComPtr<ID3D11ShaderResourceView1>> srvs;
		Ptr<DX::ComPtr<ID3D11UnorderedAccessView1>> uavs; // No backbuffer

		Ptr<Ptr<DX::ComPtr<ID3D11RenderTargetView1>>> rtvMips; // No backbuffer
		Ptr<Ptr<DX::ComPtr<ID3D11DepthStencilView>>> dsvMips; // No backbuffer
		Ptr<Ptr<DX::ComPtr<ID3D11UnorderedAccessView1>>> uavMips; // No backbuffer

		void SetupViewport(D3D11_VIEWPORT& viewport, RID rid) const noexcept;
		void SetViewport(CommandList& cl, RID rid) const noexcept;

	public:
		FrameBuffer() = default;
		FrameBuffer(GFX::Device& dev, GFX::CommandList& mainList,
			const GFX::Pipeline::FrameBufferDesc& desc);
		ZE_CLASS_DELETE(FrameBuffer);
		~FrameBuffer();

		constexpr UInt2 GetDimmensions(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return dimmensions[rid]; }

		constexpr void InitRTV(GFX::CommandList& cl, RID rid) const noexcept {}
		constexpr void InitDSV(GFX::CommandList& cl, RID rid) const noexcept {}

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

			handles[i] = reinterpret_cast<ID3D11RenderTargetView*>(rtvs[id].Get());
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

			handles[i] = reinterpret_cast<ID3D11RenderTargetView*>(rtvs[id].Get());
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