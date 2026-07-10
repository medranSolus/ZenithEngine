#pragma once
#include "GFX/Binding/Context.h"
#include "GFX/Resource/Texture/Pack.h"
#include "GFX/Resource/CBuffer.h"
#include "GFX/Pipeline/FrameBufferDesc.h"
#include "GFX/CommandSignature.h"
#include "GFX/SwapChain.h"
ZE_WARNING_PUSH
#include "nvsdk_ngx_params.h"
#include "ffx_api_types.h"
ZE_WARNING_POP

namespace ZE::RHI::DX11::Pipeline
{
	class FrameBuffer final
	{
		struct BufferData
		{
			DX::ComPtr<IResource> Resource;
			UInt2 Size = {};
			U16 Array = 0;
			U16 Mips = 0;
			PixelFormat Format = PixelFormat::Unknown;
			std::bitset<6> Flags = 0;

			constexpr bool IsCube() const noexcept { return Flags[0]; }
			constexpr void SetCube() noexcept { Flags[0] = true; }
			constexpr bool IsArrayView() const noexcept { return Flags[1]; }
			constexpr void SetArrayView() noexcept { Flags[1] = true; }
			constexpr bool IsOutsideResource() const noexcept { return Flags[2]; }
			constexpr void SetOutsideResource() noexcept { Flags[2] = true; }
			constexpr bool IsBuffer() const noexcept { return Flags[3]; }
			constexpr void SetBuffer() noexcept { Flags[3] = true; }
			constexpr bool IsTex1D() const noexcept { return Flags[4]; }
			constexpr void SetTex1D() noexcept { Flags[4] = true; }
			constexpr bool IsTex3D() const noexcept { return Flags[5]; }
			constexpr void SetTex3D() noexcept { Flags[5] = true; }
		};

#if !_ZE_MODE_RELEASE
		mutable bool isRasterActive = false;
#endif
		RID resourceCount = 0;
		std::unique_ptr<BufferData[]> resources;
		// Is SRV | correct binding slots
		mutable std::vector<std::pair<bool, Binding::Schema::SlotData>> currentSlots;

		std::unique_ptr<DX::ComPtr<IRenderTargetView>[]> rtvs;
		std::unique_ptr<DX::ComPtr<IDepthStencilView>[]> dsvs; // No backbuffer
		std::unique_ptr<DX::ComPtr<IShaderResourceView>[]> srvs;
		std::unique_ptr<DX::ComPtr<IUnorderedAccessView>[]> uavs; // No backbuffer

		std::unique_ptr<std::unique_ptr<DX::ComPtr<IRenderTargetView>[]>[]> rtvMips; // No backbuffer
		std::unique_ptr<std::unique_ptr<DX::ComPtr<IDepthStencilView>[]>[]> dsvMips; // No backbuffer
		std::unique_ptr<std::unique_ptr<DX::ComPtr<IUnorderedAccessView>[]>[]> uavMips; // No backbuffer

		void EnterRaster(GFX::CommandList& cl) const noexcept;
		void SetupViewport(D3D11_VIEWPORT& viewport, RID rid) const noexcept;
		void SetViewport(CommandList& cl, RID rid) const noexcept;

	public:
		FrameBuffer() = default;
		ZE_CLASS_MOVE(FrameBuffer);
		~FrameBuffer() = default;

		static Expected<FrameBuffer> Create(GFX::Device& dev, const GFX::Pipeline::FrameBufferDesc& desc) noexcept;

		// Barriers not needed in the API
		template<U32 BarrierCount>
		constexpr void Barrier(GFX::CommandList& cl, const std::array<GFX::Pipeline::BarrierTransition, BarrierCount>& barriers) const noexcept {}
		constexpr void Barrier(GFX::CommandList& cl, const GFX::Pipeline::BarrierTransition* barriers, U32 count) const noexcept {}
		constexpr void Barrier(GFX::CommandList& cl, const GFX::Pipeline::BarrierTransition& desc) const noexcept {}

