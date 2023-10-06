#include "RHI/DX11/Resource/Generic.h"

namespace ZE::RHI::DX11::Resource
{
	Generic::Generic(GFX::Device& dev, const GFX::Resource::GenericResourceDesc& desc)
	{
		Device& device = dev.Get().dx11;
		ZE_DX_ENABLE_ID(device);

		D3D11_SUBRESOURCE_DATA resData = {};
		resData.pSysMem = desc.InitData;
		resData.SysMemPitch = 0;
		resData.SysMemSlicePitch = 0;

		switch (desc.Type)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::GenericResourceType::Buffer:
		{
			D3D11_BUFFER_DESC bufferDesc = {};
			bufferDesc.ByteWidth = Math::AlignUp(desc.WidthOrBufferSize, 16U);
			bufferDesc.BindFlags = desc.InitState == GFX::Resource::State::StateConstantBuffer ? D3D11_BIND_CONSTANT_BUFFER : D3D11_BIND_SHADER_RESOURCE;

			switch (desc.Heap)
			{
			case GFX::Resource::GenericResourceHeap::GPU:
			{
				bufferDesc.Usage = desc.Flags == GFX::Resource::GenericResourceFlag::ReadOnly ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DEFAULT;
				bufferDesc.CPUAccessFlags = 0;
				break;
			}
			default:
				ZE_ENUM_UNHANDLED();
			case GFX::Resource::GenericResourceHeap::Upload:
			{
				bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
				bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				break;
			}
			case GFX::Resource::GenericResourceHeap::Readback:
			{
				bufferDesc.Usage = D3D11_USAGE_STAGING;
				bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
				break;
			}
			}

			bufferDesc.MiscFlags = 0; // D3D11_RESOURCE_MISC_TEXTURECUBE
			bufferDesc.StructureByteStride = 0;

			// If aligned size is greater than actual data then create new region
			std::unique_ptr<U8[]> buffData = nullptr;
			if (desc.InitData && bufferDesc.ByteWidth > desc.WidthOrBufferSize)
			{
				buffData = std::make_unique<U8[]>(bufferDesc.ByteWidth);
				std::memcpy(buffData.get(), desc.InitData, desc.WidthOrBufferSize);
				resData.pSysMem = buffData.get();
			}

			DX::ComPtr<IBuffer> res;
			ZE_DX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateBuffer(&bufferDesc, desc.InitData ? &resData : nullptr, &res));
			ZE_DX_SET_ID(res, desc.DebugName);
			ZE_DX_THROW_FAILED(res.As(&resource));
			break;
		}
		case GFX::Resource::GenericResourceType::Texture1D:
		{
			D3D11_TEXTURE1D_DESC texDesc = {};
			texDesc.Width = desc.WidthOrBufferSize;
			texDesc.MipLevels = desc.MipCount;
			texDesc.ArraySize = desc.DepthOrArraySize;
			texDesc.Format = DX::GetDXFormat(desc.Format);
			texDesc.Usage;
			texDesc.BindFlags;
			texDesc.CPUAccessFlags;
			texDesc.MiscFlags;
			break;
		}
		case GFX::Resource::GenericResourceType::Texture2D:
		{
			break;
		}
		case GFX::Resource::GenericResourceType::TextureCube:
		{
			break;
		}
		case GFX::Resource::GenericResourceType::Texture3D:
		{
			break;
		}
		}
	}
}