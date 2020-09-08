#include "DepthStencil.h"
#include "GfxExceptionMacros.h"

namespace GFX::Pipeline::Resource
{
	inline DXGI_FORMAT DepthStencil::UsageTypeless(Usage usage) noexcept
	{
		switch (usage)
		{
		default:
		case Usage::DepthStencil:
			return DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS;
		case Usage::ShadowDepth:
			return DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
		}
	}

	inline DXGI_FORMAT DepthStencil::UsageTyped(Usage usage) noexcept
	{
		switch (usage)
		{
		default:
		case Usage::DepthStencil:
			return DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
		case Usage::ShadowDepth:
			return DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
		}
	}

	DepthStencil::DepthStencil(Graphics& gfx, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture, UINT size, UINT face)
		: IBufferResource(gfx, size, size)
	{
		GFX_ENABLE_ALL(gfx);
		D3D11_TEXTURE2D_DESC depthTexDesc;
		texture->GetDesc(&depthTexDesc);

		D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc = {};
		depthViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
		depthViewDesc.Flags = 0U;
		depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		depthViewDesc.Texture2DArray.MipSlice = 0U;
		depthViewDesc.Texture2DArray.ArraySize = 1U;
		depthViewDesc.Texture2DArray.FirstArraySlice = face;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateDepthStencilView(texture.Get(), &depthViewDesc, &depthStencilView));
	}

	DepthStencil::DepthStencil(Graphics& gfx, unsigned int width, unsigned int height, bool shaderResource, Usage usage) : IBufferResource(gfx, width, height)
	{
		GFX_ENABLE_ALL(gfx);

		D3D11_TEXTURE2D_DESC depthTexDesc = { 0 };
		depthTexDesc.Width = static_cast<UINT>(width);
		depthTexDesc.Height = static_cast<UINT>(height);
		depthTexDesc.MipLevels = 0U;
		depthTexDesc.ArraySize = 1U;
		depthTexDesc.Format = UsageTypeless(usage);
		depthTexDesc.SampleDesc.Count = 1U; // Antialiasing stuff
		depthTexDesc.SampleDesc.Quality = 0U;
		depthTexDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		depthTexDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL | (shaderResource ? D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE : 0U);
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTexture = nullptr;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateTexture2D(&depthTexDesc, nullptr, &depthTexture));

		D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc = {};
		depthViewDesc.Format = UsageTyped(usage);
		depthViewDesc.Flags = 0U;
		depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
		depthViewDesc.Texture2D.MipSlice = 0U;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateDepthStencilView(depthTexture.Get(), &depthViewDesc, &depthStencilView));
	}

	Surface DepthStencil::ToSurface(Graphics& gfx, bool linearScale) const
	{
		GFX_ENABLE_ALL(gfx);

		// Create temporary texture to read from CPU side
		Microsoft::WRL::ComPtr<ID3D11Resource> resource;
		depthStencilView->GetResource(&resource);
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		resource.As(&texture);
		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		texture->GetDesc(&textureDesc);
		textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_READ;
		textureDesc.Usage = D3D11_USAGE::D3D11_USAGE_STAGING;
		textureDesc.BindFlags = 0U;
		textureDesc.MiscFlags = 0U;
		textureDesc.ArraySize = 1U;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> textureStaged;
		GFX_THROW_FAILED(GetDevice(gfx)->CreateTexture2D(&textureDesc, nullptr, &textureStaged));

		D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc;
		depthStencilView->GetDesc(&depthDesc);
		// Texture in  cube map
		if (depthDesc.ViewDimension == D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2DARRAY)
		{
			GFX_THROW_FAILED_INFO(GetContext(gfx)->CopySubresourceRegion(textureStaged.Get(), 0U, 0U, 0U, 0U, texture.Get(), depthDesc.Texture2DArray.FirstArraySlice, nullptr));
		}
		else
		{
			GFX_THROW_FAILED_INFO(GetContext(gfx)->CopyResource(textureStaged.Get(), texture.Get()));
		}

		Surface surface(GetWidth(), GetHeight());
		D3D11_MAPPED_SUBRESOURCE subResource = { 0 };
		GFX_THROW_FAILED_INFO(GetContext(gfx)->Map(textureStaged.Get(), 0U, D3D11_MAP::D3D11_MAP_READ, 0U, &subResource));
		const char* bytes = static_cast<const char*>(subResource.pData);
		for (unsigned int y = 0U; y < GetHeight(); ++y)
		{
			const Surface::Pixel* row = reinterpret_cast<const Surface::Pixel*>(bytes + subResource.RowPitch * static_cast<size_t>(y));
			for (unsigned int x = 0U; x < GetWidth(); ++x)
			{
				switch (textureDesc.Format)
				{
				case DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS:
				{
					const auto raw = 0xFFFFFF & (row + x)->GetValue();
					if (linearScale)
					{
						const float normalized = static_cast<float>(raw) / static_cast<float>(0xFFFFFF);
						const float linearized = normalized * 0.01f / (1.01f - normalized);
						const uint8_t channel = static_cast<uint8_t>(linearized * 255.0f);
						surface.PutPixel(x, y, { channel, channel, channel });
					}
					else
					{
						const uint8_t channel = raw >> 16;
						surface.PutPixel(x, y, { channel, channel, channel });
					}
					break;
				}
				case DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS:
				{
					const auto raw = *reinterpret_cast<const float*>(row + x);
					if (linearScale)
					{
						const float linearized = raw * 0.01f / (1.01f - raw);
						const uint8_t channel = static_cast<uint8_t>(linearized * 255.0f);
						surface.PutPixel(x, y, { channel, channel, channel });
					}
					else
					{
						const uint8_t channel = static_cast<uint8_t>(raw * 255.0f);
						surface.PutPixel(x, y, { channel, channel, channel });
					}
					break;
				}
				default:
					throw Surface::ImageException(__LINE__, __FILE__, "Cannot convert DepthStencil to Surface, wrong DXGI_FORMAT!");
				}
			}
		}
		GFX_THROW_FAILED_INFO(GetContext(gfx)->Unmap(textureStaged.Get(), 0U));
		return surface;
	}
}