		constexpr UInt2 GetDimmensions(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Size; }
		constexpr U16 GetArraySize(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Array; }
		constexpr U16 GetMipCount(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Mips; }
		constexpr PixelFormat GetFormat(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Format; }
		constexpr bool IsUAV(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); if (rid == BACKBUFFER_RID) return false; return uavs[rid - 1] != nullptr; }
		constexpr bool IsCubeTexture(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].IsCube(); }
		constexpr bool IsTexture1D(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].IsTex1D(); }
		constexpr bool IsTexture3D(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].IsTex3D(); }
		constexpr bool IsBuffer(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].IsBuffer(); }
		constexpr bool IsArrayView(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].IsArrayView(); }

		constexpr FfxApiResource GetFfxResource(RID rid, U32 state) const noexcept { ZE_FAIL("FFX API is not supported for DX11!"); return {}; }

		template<U8 RTVCount>
		void BeginRaster(GFX::CommandList& cl, const RID* rtv, bool adjacent) const noexcept;
		template<U8 RTVCount>
		void BeginRaster(GFX::CommandList& cl, const RID* rtv, RID dsv, bool adjacent) const noexcept;

		void BeginRasterSparse(GFX::CommandList& cl, const RID* rtv, U8 count) const noexcept;
		void BeginRasterSparse(GFX::CommandList& cl, const RID* rtv, RID dsv, U8 count) const noexcept;

		void BeginRasterDepthOnly(GFX::CommandList& cl, RID dsv) const noexcept;
		void BeginRaster(GFX::CommandList& cl, RID rtv, RID dsv) const noexcept;

		void BeginRasterDepthOnly(GFX::CommandList& cl, RID dsv, U16 mipLevel) const noexcept;
		void BeginRaster(GFX::CommandList& cl, RID rtv, RID dsv, U16 mipLevel) const noexcept;

		void SetSRV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID srv, U32 adjacentCount) const noexcept;
		void SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID uav, U32 adjacentCount) const noexcept;
		void SetUAVMip(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID uav, U16 mipLevel) const noexcept;
		void SetResourceNGX(NVSDK_NGX_Parameter* param, std::string_view name, RID res) const noexcept;

		void EndRaster(GFX::CommandList& cl) const noexcept;

		void ClearRTV(GFX::CommandList& cl, RID rtv, const ColorF4& color) const noexcept;
		void ClearDSV(GFX::CommandList& cl, RID dsv, float depth, U8 stencil) const noexcept;
		void ClearUAV(GFX::CommandList& cl, RID uav, const ColorF4& color) const noexcept;
		void ClearUAV(GFX::CommandList& cl, RID uav, const Pixel colors[4]) const noexcept;

		void Copy(GFX::Device& dev, GFX::CommandList& cl, RID src, RID dest) const noexcept;
		void CopyFullResource(GFX::CommandList& cl, RID src, RID dest) const noexcept;
		void CopyBufferRegion(GFX::CommandList& cl, RID src, U64 srcOffset, RID dest, U64 destOffset, U64 bytes) const noexcept;

		void InitResource(GFX::CommandList& cl, RID rid, const GFX::Resource::CBuffer& buffer) const noexcept;
		void InitResource(GFX::CommandList& cl, RID rid, const GFX::Resource::Texture::Pack& texture, U32 index) const noexcept;

		Status RegisterOutsideResource(RID rid, GFX::Resource::Texture::Pack& textures, U32 textureIndex, GFX::Pipeline::FrameResourceType type) noexcept;

		Status MapResource(GFX::Device& dev, RID rid, void** ptr) const noexcept;
		void UnmapResource(RID rid) const noexcept;

		Status ExecuteIndirect(GFX::CommandList& cl, GFX::CommandSignature& signature, RID commandsBuffer, U32 commandsOffset) const noexcept;
		Status SwapBackbuffer(GFX::Device& dev, GFX::SwapChain& swapChain) noexcept;

		// Gfx API Internal
		
		DX::ComPtr<IResource> GetResource(RID rid) const noexcept { ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!"); return resources[rid].Resource; }
		IRenderTargetView* GetRTV(RID rtv) const noexcept { ZE_ASSERT(rtv < resourceCount, "Resource ID outside available range!"); return rtvs[rtv].Get(); }
		IShaderResourceView* GetSRV(RID srv) const noexcept { ZE_ASSERT(srv < resourceCount, "Resource ID outside available range!"); return srvs[srv].Get(); }
		IDepthStencilView* GetDSV(RID dsv) const noexcept { ZE_ASSERT(dsv < resourceCount, "Resource ID outside available range!"); ZE_ASSERT(dsv != BACKBUFFER_RID, "Cannot use backbuffer as depth stencil!"); return dsvs[dsv - 1].Get(); }
		IUnorderedAccessView* GetUAV(RID uav) const noexcept { ZE_ASSERT(uav < resourceCount, "Resource ID outside available range!"); ZE_ASSERT(uav != BACKBUFFER_RID, "Cannot use backbuffer as unordered access!"); return uavs[uav - 1].Get(); }
	};

#pragma region Functions
	template<U8 RTVCount>
	void FrameBuffer::BeginRaster(GFX::CommandList& cl, const RID* rtv, bool adjacent) const noexcept
	{
		static_assert(RTVCount > 1, "For performance reasons FrameBuffer::BeginRaster() should be only used for multiple render targets!");
		static_assert(RTVCount <= Settings::MAX_RENDER_TARGETS, "Exceeding max number of concurrently bound render targets!");

		EnterRaster(cl);
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
		cl.Get().dx11.GetContext()->OMSetRenderTargets(RTVCount, handles, nullptr);
	}

	template<U8 RTVCount>
	void FrameBuffer::BeginRaster(GFX::CommandList& cl, const RID* rtv, RID dsv, bool adjacent) const noexcept
	{
		static_assert(RTVCount > 1, "For performance reasons FrameBuffer::BeginRaster() should be only used for multiple render targets!");
		static_assert(RTVCount <= Settings::MAX_RENDER_TARGETS, "Exceeding max number of concurrently bound render targets!");
		ZE_ASSERT(GetDSV(dsv), "Current resource is not suitable for being depth stencil!");

		EnterRaster(cl);
		ID3D11RenderTargetView* handles[RTVCount];
		D3D11_VIEWPORT vieports[RTVCount];
		for (U32 i = 0; i < RTVCount; ++i)
		{
			RID id = rtv[i];
			handles[i] = static_cast<ID3D11RenderTargetView*>(GetRTV(id));
			ZE_ASSERT(handles[i], "Current resource is not suitable for being render target!");
			SetupViewport(vieports[i], id);
		}
		cl.Get().dx11.GetContext()->RSSetViewports(RTVCount, vieports);
		cl.Get().dx11.GetContext()->OMSetRenderTargets(RTVCount, handles, GetDSV(dsv));
	}
#pragma endregion
}