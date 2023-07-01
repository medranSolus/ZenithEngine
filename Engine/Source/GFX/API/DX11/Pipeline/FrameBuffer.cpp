#include "GFX/API/DX11/Pipeline/FrameBuffer.h"

namespace ZE::GFX::API::DX11::Pipeline
{
	struct ResourceInfo
	{
		D3D11_TEXTURE2D_DESC1 Desc;
		DX::ComPtr<ITexture2D> Texture;
		std::bitset<6> Flags;

		constexpr bool IsRTV() const noexcept { return Flags[0]; }
		constexpr bool IsDSV() const noexcept { return Flags[1]; }
		constexpr bool IsSRV() const noexcept { return Flags[2]; }
		constexpr bool IsUAV() const noexcept { return Flags[3]; }
		constexpr bool IsCube() const noexcept { return Flags[4]; }
		constexpr bool UseStencilView() const noexcept { return Flags[5]; }
	};

	void FrameBuffer::SetupViewport(D3D11_VIEWPORT& viewport, RID rid) const noexcept
	{
		const UInt2 size = resources[rid].Size;
		viewport.Width = static_cast<float>(size.x);
		viewport.Height = static_cast<float>(size.y);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
	}

	void FrameBuffer::SetViewport(CommandList& cl, RID rid) const noexcept
	{
		D3D11_VIEWPORT viewport = {};
		SetupViewport(viewport, rid);
		cl.GetContext()->RSSetViewports(1, &viewport);
	}

