#pragma once
// Headers needed for DirectX 11
#include "GFX/API/DX/DXGI.h"
#include "GFX/Resource/PipelineStateDesc.h"
#include "GFX/Resource/SamplerDesc.h"
#include "WarningGuardOn.h"
#include <d3d11_4.h>
#include "WarningGuardOff.h"

namespace ZE::GFX::API::DX11
{
	// Get DirectX 11 version of comparison function
	constexpr D3D11_COMPARISON_FUNC GetComparisonFunc(GFX::Resource::CompareMethod func) noexcept;
	// Get DirectX 11 version of culling modes
	constexpr D3D11_CULL_MODE GetCulling(GFX::Resource::CullMode mode) noexcept;
	// Get DirectX 11 version of filter type
	constexpr D3D11_FILTER GetFilterType(U8 samplerType) noexcept;
	// Get DirectX 11 version of static border color for static sampler
	constexpr ColorF4 GetStaticBorderColor(GFX::Resource::Texture::EdgeColor color) noexcept;
	// Get DirectX 11 version of texture addressing mode
	constexpr D3D11_TEXTURE_ADDRESS_MODE GetTextureAddressMode(GFX::Resource::Texture::AddressMode mode) noexcept;

#pragma region Functions
	constexpr D3D11_COMPARISON_FUNC GetComparisonFunc(GFX::Resource::CompareMethod func) noexcept
	{
		switch (func)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::CompareMethod::Never:
			return D3D11_COMPARISON_NEVER;
		case GFX::Resource::CompareMethod::Less:
			return D3D11_COMPARISON_LESS;
		case GFX::Resource::CompareMethod::Equal:
			return D3D11_COMPARISON_EQUAL;
		case GFX::Resource::CompareMethod::LessEqual:
			return D3D11_COMPARISON_LESS_EQUAL;
		case GFX::Resource::CompareMethod::Greater:
			return D3D11_COMPARISON_GREATER;
		case GFX::Resource::CompareMethod::NotEqual:
			return D3D11_COMPARISON_NOT_EQUAL;
		case GFX::Resource::CompareMethod::GreaterEqual:
			return D3D11_COMPARISON_GREATER_EQUAL;
		case GFX::Resource::CompareMethod::Always:
			return D3D11_COMPARISON_ALWAYS;
		}
	}

	constexpr D3D11_CULL_MODE GetCulling(GFX::Resource::CullMode mode) noexcept
	{
		switch (mode)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::CullMode::None:
			return D3D11_CULL_NONE;
		case GFX::Resource::CullMode::Front:
			return D3D11_CULL_FRONT;
		case GFX::Resource::CullMode::Back:
			return D3D11_CULL_BACK;
		}
	}

	constexpr D3D11_FILTER GetFilterType(U8 samplerType) noexcept
	{
		if (samplerType == GFX::Resource::SamplerType::Default)
			return D3D11_FILTER_MIN_MAG_MIP_POINT; // 000
		if (samplerType == GFX::Resource::SamplerType::Linear)
			return D3D11_FILTER_MIN_MAG_MIP_LINEAR; // 111
		if (samplerType == GFX::Resource::SamplerType::Anisotropic)
			return D3D11_FILTER_ANISOTROPIC;

		switch (samplerType & GFX::Resource::SamplerType::OperationType)
		{
		case GFX::Resource::SamplerType::Comparison:
		{
			if ((samplerType & ~GFX::Resource::SamplerType::OperationType) == GFX::Resource::SamplerType::Point)
				return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT; // 000
			if ((samplerType & ~GFX::Resource::SamplerType::OperationType) == GFX::Resource::SamplerType::Linear)
				return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // 111
			if (samplerType & GFX::Resource::SamplerType::Anisotropic)
				return D3D11_FILTER_COMPARISON_ANISOTROPIC;

			if (samplerType & GFX::Resource::SamplerType::LinearMipSampling)
			{
				if (samplerType & GFX::Resource::SamplerType::LinearMagnification)
					return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR; // 110
				if (samplerType & GFX::Resource::SamplerType::LinearMinification)
					return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR; // 101
				return D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR; // 100
			}
			if (samplerType & GFX::Resource::SamplerType::LinearMagnification)
			{
				if (samplerType & GFX::Resource::SamplerType::LinearMinification)
					return D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT; // 011
				return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT; // 010
			}
			return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT; // 001
		}
		case GFX::Resource::SamplerType::Minimum:
		{
			if ((samplerType & ~GFX::Resource::SamplerType::OperationType) == GFX::Resource::SamplerType::Point)
				return D3D11_FILTER_MINIMUM_MIN_MAG_MIP_POINT; // 000
			if ((samplerType & ~GFX::Resource::SamplerType::OperationType) == GFX::Resource::SamplerType::Linear)
				return D3D11_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR; // 111
			if (samplerType & GFX::Resource::SamplerType::Anisotropic)
				return D3D11_FILTER_MINIMUM_ANISOTROPIC;

			if (samplerType & GFX::Resource::SamplerType::LinearMipSampling)
			{
				if (samplerType & GFX::Resource::SamplerType::LinearMagnification)
					return D3D11_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR; // 110
				if (samplerType & GFX::Resource::SamplerType::LinearMinification)
					return D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR; // 101
				return D3D11_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR; // 100
			}
			if (samplerType & GFX::Resource::SamplerType::LinearMagnification)
			{
				if (samplerType & GFX::Resource::SamplerType::LinearMinification)
					return D3D11_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT; // 011
				return D3D11_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT; // 010
			}
			return D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT; // 001
		}
		case GFX::Resource::SamplerType::Maximum:
		{
			if ((samplerType & ~GFX::Resource::SamplerType::OperationType) == GFX::Resource::SamplerType::Point)
				return D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_POINT; // 000
			if ((samplerType & ~GFX::Resource::SamplerType::OperationType) == GFX::Resource::SamplerType::Linear)
				return D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR; // 111
			if (samplerType & GFX::Resource::SamplerType::Anisotropic)
				return D3D11_FILTER_MAXIMUM_ANISOTROPIC;

			if (samplerType & GFX::Resource::SamplerType::LinearMipSampling)
			{
				if (samplerType & GFX::Resource::SamplerType::LinearMagnification)
					return D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR; // 110
				if (samplerType & GFX::Resource::SamplerType::LinearMinification)
					return D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR; // 101
				return D3D11_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR; // 100
			}
			if (samplerType & GFX::Resource::SamplerType::LinearMagnification)
			{
				if (samplerType & GFX::Resource::SamplerType::LinearMinification)
					return D3D11_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT; // 011
				return D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT; // 010
			}
			return D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT; // 001
		}
		}

		if (samplerType & GFX::Resource::SamplerType::LinearMipSampling)
		{
			if (samplerType & GFX::Resource::SamplerType::LinearMagnification)
				return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR; // 110
			if (samplerType & GFX::Resource::SamplerType::LinearMinification)
				return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR; // 101
			return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR; // 100
		}
		if (samplerType & GFX::Resource::SamplerType::LinearMagnification)
		{
			if (samplerType & GFX::Resource::SamplerType::LinearMinification)
				return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT; // 011
			return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT; // 010
		}
		return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT; // 001
	}

	constexpr ColorF4 GetStaticBorderColor(GFX::Resource::Texture::EdgeColor color) noexcept
	{
		switch (color)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::Texture::EdgeColor::TransparentBlack:
			return { 0.0f, 0.0f, 0.0f, 0.0f };
		case GFX::Resource::Texture::EdgeColor::SolidBlack:
			return { 0.0f, 0.0f, 0.0f, 1.0f };
		case GFX::Resource::Texture::EdgeColor::SolidWhite:
			return { 1.0f, 1.0f, 1.0f, 1.0f };
		}
	}

	constexpr D3D11_TEXTURE_ADDRESS_MODE GetTextureAddressMode(GFX::Resource::Texture::AddressMode mode) noexcept
	{
		switch (mode)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case GFX::Resource::Texture::AddressMode::Repeat:
			return D3D11_TEXTURE_ADDRESS_WRAP;
		case GFX::Resource::Texture::AddressMode::Mirror:
			return D3D11_TEXTURE_ADDRESS_MIRROR;
		case GFX::Resource::Texture::AddressMode::Edge:
			return D3D11_TEXTURE_ADDRESS_CLAMP;
		case GFX::Resource::Texture::AddressMode::BorderColor:
			return D3D11_TEXTURE_ADDRESS_BORDER;
		case GFX::Resource::Texture::AddressMode::MirrorOnce:
			return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
		}
	}
#pragma endregion
}