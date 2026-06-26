#include "RHI/DX11/Pipeline/FrameBuffer.h"

namespace ZE::RHI::DX11::Pipeline
{
	void FrameBuffer::SetupViewport(D3D11_VIEWPORT& viewport, RID rid) const noexcept
	{
		const UInt2 size = resources[rid].Size;
		viewport.Width = Utils::SafeCast<float>(size.X);
		viewport.Height = Utils::SafeCast<float>(size.Y);
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

	Expected<FrameBuffer> FrameBuffer::Create(GFX::Device& dev, const GFX::Pipeline::FrameBufferDesc& desc) noexcept
	{
		ZE_ASSERT(desc.Resources.size() > 0, "Empty FrameBuffer!");
		ZE_ASSERT(desc.Resources.size() == desc.ResourceLifetimes.size(), "Not every resource have it's associated lifetime!");
		ZE_ASSERT(desc.PassLevelCount > 0, "At least single pass level is required for passes to execute!");
		ZE_ASSERT(desc.Resources.size() < INVALID_RID, "Too much resources, needed wider type!");

		IDevice* device = dev.Get().dx11.GetDevice();

		FrameBuffer framebuffer;
		framebuffer.resourceCount = Utils::SafeCast<RID>(desc.Resources.size());

		framebuffer.resources = std::make_unique<BufferData[]>(framebuffer.resourceCount);
		framebuffer.resources[BACKBUFFER_RID].Resource = nullptr;
		framebuffer.resources[BACKBUFFER_RID].Size = desc.Resources.at(BACKBUFFER_RID).Sizes;
		framebuffer.resources[BACKBUFFER_RID].Array = desc.Resources.at(BACKBUFFER_RID).DepthOrArraySize;
		framebuffer.resources[BACKBUFFER_RID].Mips = desc.Resources.at(BACKBUFFER_RID).MipLevels;
		framebuffer.resources[BACKBUFFER_RID].Format = desc.Resources.at(BACKBUFFER_RID).Format;

		framebuffer.rtvs = std::make_unique<DX::ComPtr<IRenderTargetView>[]>(framebuffer.resourceCount);
		framebuffer.dsvs = std::make_unique<DX::ComPtr<IDepthStencilView>[]>(framebuffer.resourceCount - 1);
		framebuffer.srvs = std::make_unique<DX::ComPtr<IShaderResourceView>[]>(framebuffer.resourceCount);
		framebuffer.uavs = std::make_unique<DX::ComPtr<IUnorderedAccessView>[]>(framebuffer.resourceCount - 1);

		// Create resources and their views
		for (RID i = 1; i < framebuffer.resourceCount; ++i)
		{
			const auto& resDesc = desc.Resources.at(i);
			ZE_ASSERT_WARN(resDesc.Flags & GFX::Pipeline::FrameResourceFlag::InternalResourceActive, "Resource don't contain active flag! Redundant memory will be allocated on CPU.");
			if (resDesc.Flags & GFX::Pipeline::FrameResourceFlag::InternalResourceActive)
			{
				auto& dataDesc = framebuffer.resources[i];
				switch (resDesc.Type)
				{
				case GFX::Pipeline::FrameResourceType::Buffer:
					dataDesc.SetBuffer();
					break;
				case GFX::Pipeline::FrameResourceType::Texture1D:
					dataDesc.SetTex1D();
					break;
				default:
					ZE_ENUM_UNHANDLED();
				case GFX::Pipeline::FrameResourceType::Texture2D:
					break;
				case GFX::Pipeline::FrameResourceType::TextureCube:
					dataDesc.SetCube();
					break;
				case GFX::Pipeline::FrameResourceType::Texture3D:
					dataDesc.SetTex3D();
					break;
				}

				if (resDesc.Flags & GFX::Pipeline::FrameResourceFlag::NoResourceCreation)
				{
					ZE_FAIL("Memory only resources are not supported in DX11 backend due to simplified memory management!");
					return std::unexpected(ZE_DX_ERROR(DX::Error::NO_MEMORY_ONLY_RES));
				}
				else if (resDesc.Flags & GFX::Pipeline::FrameResourceFlag::OutsideResource)
				{
					dataDesc.Size = { 0, 0 };
					dataDesc.Array = 0;
					dataDesc.Mips = 0;
					dataDesc.Format = resDesc.Format;
					dataDesc.SetOutsideResource();
				}
				else
				{
					dataDesc.Size = resDesc.GetResolutionAdjustedSizes();
					dataDesc.Array = resDesc.DepthOrArraySize;
					dataDesc.Mips = resDesc.MipLevels ? resDesc.MipLevels : Math::GetMipLevels(dataDesc.Size.X, dataDesc.Size.Y);
					dataDesc.Format = resDesc.Format;

					// Get correct usage flags
					const bool isRT = resDesc.Flags & (GFX::Pipeline::FrameResourceFlag::ForceRTV | GFX::Pipeline::FrameResourceFlag::InternalUsageRenderTarget);
					const bool isDS = resDesc.Flags & (GFX::Pipeline::FrameResourceFlag::ForceDSV | GFX::Pipeline::FrameResourceFlag::InternalUsageDepth);
					const bool isUA = resDesc.Flags & (GFX::Pipeline::FrameResourceFlag::ForceUAV | GFX::Pipeline::FrameResourceFlag::InternalUsageUnorderedAccess);
					const bool isSR = resDesc.Flags & (GFX::Pipeline::FrameResourceFlag::ForceSRV | GFX::Pipeline::FrameResourceFlag::InternalUsageShaderResource);
					
					U32 bindFlags = 0;
					if (isRT)
					{
						ZE_ASSERT(!isDS, "Cannot create depth stencil and render target view for same buffer!");
						ZE_ASSERT(!Utils::IsDepthStencilFormat(dataDesc.Format), "Cannot use depth stencil format with render target!");

						bindFlags |= D3D11_BIND_RENDER_TARGET;
					}
					if (isDS)
					{
						ZE_ASSERT(!isRT, "Cannot create depth stencil and render target view for same buffer!");
						ZE_ASSERT(!isUA, "Cannot create depth stencil and unordered access view for same buffer!");

						bindFlags |= D3D11_BIND_DEPTH_STENCIL;
					}
					if (isUA)
					{
						ZE_ASSERT(!isDS, "Cannot create depth stencil and unordered access view for same buffer!");

						bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
					}
					if (isSR)
						bindFlags |= D3D11_BIND_SHADER_RESOURCE;

					// Create proper resource type
					switch (resDesc.Type)
					{
					case GFX::Pipeline::FrameResourceType::Buffer:
					{
						ZE_ASSERT(!isRT, "Cannot create render target view for buffer resource!");
						ZE_ASSERT(!isDS, "Cannot create buffer resource as depth stencil!");

						D3D11_BUFFER_DESC buffDesc = {};
						buffDesc.ByteWidth = dataDesc.Size.X;
						buffDesc.Usage = D3D11_USAGE_DEFAULT;
						buffDesc.BindFlags = bindFlags;
						buffDesc.CPUAccessFlags = 0;
						buffDesc.MiscFlags = 0;
						buffDesc.StructureByteStride = dataDesc.Size.Y;

						if (resDesc.Flags & GFX::Pipeline::FrameResourceFlag::RawBufferView)
							buffDesc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
						if (buffDesc.StructureByteStride != 0)
							buffDesc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

						DX::ComPtr<IBuffer> buffer;
						ZE_DX_RET_FAILED_EXPECT(device->CreateBuffer(&buffDesc, nullptr, &buffer));
						ZE_DX_RET_FAILED_EXPECT(buffer.As(&dataDesc.Resource));
						break;
					}
					case GFX::Pipeline::FrameResourceType::Texture1D:
					{
						ZE_ASSERT(dataDesc.Size.Y == 1, "Height of the 1D texture must be 1!");

						D3D11_TEXTURE1D_DESC texDesc = {};
						texDesc.Width = dataDesc.Size.X;
						texDesc.MipLevels = dataDesc.Mips;
						texDesc.ArraySize = dataDesc.Array;
						texDesc.Format = DX::GetDXFormat(dataDesc.Format);
						texDesc.Usage = D3D11_USAGE_DEFAULT;
						texDesc.BindFlags = bindFlags;
						texDesc.CPUAccessFlags = 0;
						texDesc.MiscFlags = 0;

						if (dataDesc.Array > 1 || resDesc.Flags & GFX::Pipeline::FrameResourceFlag::ArrayView)
							dataDesc.SetArrayView();

						DX::ComPtr<ITexture1D> texture;
						ZE_DX_RET_FAILED_EXPECT(device->CreateTexture1D(&texDesc, nullptr, &texture));
						ZE_DX_RET_FAILED_EXPECT(texture.As(&dataDesc.Resource));
						break;
					}
					default:
						ZE_ENUM_UNHANDLED();
					case GFX::Pipeline::FrameResourceType::Texture2D:
					case GFX::Pipeline::FrameResourceType::TextureCube:
					{
						D3D11_TEXTURE2D_DESC1 texDesc = {};
						texDesc.Width = dataDesc.Size.X;
						texDesc.Height = dataDesc.Size.Y;
						texDesc.MipLevels = dataDesc.Mips;
						texDesc.ArraySize = dataDesc.Array;
						texDesc.Format = DX::GetTypedDepthDXFormat(dataDesc.Format);
						texDesc.SampleDesc.Count = 1;
						texDesc.SampleDesc.Quality = 0;
						texDesc.Usage = D3D11_USAGE_DEFAULT;
						texDesc.BindFlags = bindFlags;
						texDesc.CPUAccessFlags = 0;
						texDesc.MiscFlags = 0;
						texDesc.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;

						if (dataDesc.IsCube())
						{
							texDesc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
							texDesc.ArraySize *= 6;
						}
						if (dataDesc.Array > 1 || resDesc.Flags & GFX::Pipeline::FrameResourceFlag::ArrayView)
							dataDesc.SetArrayView();

						DX::ComPtr<ITexture2D> texture;
						ZE_DX_RET_FAILED_EXPECT(device->CreateTexture2D1(&texDesc, nullptr, &texture));
						ZE_DX_RET_FAILED_EXPECT(texture.As(&dataDesc.Resource));
						break;
					}
					case GFX::Pipeline::FrameResourceType::Texture3D:
					{
						ZE_ASSERT(!isDS, "Cannot create 3D texture as depth stencil!");

						D3D11_TEXTURE3D_DESC1 texDesc = {};
						texDesc.Width = dataDesc.Size.X;
						texDesc.Height = dataDesc.Size.Y;
						texDesc.Depth = dataDesc.Array;
						texDesc.MipLevels = dataDesc.Mips;
						texDesc.Format = DX::GetDXFormat(dataDesc.Format);
						texDesc.Usage = D3D11_USAGE_DEFAULT;
						texDesc.BindFlags = bindFlags;
						texDesc.CPUAccessFlags = 0;
						texDesc.MiscFlags = 0;

						DX::ComPtr<ITexture3D> texture;
						ZE_DX_RET_FAILED_EXPECT(device->CreateTexture3D1(&texDesc, nullptr, &texture));
						ZE_DX_RET_FAILED_EXPECT(texture.As(&dataDesc.Resource));
						break;
					}
					}

					if (resDesc.Flags & GFX::Pipeline::FrameResourceFlag::Temporal)
					{

					}

					if (isRT)
					{
						D3D11_RENDER_TARGET_VIEW_DESC1 rtvDesc = {};
						rtvDesc.Format = DX::GetDXFormat(dataDesc.Format);

						switch (resDesc.Type)
						{
						case GFX::Pipeline::FrameResourceType::Buffer:
						{
							rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_BUFFER;
							rtvDesc.Buffer.FirstElement = 0;
							rtvDesc.Buffer.NumElements = dataDesc.Size.X;
							break;
						}
						case GFX::Pipeline::FrameResourceType::Texture1D:
						{
							if (dataDesc.IsArrayView())
							{
								rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
								rtvDesc.Texture1DArray.MipSlice = 0;
								rtvDesc.Texture1DArray.FirstArraySlice = 0;
								rtvDesc.Texture1DArray.ArraySize = dataDesc.Array;
							}
							else
							{
								rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
								rtvDesc.Texture1D.MipSlice = 0;
							}
							break;
						}
						default:
							ZE_ENUM_UNHANDLED();
						case GFX::Pipeline::FrameResourceType::Texture2D:
						{
							if (dataDesc.IsArrayView())
							{
								rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
								rtvDesc.Texture2DArray.MipSlice = 0;
								rtvDesc.Texture2DArray.FirstArraySlice = 0;
								rtvDesc.Texture2DArray.ArraySize = dataDesc.Array;
								rtvDesc.Texture2DArray.PlaneSlice = 0;
							}
							else
							{
								rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
								rtvDesc.Texture2D.MipSlice = 0;
								rtvDesc.Texture2D.PlaneSlice = 0;
							}
							break;
						}
						case GFX::Pipeline::FrameResourceType::TextureCube:
						{
							rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
							rtvDesc.Texture2DArray.MipSlice = 0;
							rtvDesc.Texture2DArray.FirstArraySlice = 0;
							rtvDesc.Texture2DArray.ArraySize = dataDesc.Array * 6;
							rtvDesc.Texture2DArray.PlaneSlice = 0;
							break;
						}
						case GFX::Pipeline::FrameResourceType::Texture3D:
						{
							rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
							rtvDesc.Texture3D.MipSlice = 0;
							rtvDesc.Texture3D.FirstWSlice = 0;
							rtvDesc.Texture3D.WSize = dataDesc.Array;
							break;
						}
						}
						ZE_DX_RET_FAILED_EXPECT(device->CreateRenderTargetView1(dataDesc.Resource.Get(), &rtvDesc, &framebuffer.rtvs[i]));

						// Generate views for proper mips
						if (dataDesc.Mips > 1)
						{
							if (!framebuffer.rtvMips)
								framebuffer.rtvMips = std::make_unique<std::unique_ptr<DX::ComPtr<IRenderTargetView>[]>[]>(framebuffer.resourceCount - 1);

							auto& targetResourceMip = framebuffer.rtvMips[i - 1];
							targetResourceMip = std::make_unique_for_overwrite<DX::ComPtr<IRenderTargetView>[]>(dataDesc.Mips);
							targetResourceMip[0] = framebuffer.rtvs[i];
							for (U16 j = 1; j < dataDesc.Mips; ++j)
							{
								switch (resDesc.Type)
								{
								case GFX::Pipeline::FrameResourceType::Buffer:
								{
									ZE_FAIL("No mip levels for buffer resources!");
									break;
								}
								case GFX::Pipeline::FrameResourceType::Texture1D:
								{
									if (dataDesc.IsArrayView())
										rtvDesc.Texture1DArray.MipSlice = j;
									else
										rtvDesc.Texture1D.MipSlice = j;
									break;
								}
								default:
									ZE_ENUM_UNHANDLED();
								case GFX::Pipeline::FrameResourceType::Texture2D:
								{
									if (dataDesc.IsArrayView())
										rtvDesc.Texture2DArray.MipSlice = j;
									else
										rtvDesc.Texture2D.MipSlice = j;
									break;
								}
								case GFX::Pipeline::FrameResourceType::TextureCube:
								{
									rtvDesc.Texture2DArray.MipSlice = j;
									break;
								}
								case GFX::Pipeline::FrameResourceType::Texture3D:
								{
									rtvDesc.Texture3D.MipSlice = j;
									break;
								}
								}
								ZE_DX_RET_FAILED_EXPECT(device->CreateRenderTargetView1(dataDesc.Resource.Get(), &rtvDesc, &targetResourceMip[j]));
							}
						}
					}
					else if (isDS)
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
							framebuffer.resources[i].SetArrayView();
						}
						else
						{
							dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
							dsvDesc.Texture2D.MipSlice = 0;
						}
						ZE_DX_THROW_FAILED(device->CreateDepthStencilView(info.Texture.Get(), &dsvDesc, &framebuffer.dsvs[i - 1]));

						// Generate views for proper mips
						if (dataDesc.Mips > 1)
						{
							if (!framebuffer.dsvMips)
								framebuffer.dsvMips = std::make_unique<std::unique_ptr<DX::ComPtr<IDepthStencilView>[]>[]>(framebuffer.resourceCount - 1);

							auto& targetResourceMip = framebuffer.dsvMips[i - 1];
							targetResourceMip = std::make_unique_for_overwrite<DX::ComPtr<IDepthStencilView>[]>(dataDesc.Mips);
							targetResourceMip[0] = framebuffer.dsvs[i - 1];
							for (U16 j = 1; j < dataDesc.Mips; ++j)
							{
								if (dataDesc.ArraySize > 1)
									dsvDesc.Texture2DArray.MipSlice = j;
								else
									dsvDesc.Texture2D.MipSlice = j;

								ZE_DX_THROW_FAILED(device->CreateDepthStencilView(info.Texture.Get(), &dsvDesc, &targetResourceMip[j]));
							}
						}
					}
					if (isUA)
					{

					}
					if (isSR)
					{
						DXGI_FORMAT format = DX::ConvertDepthFormatToResourceView(DX::GetTypedDepthDXFormat(dataDesc.Format), resDesc.Flags & GFX::Pipeline::FrameResourceFlag::StencilView);

					}



					

				}
			}
		}