	FrameBuffer::FrameBuffer(GFX::Device& dev, GFX::CommandList& mainList,
		const GFX::Pipeline::FrameBufferDesc& desc)
	{
		ZE_DX_ENABLE_ID(dev.Get().dx11);
		auto* device = dev.Get().dx11.GetDevice();

		resourceCount = desc.ResourceInfo.size();
		ZE_ASSERT(desc.ResourceInfo.size() <= UINT16_MAX, "Too much resources, needed wider type!");

		resources = new BufferData[resourceCount];
		resources[0].Resource = nullptr;
		resources[0].Size = { desc.ResourceInfo.front().Width, desc.ResourceInfo.front().Height };
		bool rtvMipsPresent = false;
		bool dsvMipsPresent = false;
		bool uavMipsPresent = false;

		// Create textures
		std::vector<ResourceInfo> resourcesInfo;
		resourcesInfo.reserve(resourceCount - 1);
		D3D11_TEXTURE2D_DESC1 texDesc = {};
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.CPUAccessFlags = 0;
		texDesc.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;
		for (RID i = 1; i < resourceCount; ++i)
		{
			const auto& res = desc.ResourceInfo.at(i);
			resources[i].Size = { res.Width, res.Height };

			texDesc.Width = res.Width;
			texDesc.Height = res.Height;
			texDesc.ArraySize = res.ArraySize;
			texDesc.MipLevels = res.MipLevels;
			texDesc.Format = DX::GetNonDepthDXFormat(res.Format);
			if (!texDesc.MipLevels)
				texDesc.MipLevels = 1;
			if (res.Flags & GFX::Pipeline::FrameResourceFlags::Cube)
			{
				texDesc.ArraySize *= 6;
				texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
			}
			else
				texDesc.MiscFlags = 0;

			texDesc.BindFlags = 0;
			bool isRT = false, isDS = false, isUA = false, isSR = false;
			if (res.Flags & GFX::Pipeline::FrameResourceFlags::ForceSRV)
			{
				isSR = true;
				texDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
			}
			if (res.Flags & GFX::Pipeline::FrameResourceFlags::ForceDSV)
			{
				texDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
				isDS = true;
				if (texDesc.MipLevels > 1)
					dsvMipsPresent = true;
			}

			for (const auto& state : desc.ResourceLifetimes.at(i))
			{
				switch (state.second)
				{
				case GFX::Resource::StateRenderTarget:
				{
					ZE_ASSERT(!isDS, "Cannot create depth stencil and render target view for same buffer!");
					ZE_ASSERT(!Utils::IsDepthStencilFormat(res.Format), "Cannot use depth stencil format with render target!");

					texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
					isRT = true;
					if (texDesc.MipLevels > 1)
						rtvMipsPresent = true;
					break;
				}
				case GFX::Resource::StateDepthRead:
				case GFX::Resource::StateDepthWrite:
				{
					ZE_ASSERT((res.Flags & GFX::Pipeline::FrameResourceFlags::SimultaneousAccess) == 0, "Simultaneous access cannot be used on depth stencil!");
					ZE_ASSERT(!isRT, "Cannot create depth stencil and render target view for same buffer!");
					ZE_ASSERT(!isUA, "Cannot create depth stencil and unordered access view for same buffer!");

					texDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
					isDS = true;
					if (texDesc.MipLevels > 1)
						dsvMipsPresent = true;
					break;
				}
				case GFX::Resource::StateUnorderedAccess:
				{
					ZE_ASSERT(!isDS, "Cannot create depth stencil and unordered access view for same buffer!");

					texDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
					isUA = true;
					if (texDesc.MipLevels > 1)
						uavMipsPresent = true;
					break;
				}
				case GFX::Resource::StateShaderResourcePS:
				case GFX::Resource::StateShaderResourceNonPS:
				case GFX::Resource::StateShaderResourceAll:
				{
					texDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
					isSR = true;
					break;
				}
				default:
					break;
				}
			}

			DX::ComPtr<ITexture2D> texture = nullptr;
			ZE_DX_THROW_FAILED(device->CreateTexture2D1(&texDesc, nullptr, &texture));
			ZE_DX_THROW_FAILED(texture.As(&resources[i].Resource));

			resourcesInfo.emplace_back(texDesc, std::move(texture),
				static_cast<U8>(isRT) | (isDS << 1) | (isSR << 2) | (isUA << 3)
				| ((res.Flags & GFX::Pipeline::FrameResourceFlags::Cube) << 4)
				| ((res.Flags & GFX::Pipeline::FrameResourceFlags::StencilView) << 5));
		}

		// Create view arrays
		rtvs = new DX::ComPtr<IRenderTargetView>[resourceCount];
		dsvs = new DX::ComPtr<IDepthStencilView>[resourceCount - 1];
		srvs = new DX::ComPtr<IShaderResourceView>[resourceCount];
		uavs = new DX::ComPtr<IUnorderedAccessView>[resourceCount - 1];
		if (rtvMipsPresent)
			rtvMips = new Ptr<DX::ComPtr<IRenderTargetView>>[resourceCount - 1];
		if (dsvMipsPresent)
			dsvMips = new Ptr<DX::ComPtr<IDepthStencilView>>[resourceCount - 1];
		if (uavMipsPresent)
			uavMips = new Ptr<DX::ComPtr<IUnorderedAccessView>>[resourceCount - 1];

		// Create views
		for (RID i = 1; auto& info : resourcesInfo)
		{
			if (info.IsRTV())
			{
				D3D11_RENDER_TARGET_VIEW_DESC1 rtvDesc = {};
				rtvDesc.Format = info.Desc.Format;
				if (info.Desc.ArraySize > 1)
				{
					rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
					rtvDesc.Texture2DArray.MipSlice = 0;
					rtvDesc.Texture2DArray.FirstArraySlice = 0;
					rtvDesc.Texture2DArray.ArraySize = info.Desc.ArraySize;
					rtvDesc.Texture2DArray.PlaneSlice = 0;
				}
				else
				{
					rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
					rtvDesc.Texture2D.MipSlice = 0;
					rtvDesc.Texture2D.PlaneSlice = 0;
				}
				ZE_DX_THROW_FAILED_INFO(device->CreateRenderTargetView1(info.Texture.Get(), &rtvDesc, &rtvs[i]));

				// Generate views for proper mips
				if (info.Desc.MipLevels > 1)
				{
					auto& targetResourceMip = rtvMips[i - 1];
					targetResourceMip = new DX::ComPtr<IRenderTargetView>[info.Desc.MipLevels];
					targetResourceMip[0] = rtvs[i];
					for (U16 j = 1; j < info.Desc.MipLevels; ++j)
					{
						if (info.Desc.ArraySize > 1)
							rtvDesc.Texture2DArray.MipSlice = j;
						else
							rtvDesc.Texture2D.MipSlice = j;

						ZE_DX_THROW_FAILED_INFO(device->CreateRenderTargetView1(info.Texture.Get(), &rtvDesc, &targetResourceMip[j]));
					}
				}
			}
			else if (info.IsDSV())
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
				dsvDesc.Format = DX::ConvertDepthFormatToDSV(info.Desc.Format);
				dsvDesc.Flags = 0;
				if (info.Desc.ArraySize > 1)
				{
					dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
					dsvDesc.Texture2DArray.MipSlice = 0;
					dsvDesc.Texture2DArray.FirstArraySlice = 0;
					dsvDesc.Texture2DArray.ArraySize = info.Desc.ArraySize;
				}
				else
				{
					dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
					dsvDesc.Texture2D.MipSlice = 0;
				}
				ZE_DX_THROW_FAILED(device->CreateDepthStencilView(info.Texture.Get(), &dsvDesc, &dsvs[i - 1]));

				// Generate views for proper mips
				if (info.Desc.MipLevels > 1)
				{
					auto& targetResourceMip = dsvMips[i - 1];
					targetResourceMip = new DX::ComPtr<IDepthStencilView>[info.Desc.MipLevels];
					targetResourceMip[0] = dsvs[i];
					for (U16 j = 1; j < info.Desc.MipLevels; ++j)
					{
						if (info.Desc.ArraySize > 1)
							dsvDesc.Texture2DArray.MipSlice = j;
						else
							dsvDesc.Texture2D.MipSlice = j;

						ZE_DX_THROW_FAILED(device->CreateDepthStencilView(info.Texture.Get(), &dsvDesc, &targetResourceMip[j]));
					}
				}
			}
			if (info.IsUAV())
			{
				D3D11_UNORDERED_ACCESS_VIEW_DESC1 uavDesc = {};
				uavDesc.Format = DX::ConvertDepthFormatToResourceView(info.Desc.Format, info.UseStencilView());
				if (info.Desc.ArraySize > 1)
				{
					uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
					uavDesc.Texture2DArray.MipSlice = 0;
					uavDesc.Texture2DArray.FirstArraySlice = 0;
					uavDesc.Texture2DArray.ArraySize = info.Desc.ArraySize;
					uavDesc.Texture2DArray.PlaneSlice = 0;
				}
				else
				{
					uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
					uavDesc.Texture2D.MipSlice = 0;
					uavDesc.Texture2D.PlaneSlice = 0;
				}
				ZE_DX_THROW_FAILED(device->CreateUnorderedAccessView1(info.Texture.Get(), &uavDesc, &uavs[i - 1]));

				// Generate views for proper mips
				if (info.Desc.MipLevels > 1)
				{
					auto& targetResourceMip = uavMips[i - 1];
					targetResourceMip = new DX::ComPtr<IUnorderedAccessView>[info.Desc.MipLevels];
					targetResourceMip[0] = uavs[i - 1];

					for (U16 j = 1; j < info.Desc.MipLevels; ++j)
					{
						if (info.Desc.ArraySize > 1)
							uavDesc.Texture2DArray.MipSlice = j;
						else
							uavDesc.Texture2D.MipSlice = j;

						ZE_DX_THROW_FAILED(device->CreateUnorderedAccessView1(info.Texture.Get(), &uavDesc, &targetResourceMip[j]));
					}
				}
			}
			if (info.IsSRV())
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC1 srvDesc = {};
				srvDesc.Format = DX::ConvertDepthFormatToResourceView(info.Desc.Format, info.UseStencilView());
				if (info.IsCube())
				{
					if (info.Desc.ArraySize > 6)
					{
						srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
						srvDesc.TextureCubeArray.MostDetailedMip = 0;
						srvDesc.TextureCubeArray.MipLevels = info.Desc.MipLevels;
						srvDesc.TextureCubeArray.First2DArrayFace = 0;
						srvDesc.TextureCubeArray.NumCubes = info.Desc.ArraySize / 6;
					}
					else
					{
						srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
						srvDesc.TextureCube.MostDetailedMip = 0;
						srvDesc.TextureCube.MipLevels = info.Desc.MipLevels;
					}
				}
				else if (info.Desc.ArraySize > 1)
				{
					srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
					srvDesc.Texture2DArray.MostDetailedMip = 0;
					srvDesc.Texture2DArray.MipLevels = info.Desc.MipLevels;
					srvDesc.Texture2DArray.FirstArraySlice = 0;
					srvDesc.Texture2DArray.ArraySize = info.Desc.ArraySize;
					srvDesc.Texture2DArray.PlaneSlice = 0;
				}
				else
				{
					srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
					srvDesc.Texture2D.MostDetailedMip = 0;
					srvDesc.Texture2D.MipLevels = info.Desc.MipLevels;
					srvDesc.Texture2D.PlaneSlice = 0;
				}
				ZE_DX_THROW_FAILED(device->CreateShaderResourceView1(info.Texture.Get(), &srvDesc, &srvs[i]));
			}
			++i;
		}
	}

	FrameBuffer::~FrameBuffer()
	{
		if (resources)
			resources.DeleteArray();
		if (rtvs)
			rtvs.DeleteArray();
		if (dsvs)
			dsvs.DeleteArray();
		if (srvs)
			srvs.DeleteArray();
		if (uavs)
			uavs.DeleteArray();
		--resourceCount;
		if (rtvMips)
		{
			for (U32 i = 0; i < resourceCount; ++i)
				if (rtvMips[i])
					rtvMips[i].DeleteArray();
			rtvMips.DeleteArray();
		}
		if (dsvMips)
		{
			for (U32 i = 0; i < resourceCount; ++i)
				if (dsvMips[i])
					dsvMips[i].DeleteArray();
			dsvMips.DeleteArray();
		}
		if (uavMips)
		{
			for (U32 i = 0; i < resourceCount; ++i)
				if (uavMips[i])
					uavMips[i].DeleteArray();
			uavMips.DeleteArray();
		}
	}

	void FrameBuffer::Copy(GFX::CommandList& cl, RID src, RID dest) const noexcept
	{
		ZE_ASSERT(src < resourceCount, "Source resource ID outside available range!");
		ZE_ASSERT(dest < resourceCount, "Destination resource ID outside available range!");
		ZE_ASSERT(GetDimmensions(src) == GetDimmensions(dest), "Resources must have same dimmensions for copy!");

		cl.Get().dx11.GetContext()->CopyResource(resources[dest].Resource.Get(), resources[src].Resource.Get());
	}

	void FrameBuffer::SetRTV(GFX::CommandList& cl, RID rid) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rtvs[rid], "Current resource is not suitable for being render target!");

		SetViewport(cl.Get().dx11, rid);
		cl.Get().dx11.GetContext()->OMSetRenderTargets(1, reinterpret_cast<ID3D11RenderTargetView* const*>(rtvs[rid].GetAddressOf()), nullptr);
	}

	void FrameBuffer::SetRTV(GFX::CommandList& cl, RID rid, U16 mipLevel) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rtvs[rid], "Current resource is not suitable for being render target!");
		ZE_ASSERT(rtvMips != nullptr, "Mips not supported as no resource has been created with mips greater than 1!");
		ZE_ASSERT(rtvMips[rid - 1] != nullptr, "Mips for current resource not supported!");

		SetViewport(cl.Get().dx11, rid);
		cl.Get().dx11.GetContext()->OMSetRenderTargets(1, reinterpret_cast<ID3D11RenderTargetView* const*>(rtvMips[rid - 1][mipLevel].GetAddressOf()), nullptr);
	}

	void FrameBuffer::SetDSV(GFX::CommandList& cl, RID rid) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(dsvs[rid - 1], "Current resource is not suitable for being depth stencil!");

		SetViewport(cl.Get().dx11, rid);
		cl.Get().dx11.GetContext()->OMSetRenderTargets(0, nullptr, dsvs[rid - 1].Get());
	}

	void FrameBuffer::SetDSV(GFX::CommandList& cl, RID rid, U16 mipLevel) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(dsvs[rid - 1], "Current resource is not suitable for being depth stencil!");
		ZE_ASSERT(dsvMips != nullptr, "Mips not supported as no resource has been created with mips greater than 1!");
		ZE_ASSERT(dsvMips[rid - 1] != nullptr, "Mips for current resource not supported!");

		SetViewport(cl.Get().dx11, rid);
		cl.Get().dx11.GetContext()->OMSetRenderTargets(0, nullptr, dsvMips[rid - 1][mipLevel].Get());
	}

	void FrameBuffer::SetOutput(GFX::CommandList& cl, RID rtv, RID dsv) const noexcept
	{
		ZE_ASSERT(rtv < resourceCount, "RTV resource ID outside available range!");
		ZE_ASSERT(dsv < resourceCount, "DSV resource ID outside available range!");
		ZE_ASSERT(dsv != 0, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(rtvs[rtv], "Current resource is not suitable for being render target!");
		ZE_ASSERT(dsvs[dsv - 1], "Current resource is not suitable for being depth stencil!");

		SetViewport(cl.Get().dx11, rtv);
		cl.Get().dx11.GetContext()->OMSetRenderTargets(1, reinterpret_cast<ID3D11RenderTargetView* const*>(rtvs[rtv].GetAddressOf()), dsvs[dsv - 1].Get());
	}

	void FrameBuffer::SetSRV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(srvs[rid], "Current resource is not suitable for being shader resource!");

		auto& schema = bindCtx.BindingSchema.Get().dx11;
		auto slotInfo = schema.GetCurrentSlot(bindCtx.Count++);

		auto* ctx = cl.Get().dx11.GetContext();
		for (U32 i = 0; i < slotInfo.SlotsCount; ++i)
		{
			auto slotData = schema.GetSlotData(slotInfo.DataStart + i);
			currentSlots.emplace_back(true, slotData);
			for (U32 j = 0; j < slotData.Count; ++j, ++slotData.BindStart, ++rid)
			{
				if (slotData.Shaders & GFX::Resource::ShaderType::Compute)
					ctx->CSSetShaderResources(slotData.BindStart, 1, reinterpret_cast<ID3D11ShaderResourceView* const*>(srvs[rid].GetAddressOf()));
				else
				{
					if (slotData.Shaders & GFX::Resource::ShaderType::Vertex)
						ctx->VSSetShaderResources(slotData.BindStart, 1, reinterpret_cast<ID3D11ShaderResourceView* const*>(srvs[rid].GetAddressOf()));
					if (slotData.Shaders & GFX::Resource::ShaderType::Domain)
						ctx->DSSetShaderResources(slotData.BindStart, 1, reinterpret_cast<ID3D11ShaderResourceView* const*>(srvs[rid].GetAddressOf()));
					if (slotData.Shaders & GFX::Resource::ShaderType::Hull)
						ctx->HSSetShaderResources(slotData.BindStart, 1, reinterpret_cast<ID3D11ShaderResourceView* const*>(srvs[rid].GetAddressOf()));
					if (slotData.Shaders & GFX::Resource::ShaderType::Geometry)
						ctx->GSSetShaderResources(slotData.BindStart, 1, reinterpret_cast<ID3D11ShaderResourceView* const*>(srvs[rid].GetAddressOf()));
					if (slotData.Shaders & GFX::Resource::ShaderType::Pixel)
						ctx->PSSetShaderResources(slotData.BindStart, 1, reinterpret_cast<ID3D11ShaderResourceView* const*>(srvs[rid].GetAddressOf()));
				}
			}
		}
	}

	void FrameBuffer::SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(uavs[rid - 1], "Current resource is not suitable for being unnordered access!");

		auto& schema = bindCtx.BindingSchema.Get().dx11;
		auto slotInfo = schema.GetCurrentSlot(bindCtx.Count++);

		--rid;
		auto* ctx = cl.Get().dx11.GetContext();
		for (U32 i = 0; i < slotInfo.SlotsCount; ++i)
		{
			auto slotData = schema.GetSlotData(slotInfo.DataStart + i);
			currentSlots.emplace_back(false, slotData);
			for (U32 j = 0; j < slotData.Count; ++j, ++slotData.BindStart)
			{
				if (slotData.Shaders & GFX::Resource::ShaderType::Compute)
					ctx->CSSetUnorderedAccessViews(slotData.BindStart, 1, reinterpret_cast<ID3D11UnorderedAccessView* const*>(uavs[rid++].GetAddressOf()), nullptr);
				else
				{
					ZE_FAIL("Cannot use UAV outside compute shader!");
				}
			}
		}
	}

	void FrameBuffer::SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID rid, U16 mipLevel) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(uavs[rid - 1], "Current resource is not suitable for being unnordered access!");
		ZE_ASSERT(uavMips != nullptr, "Mips not supported as no resource has been created with mips greater than 1!");
		ZE_ASSERT(uavMips[rid - 1] != nullptr, "Mips for current resource not supported!");

		auto& schema = bindCtx.BindingSchema.Get().dx11;
		auto slotInfo = schema.GetCurrentSlot(bindCtx.Count++);

		--rid;
		auto* ctx = cl.Get().dx11.GetContext();
		for (U32 i = 0; i < slotInfo.SlotsCount; ++i)
		{
			auto slotData = schema.GetSlotData(slotInfo.DataStart + i);
			currentSlots.emplace_back(false, slotData);
			for (U32 j = 0; j < slotData.Count; ++j, ++slotData.BindStart)
			{
				if (slotData.Shaders & GFX::Resource::ShaderType::Compute)
					ctx->CSSetUnorderedAccessViews(slotData.BindStart, 1, reinterpret_cast<ID3D11UnorderedAccessView* const*>(uavMips[rid][mipLevel++].GetAddressOf()), nullptr);
				else
				{
					ZE_FAIL("Cannot use UAV outside compute shader!");
				}
			}
		}
	}

	void FrameBuffer::BarrierTransition(GFX::CommandList& cl, RID rid, GFX::Resource::State before, GFX::Resource::State after) const noexcept
	{
		auto* ctx = cl.Get().dx11.GetContext();
		if (before == GFX::Resource::StateRenderTarget
			|| before == GFX::Resource::StateDepthRead
			|| before == GFX::Resource::StateDepthWrite)
			ctx->OMSetRenderTargets(0, nullptr, nullptr);
		else if (before == GFX::Resource::StateUnorderedAccess)
		{
			ID3D11UnorderedAccessView* nullUav[D3D11_PS_CS_UAV_REGISTER_COUNT] = { nullptr };
			for (auto it = currentSlots.begin(); it != currentSlots.end();)
			{
				if (!it->first)
				{
					ZE_ASSERT(it->second.BindStart + it->second.Count < D3D11_PS_CS_UAV_REGISTER_COUNT, "Too wide binding range!");
					ctx->CSSetUnorderedAccessViews(it->second.BindStart, it->second.Count, nullUav, nullptr);
					it = currentSlots.erase(it);
				}
				else
					++it;
			}
		}
		else
		{
			ID3D11ShaderResourceView* nullSrv[D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT] = { nullptr };
			for (auto it = currentSlots.begin(); it != currentSlots.end();)
			{
				if (it->first)
				{
					ZE_ASSERT(it->second.BindStart + it->second.Count < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT, "Too wide binding range!");

					if (it->second.Shaders & GFX::Resource::ShaderType::Vertex)
						ctx->VSSetShaderResources(it->second.BindStart, it->second.Count, nullSrv);
					if (it->second.Shaders & GFX::Resource::ShaderType::Domain)
						ctx->DSSetShaderResources(it->second.BindStart, it->second.Count, nullSrv);
					if (it->second.Shaders & GFX::Resource::ShaderType::Hull)
						ctx->HSSetShaderResources(it->second.BindStart, it->second.Count, nullSrv);
					if (it->second.Shaders & GFX::Resource::ShaderType::Geometry)
						ctx->GSSetShaderResources(it->second.BindStart, it->second.Count, nullSrv);
					if (it->second.Shaders & GFX::Resource::ShaderType::Pixel)
						ctx->PSSetShaderResources(it->second.BindStart, it->second.Count, nullSrv);
					it = currentSlots.erase(it);
				}
				else
					++it;
			}
		}
	}

	void FrameBuffer::ClearRTV(GFX::CommandList& cl, RID rid, const ColorF4& color) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rtvs[rid], "Current resource is not suitable for being render target!");

		cl.Get().dx11.GetContext()->ClearRenderTargetView(rtvs[rid].Get(), reinterpret_cast<const float*>(&color));
	}

	void FrameBuffer::ClearDSV(GFX::CommandList& cl, RID rid, float depth, U8 stencil) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(dsvs[rid - 1], "Current resource is not suitable for being depth stencil!");

		cl.Get().dx11.GetContext()->ClearDepthStencilView(dsvs[rid - 1].Get(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
	}

	void FrameBuffer::ClearUAV(GFX::CommandList& cl, RID rid, const ColorF4& color) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(uavs[rid - 1], "Current resource is not suitable for being unnordered access!");

		cl.Get().dx11.GetContext()->ClearUnorderedAccessViewFloat(uavs[rid - 1].Get(),
			reinterpret_cast<const float*>(&color));
	}

	void FrameBuffer::ClearUAV(GFX::CommandList& cl, RID rid, const Pixel colors[4]) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != 0, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(uavs[rid - 1], "Current resource is not suitable for being unnordered access!");

		cl.Get().dx11.GetContext()->ClearUnorderedAccessViewUint(uavs[rid - 1].Get(),
			reinterpret_cast<const U32*>(colors));
	}

	void FrameBuffer::SwapBackbuffer(GFX::Device& dev, GFX::SwapChain& swapChain) noexcept
	{
		if (resources[0].Resource == nullptr)
		{
			resources[0].Resource = swapChain.Get().dx11.GetBuffer();
			rtvs[0] = swapChain.Get().dx11.GetRTV();
			srvs[0] = swapChain.Get().dx11.GetSRV();
		}
	}

	void FrameBuffer::ExitTransitions(GFX::Device& dev, GFX::CommandList& cl, U64 level) const noexcept
	{
		auto* ctx = cl.Get().dx11.GetContext();
		ctx->OMSetRenderTargets(0, nullptr, nullptr);

		ID3D11ShaderResourceView* nullSrv[D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT] = { nullptr };
		ID3D11UnorderedAccessView* nullUav[D3D11_PS_CS_UAV_REGISTER_COUNT] = { nullptr };

		for (const auto& slot : currentSlots)
		{
			if (slot.first)
			{
				ZE_ASSERT(slot.second.BindStart + slot.second.Count < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT, "Too wide binding range!");

				if (slot.second.Shaders & GFX::Resource::ShaderType::Vertex)
					ctx->VSSetShaderResources(slot.second.BindStart, slot.second.Count, nullSrv);
				if (slot.second.Shaders & GFX::Resource::ShaderType::Domain)
					ctx->DSSetShaderResources(slot.second.BindStart, slot.second.Count, nullSrv);
				if (slot.second.Shaders & GFX::Resource::ShaderType::Hull)
					ctx->HSSetShaderResources(slot.second.BindStart, slot.second.Count, nullSrv);
				if (slot.second.Shaders & GFX::Resource::ShaderType::Geometry)
					ctx->GSSetShaderResources(slot.second.BindStart, slot.second.Count, nullSrv);
				if (slot.second.Shaders & GFX::Resource::ShaderType::Pixel)
					ctx->PSSetShaderResources(slot.second.BindStart, slot.second.Count, nullSrv);
			}
			else
			{
				ZE_ASSERT(slot.second.BindStart + slot.second.Count < D3D11_PS_CS_UAV_REGISTER_COUNT, "Too wide binding range!");
				ctx->CSSetUnorderedAccessViews(slot.second.BindStart, slot.second.Count, nullUav, nullptr);
			}
		}
		currentSlots.clear();
	}
}