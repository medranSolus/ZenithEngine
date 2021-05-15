#include "GFX/Pipeline/Resource/DepthStencil.h"
#include "GFX/Resource/GfxDebugName.h"
#include "Exception/ImageException.h"
#include "GFX/Graphics.h"

namespace ZE::GFX::Pipeline::Resource
{
	constexpr DXGI_FORMAT DepthStencil::UsageTypeless(Usage usage) noexcept
	{
		switch (usage)
		{
		default:
		case Usage::DepthStencil:
			return DXGI_FORMAT_R24G8_TYPELESS;
		case Usage::DepthOnly:
			return DXGI_FORMAT_R32_TYPELESS;
		}
	}

	constexpr DXGI_FORMAT DepthStencil::UsageTyped(Usage usage) noexcept
	{
		switch (usage)
		{
		default:
		case Usage::DepthStencil:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case Usage::DepthOnly:
			return DXGI_FORMAT_D32_FLOAT;
		}
	}

	DepthStencil::DepthStencil(Graphics& gfx, U32 width, U32 height, bool shaderResource, Usage usage)
		: IBufferResource(gfx, width, height), usage(usage)
	{
		ZE_GFX_ENABLE_ALL(gfx);

		D3D11_TEXTURE2D_DESC depthTexDesc = { 0 };
		depthTexDesc.Width = static_cast<UINT>(width);
		depthTexDesc.Height = static_cast<UINT>(height);
		depthTexDesc.MipLevels = 0;
		depthTexDesc.ArraySize = 1;
		depthTexDesc.Format = UsageTypeless(usage);
		depthTexDesc.SampleDesc.Count = 1; // Antialiasing stuff
		depthTexDesc.SampleDesc.Quality = 0;
		depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
		depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | (shaderResource ? D3D11_BIND_SHADER_RESOURCE : 0);
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTexture = nullptr;
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateTexture2D(&depthTexDesc, nullptr, &depthTexture));

		D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc = {};
		depthViewDesc.Format = UsageTyped(usage);
		depthViewDesc.Flags = 0;
		depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
		depthViewDesc.Texture2D.MipSlice = 0;
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateDepthStencilView(depthTexture.Get(), &depthViewDesc, &depthStencilView));
		ZE_GFX_SET_RID(depthStencilView.Get());
	}

	DepthStencil::DepthStencil(Graphics& gfx, Usage usage)
		: DepthStencil(gfx, gfx.GetWidth(), gfx.GetHeight(), false, usage) {}

	DepthStencil::DepthStencil(Graphics& gfx, U32 size)
		: IBufferResource(gfx, size, size), usage(usage)
	{
		ZE_GFX_ENABLE_ALL(gfx);

		D3D11_TEXTURE2D_DESC depthTexDesc = { 0 };
		depthTexDesc.Width = depthTexDesc.Height = static_cast<UINT>(size);
		depthTexDesc.MipLevels = 1;
		depthTexDesc.ArraySize = 6;
		depthTexDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
		depthTexDesc.SampleDesc.Count = 1; // Antialiasing stuff
		depthTexDesc.SampleDesc.Quality = 0;
		depthTexDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		depthTexDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTexture = nullptr;
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateTexture2D(&depthTexDesc, nullptr, &depthTexture));

		D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc = {};
		depthViewDesc.Format = depthTexDesc.Format;
		depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		depthViewDesc.Flags = 0U;
		depthViewDesc.Texture2DArray.MipSlice = 0;
		depthViewDesc.Texture2DArray.ArraySize = 6;
		depthViewDesc.Texture2DArray.FirstArraySlice = 0;
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateDepthStencilView(depthTexture.Get(), &depthViewDesc, &depthStencilView));
		ZE_GFX_SET_RID(depthStencilView.Get());
	}

#ifdef _ZE_MODE_DEBUG
	std::string DepthStencil::GetRID() const noexcept
	{
		return "DS" + std::to_string(GetWidth()) + "x" + std::to_string(GetHeight()) + "#" + std::to_string(static_cast<bool>(usage));
	}
#endif

	Surface DepthStencil::ToSurface(Graphics& gfx, bool linearScale) const
	{
		ZE_GFX_ENABLE_ALL(gfx);

		// Create temporary texture to read from CPU side
		Microsoft::WRL::ComPtr<ID3D11Resource> resource;
		depthStencilView->GetResource(&resource);
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		resource->QueryInterface(IID_PPV_ARGS(&texture));

		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		texture->GetDesc(&textureDesc);
		textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		textureDesc.Usage = D3D11_USAGE_STAGING;
		textureDesc.BindFlags = 0;
		textureDesc.MiscFlags = 0;
		textureDesc.ArraySize = 1;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> textureStaged;
		ZE_GFX_THROW_FAILED(GetDevice(gfx)->CreateTexture2D(&textureDesc, nullptr, &textureStaged));

		D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc;
		depthStencilView->GetDesc(&depthDesc);
		// Texture in  cube map
		if (depthDesc.ViewDimension == D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2DARRAY)
		{
			ZE_GFX_THROW_FAILED_INFO(GetContext(gfx)->CopySubresourceRegion(textureStaged.Get(), 0, 0, 0, 0,
				texture.Get(), depthDesc.Texture2DArray.FirstArraySlice, nullptr));
		}
		else
		{
			ZE_GFX_THROW_FAILED_INFO(GetContext(gfx)->CopyResource(textureStaged.Get(), texture.Get()));
		}

		Surface surface(GetWidth(), GetHeight());
		D3D11_MAPPED_SUBRESOURCE subResource = { 0 };
		ZE_GFX_THROW_FAILED_INFO(GetContext(gfx)->Map(textureStaged.Get(), 0, D3D11_MAP_READ, 0, &subResource));
		const U8* bytes = static_cast<const U8*>(subResource.pData);
		for (U32 y = 0; y < GetHeight(); ++y)
		{
			const Pixel* row = reinterpret_cast<const Pixel*>(bytes + subResource.RowPitch * static_cast<U64>(y));
			for (U32 x = 0; x < GetWidth(); ++x)
			{
				switch (textureDesc.Format)
				{
				case DXGI_FORMAT_R24G8_TYPELESS:
				{
					const U32 raw = 0xFFFFFF & (row + x)->GetValue();
					if (linearScale)
					{
						const float normalized = static_cast<float>(raw) / static_cast<float>(0xFFFFFF);
						const float linearized = normalized * 0.01f / (1.01f - normalized);
						const U8 channel = static_cast<U8>(linearized * 255.0f);
						surface.PutPixel(x, y, { channel, channel, channel });
					}
					else
					{
						const U8 channel = raw >> 16;
						surface.PutPixel(x, y, { channel, channel, channel });
					}
					break;
				}
				case DXGI_FORMAT_R32_TYPELESS:
				{
					const float raw = *reinterpret_cast<const float*>(row + x);
					if (linearScale)
					{
						const float linearized = raw * 0.01f / (1.01f - raw);
						const U8 channel = static_cast<U8>(linearized * 255.0f);
						surface.PutPixel(x, y, { channel, channel, channel });
					}
					else
					{
						const U8 channel = static_cast<U8>(raw * 255.0f);
						surface.PutPixel(x, y, { channel, channel, channel });
					}
					break;
				}
				default:
					throw ZE_IMG_EXCEPT("Cannot convert DepthStencil to Surface, wrong DXGI_FORMAT!");
				}
			}
		}
		ZE_GFX_THROW_FAILED_INFO(GetContext(gfx)->Unmap(textureStaged.Get(), 0));
		return surface;
	}
}