		// Create view arrays
		if (dsvMipsPresent)
		if (uavMipsPresent)
			framebuffer.uavMips = std::make_unique<std::unique_ptr<DX::ComPtr<IUnorderedAccessView>[]>[]>(framebuffer.resourceCount - 1);
		
		// Create views
		for (RID i = 1; const auto& info : resourcesInfo)
		{
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
					framebuffer.resources[i].SetArrayView();
				}
				else
				{
					uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
					uavDesc.Texture2D.MipSlice = 0;
					uavDesc.Texture2D.PlaneSlice = 0;
				}
				ZE_DX_THROW_FAILED(device->CreateUnorderedAccessView1(info.Texture.Get(), &uavDesc, &framebuffer.uavs[i - 1]));

				// Generate views for proper mips
				if (info.Desc.MipLevels > 1)
				{
					auto& targetResourceMip = framebuffer.uavMips[i - 1];
					targetResourceMip = new DX::ComPtr<IUnorderedAccessView>[info.Desc.MipLevels];
					targetResourceMip[0] = framebuffer.uavs[i - 1];

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
						framebuffer.resources[i].SetArrayView();
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
					framebuffer.resources[i].SetArrayView();
				}
				else
				{
					srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
					srvDesc.Texture2D.MostDetailedMip = 0;
					srvDesc.Texture2D.MipLevels = info.Desc.MipLevels;
					srvDesc.Texture2D.PlaneSlice = 0;
				}
				ZE_DX_THROW_FAILED(device->CreateShaderResourceView1(info.Texture.Get(), &srvDesc, &framebuffer.srvs[i]));
			}
			++i;
		}
	}

	void FrameBuffer::BeginRasterSparse(GFX::CommandList& cl, const RID* rtv, U8 count) const noexcept
	{
		ZE_ASSERT(count <= Settings::MAX_RENDER_TARGETS, "Too many render targets!");

		ID3D11RenderTargetView* handles[Settings::MAX_RENDER_TARGETS];
		D3D11_VIEWPORT vieports[Settings::MAX_RENDER_TARGETS];
		U8 realCount = 0;
		for (U32 i = 0; i < count; ++i)
		{
			RID id = rtv[i];
			if (id != INVALID_RID)
			{
				ZE_ASSERT(id < resourceCount, "Resource ID outside available range!");

				handles[realCount] = static_cast<ID3D11RenderTargetView*>(GetRTV(id));
				ZE_ASSERT(handles[realCount], "Current resource is not suitable for being render target!");

				SetupViewport(vieports[realCount], id);
				++realCount;
			}
		}
		cl.Get().dx11.GetContext()->RSSetViewports(realCount, vieports);
		cl.Get().dx11.GetContext()->OMSetRenderTargets(realCount, handles, nullptr);
	}

	void FrameBuffer::BeginRasterSparse(GFX::CommandList& cl, const RID* rtv, RID dsv, U8 count) const noexcept
	{
		ZE_ASSERT(count <= Settings::MAX_RENDER_TARGETS, "Too many render targets!");
		ZE_ASSERT(dsv != BACKBUFFER_RID, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(GetDSV(dsv), "Current resource is not suitable for being depth stencil!");

		ID3D11RenderTargetView* handles[Settings::MAX_RENDER_TARGETS];
		D3D11_VIEWPORT vieports[Settings::MAX_RENDER_TARGETS];
		U8 realCount = 0;
		for (U32 i = 0; i < count; ++i)
		{
			RID id = rtv[i];
			if (id != INVALID_RID)
			{
				ZE_ASSERT(id < resourceCount, "Resource ID outside available range!");

				handles[realCount] = static_cast<ID3D11RenderTargetView*>(GetRTV(id));
				ZE_ASSERT(handles[realCount], "Current resource is not suitable for being render target!");

				SetupViewport(vieports[realCount], id);
				++realCount;
			}
		}
		cl.Get().dx11.GetContext()->RSSetViewports(realCount, vieports);
		cl.Get().dx11.GetContext()->OMSetRenderTargets(realCount, handles, GetDSV(dsv));
	}

	void FrameBuffer::BeginRasterDepthOnly(GFX::CommandList& cl, RID dsv) const noexcept
	{
		ZE_ASSERT(GetDSV(dsv), "Current resource is not suitable for being depth stencil!");

		SetViewport(cl.Get().dx11, dsv);
		cl.Get().dx11.GetContext()->OMSetRenderTargets(0, nullptr, GetDSV(dsv));
	}

	void FrameBuffer::BeginRaster(GFX::CommandList& cl, RID rtv, RID dsv) const noexcept
	{
		ZE_ASSERT(GetRTV(rtv), "Current resource is not suitable for being render target!");
		ZE_ASSERT(GetDSV(dsv), "Current resource is not suitable for being depth stencil!");

		SetViewport(cl.Get().dx11, rtv);
		auto* view = reinterpret_cast<ID3D11RenderTargetView*>(GetRTV(rtv));
		cl.Get().dx11.GetContext()->OMSetRenderTargets(1, &view, GetDSV(dsv));
	}

	void FrameBuffer::BeginRasterDepthOnly(GFX::CommandList& cl, RID dsv, U16 mipLevel) const noexcept
	{
		ZE_ASSERT(GetDSV(dsv), "Current resource is not suitable for being depth stencil!");
		ZE_ASSERT(dsvMips != nullptr, "Mips not supported as no resource has been created with mips greater than 1!");
		ZE_ASSERT(dsvMips[dsv - 1] != nullptr, "Mips for current resource not supported!");
		ZE_ASSERT(mipLevel < GetMipCount(dsv), "Mip level outside available range!");

		SetViewport(cl.Get().dx11, dsv);
		cl.Get().dx11.GetContext()->OMSetRenderTargets(0, nullptr, dsvMips[dsv - 1][mipLevel].Get());
	}

	void FrameBuffer::BeginRaster(GFX::CommandList& cl, RID rtv, RID dsv, U16 mipLevel) const noexcept
	{
		ZE_ASSERT(GetRTV(rtv), "Current resource is not suitable for being render target!");
		ZE_ASSERT(GetDSV(dsv), "Current resource is not suitable for being depth stencil!");
		ZE_ASSERT(rtvMips != nullptr, "Mips not supported as no resource has been created with mips greater than 1!");
		ZE_ASSERT(dsvMips != nullptr, "Mips not supported as no resource has been created with mips greater than 1!");
		ZE_ASSERT(rtvMips[rtv - 1] != nullptr, "Mips for current resource not supported!");
		ZE_ASSERT(dsvMips[dsv - 1] != nullptr, "Mips for current resource not supported!");
		ZE_ASSERT(mipLevel < GetMipCount(rtv), "Mip level outside available range!");
		ZE_ASSERT(mipLevel < GetMipCount(dsv), "Mip level outside available range!");

		SetViewport(cl.Get().dx11, rtv);
		cl.Get().dx11.GetContext()->OMSetRenderTargets(1, reinterpret_cast<ID3D11RenderTargetView**>(rtvMips[rtv - 1][mipLevel].GetAddressOf()), dsv != INVALID_RID ? reinterpret_cast<ID3D11DepthStencilView*>(dsvMips[dsv - 1][mipLevel].Get()) : nullptr);
	}

	void FrameBuffer::SetSRV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID srv) const noexcept
	{
		ZE_ASSERT(GetSRV(srv), "Current resource is not suitable for being shader resource!");

		auto& schema = bindCtx.BindingSchema.Get().dx11;
		auto slotInfo = schema.GetCurrentSlot(bindCtx.Count++);

		auto* ctx = cl.Get().dx11.GetContext();
		for (U32 i = 0; i < slotInfo.SlotsCount; ++i)
		{
			auto slotData = schema.GetSlotData(slotInfo.DataStart + i);
			currentSlots.emplace_back(true, slotData);
			for (U32 j = 0; j < slotData.Count; ++j, ++slotData.BindStart, ++srv)
			{
				auto* view = GetSRV(srv);
				if (slotData.Shaders & GFX::Resource::ShaderType::Compute)
					ctx->CSSetShaderResources(slotData.BindStart, 1, reinterpret_cast<ID3D11ShaderResourceView**>(&view));
				else
				{
					if (slotData.Shaders & GFX::Resource::ShaderType::Vertex)
						ctx->VSSetShaderResources(slotData.BindStart, 1, reinterpret_cast<ID3D11ShaderResourceView**>(&view));
					if (slotData.Shaders & GFX::Resource::ShaderType::Domain)
						ctx->DSSetShaderResources(slotData.BindStart, 1, reinterpret_cast<ID3D11ShaderResourceView**>(&view));
					if (slotData.Shaders & GFX::Resource::ShaderType::Hull)
						ctx->HSSetShaderResources(slotData.BindStart, 1, reinterpret_cast<ID3D11ShaderResourceView**>(&view));
					if (slotData.Shaders & GFX::Resource::ShaderType::Geometry)
						ctx->GSSetShaderResources(slotData.BindStart, 1, reinterpret_cast<ID3D11ShaderResourceView**>(&view));
					if (slotData.Shaders & GFX::Resource::ShaderType::Pixel)
						ctx->PSSetShaderResources(slotData.BindStart, 1, reinterpret_cast<ID3D11ShaderResourceView**>(&view));
				}
			}
		}
	}

	void FrameBuffer::SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID uav) const noexcept
	{
		ZE_ASSERT(uav < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(uav != BACKBUFFER_RID, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(GetUAV(uav), "Current resource is not suitable for being unnordered access!");

		auto& schema = bindCtx.BindingSchema.Get().dx11;
		auto slotInfo = schema.GetCurrentSlot(bindCtx.Count++);

		--uav;
		auto* ctx = cl.Get().dx11.GetContext();
		for (U32 i = 0; i < slotInfo.SlotsCount; ++i)
		{
			auto slotData = schema.GetSlotData(slotInfo.DataStart + i);
			currentSlots.emplace_back(false, slotData);
			for (U32 j = 0; j < slotData.Count; ++j, ++slotData.BindStart)
			{
				if (slotData.Shaders & GFX::Resource::ShaderType::Compute)
					ctx->CSSetUnorderedAccessViews(slotData.BindStart, 1, reinterpret_cast<ID3D11UnorderedAccessView* const*>(uavs[uav++].GetAddressOf()), nullptr);
				else
				{
					ZE_FAIL("Cannot use UAV outside compute shader!");
				}
			}
		}
	}

	void FrameBuffer::SetUAV(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, RID uav, U16 mipLevel) const noexcept
	{
		ZE_ASSERT(uav < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(uav != BACKBUFFER_RID, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(GetUAV(uav), "Current resource is not suitable for being unnordered access!");
		ZE_ASSERT(uavMips != nullptr, "Mips not supported as no UAV resource has been created with mips greater than 1!");
		ZE_ASSERT(uavMips[uav - 1] != nullptr, "Mips for current resource not supported!");
		ZE_ASSERT(mipLevel < GetMipCount(uav), "Mip level outside available UAV range!");

		auto& schema = bindCtx.BindingSchema.Get().dx11;
		auto slotInfo = schema.GetCurrentSlot(bindCtx.Count++);

		--uav;
		auto* ctx = cl.Get().dx11.GetContext();
		for (U32 i = 0; i < slotInfo.SlotsCount; ++i)
		{
			auto slotData = schema.GetSlotData(slotInfo.DataStart + i);
			currentSlots.emplace_back(false, slotData);
			for (U32 j = 0; j < slotData.Count; ++j, ++slotData.BindStart)
			{
				if (slotData.Shaders & GFX::Resource::ShaderType::Compute)
					ctx->CSSetUnorderedAccessViews(slotData.BindStart, 1, reinterpret_cast<ID3D11UnorderedAccessView* const*>(uavMips[uav][mipLevel++].GetAddressOf()), nullptr);
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

	void FrameBuffer::SetResourceNGX(NVSDK_NGX_Parameter* param, std::string_view name, RID res) const noexcept
	{
		ZE_ASSERT(res < resourceCount, "Resource ID outside available range!");

		param->Set(name.data(), GetResource(res).Get());
	}

	void FrameBuffer::EndRaster(GFX::CommandList& cl) const noexcept
	{
#if !_ZE_MODE_RELEASE
		ZE_ASSERT(isRasterActive, "Calling EndRaster() while not in rasterization mode!");

		isRasterActive = false;
#endif
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
		ZE_ASSERT(rid != BACKBUFFER_RID, "Cannot use backbuffer as depth stencil!");
		ZE_ASSERT(GetDSV(rid), "Current resource is not suitable for being depth stencil!");

		cl.Get().dx11.GetContext()->ClearDepthStencilView(GetDSV(rid),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
	}

	void FrameBuffer::ClearUAV(GFX::CommandList& cl, RID rid, const ColorF4& color) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != BACKBUFFER_RID, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(GetUAV(rid), "Current resource is not suitable for being unnordered access!");

		cl.Get().dx11.GetContext()->ClearUnorderedAccessViewFloat(GetUAV(rid), reinterpret_cast<const float*>(&color));
	}

	void FrameBuffer::ClearUAV(GFX::CommandList& cl, RID rid, const Pixel colors[4]) const noexcept
	{
		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(rid != BACKBUFFER_RID, "Cannot use backbuffer as unnordered access!");
		ZE_ASSERT(GetUAV(rid), "Current resource is not suitable for being unnordered access!");

		cl.Get().dx11.GetContext()->ClearUnorderedAccessViewUint(GetUAV(rid), reinterpret_cast<const U32*>(colors));
	}

	void FrameBuffer::Copy(GFX::Device& dev, GFX::CommandList& cl, RID src, RID dest) const noexcept
	{

		ZE_ASSERT(src < resourceCount, "Source resource ID outside available range!");
		ZE_ASSERT(dest < resourceCount, "Destination resource ID outside available range!");
		ZE_ASSERT(GetDimmensions(src) == GetDimmensions(dest), "Resources must have same dimmensions for copy!");

		IDevice* device = dev.Get().dx12.GetDevice();
		IGraphicsCommandList* list = cl.Get().dx12.GetList();
		IResource* srcRes = GetResource(src).Get();
		IResource* destRes = GetResource(dest).Get();
		D3D12_RESOURCE_DESC1 srcDesc = srcRes->GetDesc1();
		D3D12_RESOURCE_DESC1 destDesc = destRes->GetDesc1();

		if (destDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER || srcDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
		{
			D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
			srcLocation.pResource = srcRes;
			if (srcDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
			{
				srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				srcLocation.SubresourceIndex = 0;
			}
			else
			{
				srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				device->GetCopyableFootprints1(&srcDesc, 0, 1, 0, &srcLocation.PlacedFootprint, nullptr, nullptr, nullptr);
			}

			D3D12_TEXTURE_COPY_LOCATION destLocation = {};
			destLocation.pResource = destRes;
			if (destDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
			{
				destLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				destLocation.SubresourceIndex = 0;
			}
			else
			{
				destLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				device->GetCopyableFootprints1(&destDesc, 0, 1, 0, &destLocation.PlacedFootprint, nullptr, nullptr, nullptr);
			}
			list->CopyTextureRegion(&destLocation, 0, 0, 0, &srcLocation, nullptr);
		}
		else
			list->CopyBufferRegion(destRes, 0, srcRes, 0, destDesc.Width);
	}

	void FrameBuffer::CopyFullResource(GFX::CommandList& cl, RID src, RID dest) const noexcept
	{
		ZE_ASSERT(src < resourceCount, "Source resource ID outside available range!");
		ZE_ASSERT(dest < resourceCount, "Destination resource ID outside available range!");
		ZE_ASSERT(GetDimmensions(src) == GetDimmensions(dest), "Resources must have same dimmensions for copy!");

		cl.Get().dx11.GetContext()->CopyResource(resources[dest].Resource.Get(), resources[src].Resource.Get());
	}

	void FrameBuffer::CopyBufferRegion(GFX::CommandList& cl, RID src, U64 srcOffset, RID dest, U64 destOffset, U64 bytes) const noexcept
	{
		ZE_ASSERT(src < resourceCount, "Source resource ID outside available range!");
		ZE_ASSERT(dest < resourceCount, "Destination resource ID outside available range!");
		ZE_ASSERT(srcOffset + bytes <= GetDimmensions(src).X, "Source copy region outside of resource!");
		ZE_ASSERT(destOffset + bytes <= GetDimmensions(dest).X, "Destination copy region outside of resource!");

		D3D11_BOX box = {};
		box.left = srcOffset;
		box.right = srcOffset + bytes;
		box.top = 0;
		box.bottom = 1;
		box.front = 0;
		box.back = 1;
		cl.Get().dx11.GetContext()->CopySubresourceRegion1(GetResource(dest).Get(), 0, destOffset, 0, 0, GetResource(src).Get(), 0, &box, 0);
	}

	void FrameBuffer::InitResource(GFX::CommandList& cl, RID rid, const GFX::Resource::CBuffer& buffer) const noexcept
	{
		ZE_ASSERT(IsBuffer(rid), "Trying to initialize non-buffer resource with CBuffer!");

		cl.Get().dx11.GetContext()->CopyResource(GetResource(rid).Get(), buffer.Get().dx11.GetBuffer());
	}

	void FrameBuffer::InitResource(GFX::CommandList& cl, RID rid, const GFX::Resource::Texture::Pack& texture, U32 index) const noexcept
	{

		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(!IsBuffer(rid), "Trying to initialize non-texture resource with texture!");

		cl.Get().dx11.GetContext()->CopyResource(GetResource(rid).Get(), texture.Get().dx11.GetResource(index));
	}

	void FrameBuffer::Barrier(GFX::CommandList& cl, const GFX::Pipeline::BarrierTransition* barriers, U32 count) const noexcept
	{

		ZE_ASSERT(barriers, "Empty barriers to perform!");
		ZE_ASSERT(count > 0, "No barriers to perform!");

		std::vector<D3D12_TEXTURE_BARRIER> texBarriers;
		std::vector<D3D12_BUFFER_BARRIER> buffBarriers;
		texBarriers.reserve(count);
		U32 texCount = 0, buffCount = 0;
		for (U32 i = 0; i < count; ++i)
		{
			if (IsBuffer(barriers[i].Resource))
			{
				FillBarier(buffBarriers.emplace_back(), barriers[i]);
				++buffCount;
			}
			else
			{
				FillBarier(texBarriers.emplace_back(), barriers[i]);
				++texCount;
			}
		}
		PerformBarrier(cl.Get().dx12, texBarriers.data(), texCount, buffBarriers.data(), buffCount);
	}

	void FrameBuffer::Barrier(GFX::CommandList& cl, const GFX::Pipeline::BarrierTransition& desc) const noexcept
	{

		D3D12_TEXTURE_BARRIER barrierTex;
		D3D12_BUFFER_BARRIER barrierBuff;
		bool buffer = IsBuffer(desc.Resource);
		if (buffer)
			FillBarier(barrierBuff, desc);
		else
			FillBarier(barrierTex, desc);
		PerformBarrier(cl.Get().dx12, &barrierTex, static_cast<U32>(!buffer), &barrierBuff, static_cast<U32>(buffer));
	}

	void FrameBuffer::RegisterOutsideResource(RID rid, GFX::Resource::Texture::Pack& textures, U32 textureIndex, GFX::Pipeline::FrameResourceType type) noexcept
	{

		ZE_ASSERT(rid < resourceCount, "Resource ID outside available range!");
		ZE_ASSERT(resources[rid].IsOutsideResource(), "Trying to register data to incorrect not outside resource!");
		ZE_ASSERT(type != GFX::Pipeline::FrameResourceType::Buffer, "Cannot register buffer resource when passing texture pack!");
		ZE_ASSERT(textures.Get().dx12.GetDescInfo().GpuSide, "Texture descriptors need to be visible for GPU!");

		auto& res = resources[rid];
		res.Resource = textures.Get().dx12.GetRes(textureIndex);

		const auto& resDesc = res.Resource->GetDesc1();
		const auto& texDescInfo = textures.Get().dx12.GetDescInfo();

		res.Size = { Utils::SafeCast<U32>(resDesc.Width), resDesc.Height };
		res.Array = resDesc.DepthOrArraySize;
		res.Mips = resDesc.MipLevels;
		res.Format = DX::GetFormatFromDX(resDesc.Format);

		if (type == GFX::Pipeline::FrameResourceType::TextureCube)
			res.SetCube();
		else if (type != GFX::Pipeline::FrameResourceType::Texture3D && res.Array > 1)
			res.SetArrayView();

		srvHandles[rid].CpuShaderVisibleHandle = texDescInfo.CPU;
		srvHandles[rid].GpuShaderVisibleHandle = texDescInfo.GPU;
	}

	Status FrameBuffer::MapResource(GFX::Device& dev, RID rid, void** ptr) const noexcept
	{
		auto res = GetResource(rid);

		// Retrieve original usage
		D3D11_USAGE usage = D3D11_USAGE_DEFAULT;
		UINT accessFlags = 0;
		if (IsBuffer(rid))
		{
			DX::ComPtr<IBuffer> buffer;
			ZE_DX_RET_FAILED(res.As(&buffer));

			D3D11_BUFFER_DESC desc = {};
			buffer->GetDesc(&desc);
			usage = desc.Usage;
			accessFlags = desc.CPUAccessFlags;
		}
		else if (IsTexture3D(rid))
		{
			DX::ComPtr<ITexture3D> tex;
			ZE_DX_RET_FAILED(res.As(&tex));

			D3D11_TEXTURE3D_DESC1 desc = {};
			tex->GetDesc1(&desc);
			usage = desc.Usage;
			accessFlags = desc.CPUAccessFlags;
			
		}
		else if (IsTexture1D(rid))
		{
			DX::ComPtr<ITexture1D> tex;
			ZE_DX_RET_FAILED(res.As(&tex));

			D3D11_TEXTURE1D_DESC desc = {};
			tex->GetDesc(&desc);
			usage = desc.Usage;
			accessFlags = desc.CPUAccessFlags;
		}
		else
		{
			DX::ComPtr<ITexture2D> tex;
			ZE_DX_RET_FAILED(res.As(&tex));

			D3D11_TEXTURE2D_DESC1 desc = {};
			tex->GetDesc1(&desc);
			usage = desc.Usage;
			accessFlags = desc.CPUAccessFlags;
		}

		D3D11_MAP mapType = D3D11_MAP_READ;
		switch (usage)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case D3D11_USAGE_DEFAULT:
		case D3D11_USAGE_IMMUTABLE:
			return ZE_DX_ERROR(DX::Error::INVALID_MAP_RESOURCE);
		case D3D11_USAGE_DYNAMIC:
			mapType = D3D11_MAP_WRITE_DISCARD;
			break;
		case D3D11_USAGE_STAGING:
		{
			const bool write = accessFlags & D3D11_CPU_ACCESS_WRITE;
			if (accessFlags & D3D11_CPU_ACCESS_READ && write)
				mapType = D3D11_MAP_READ_WRITE;
			else if (write)
				mapType = D3D11_MAP_WRITE;
			break;
		}
		}

		D3D11_MAPPED_SUBRESOURCE subres = {};
		ZE_DX_RET_FAILED(dev.Get().dx11.GetMainContext()->Map(res.Get(), 0, mapType, 0, &subres));
		*ptr = subres.pData;
		return {};
	}

	void FrameBuffer::UnmapResource(RID rid) const noexcept
	{
		auto res = GetResource(rid);

		DX::ComPtr<ID3D11Device> dev;
		res->GetDevice(&dev);

		DX::ComPtr<ID3D11DeviceContext> ctx;
		dev->GetImmediateContext(&ctx);
		ctx->Unmap(res.Get(), 0);
	}

	Status FrameBuffer::ExecuteIndirect(GFX::CommandList& cl, GFX::CommandSignature& signature, RID commandsBuffer, U32 commandsOffset) const noexcept
	{
		DX::ComPtr<IBuffer> argsBuffer;
		ZE_DX_RET_FAILED(GetResource(commandsBuffer).As(&argsBuffer));

		switch (signature.Get().dx11.GetType())
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::IndirectCommandType::Dispatch:
			cl.Get().dx11.GetContext()->DispatchIndirect(argsBuffer.Get(), commandsOffset);
			break;
		}
		return {};
	}

	Status FrameBuffer::SwapBackbuffer(GFX::Device& dev, GFX::SwapChain& swapChain) noexcept
	{
		if (resources[BACKBUFFER_RID].Resource == nullptr)
		{
			resources[BACKBUFFER_RID].Resource = swapChain.Get().dx11.GetBuffer();
			rtvs[BACKBUFFER_RID] = swapChain.Get().dx11.GetRTV();
			srvs[BACKBUFFER_RID] = swapChain.Get().dx11.GetSRV();
		}
		return {};
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