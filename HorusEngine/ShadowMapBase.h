#pragma once
#include "Material.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Visual
{
	template<bool cube>
	class ShadowMapBase : public IVisual
	{
		GfxResPtr<Resource::ConstBufferExPixelCache> sourcePixelBuffer;
		GfxResPtr<Resource::ConstBufferExPixelCache> parallaxBuffer;
		GfxResPtr<Resource::Texture> diffuseTexture;
		GfxResPtr<Resource::Texture> normalMap;
		GfxResPtr<Resource::Texture> parallaxMap;

		static inline Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		ShadowMapBase(Graphics& gfx, std::shared_ptr<Material> material);
		virtual ~ShadowMapBase() = default;

		inline bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return false; }

		void Bind(Graphics& gfx) override;
	};

	template<bool cube>
	inline Data::CBuffer::DCBLayout ShadowMapBase<cube>::MakeLayout() noexcept
	{
		static Data::CBuffer::DCBLayout layout;
		static bool initNeeded = true;
		if (initNeeded)
		{
			layout.Add(DCBElementType::Float, "parallaxScale");
			initNeeded = false;
		}
		return layout;
	}

	template<bool cube>
	ShadowMapBase<cube>::ShadowMapBase(Graphics& gfx, std::shared_ptr<Material> material)
	{
		std::string shaderType = "";
		if (material->IsTexture())
		{
			shaderType = "Texture";
			diffuseTexture = material->GetTexture();
			if (material->IsParallax())
			{
				shaderType += "Parallax";
				normalMap = material->GetNormalMap();
				parallaxMap = material->GetParallaxMap();
				sourcePixelBuffer = material->GetBuffer();
				parallaxBuffer = Resource::ConstBufferExPixelCache::Get(gfx, sourcePixelBuffer->GetRID() + "_shadowPax", MakeLayout(), 1U);
				parallaxBuffer->GetBuffer()["parallaxScale"] = static_cast<float>(sourcePixelBuffer->GetBufferConst()["parallaxScale"]);
			}
		}
		AddBind(Resource::PixelShader::Get(gfx, "ShadowPS" + shaderType));
		GfxResPtr<Resource::VertexShader> vertexShader;
		if constexpr (cube)
		{
			AddBind(Resource::GeometryShader::Get(gfx, "ShadowCubeGS" + shaderType));
			vertexShader = Resource::VertexShader::Get(gfx, "ShadowCubeVS" + shaderType);
		}
		else
			vertexShader = Resource::VertexShader::Get(gfx, "ShadowVS" + shaderType);
		AddBind(Resource::InputLayout::Get(gfx, material->GerVertexLayout(), vertexShader));
		AddBind(std::move(vertexShader));
	}

	template<bool cube>
	void ShadowMapBase<cube>::Bind(Graphics& gfx)
	{
		IVisual::Bind(gfx);
		if (parallaxBuffer != nullptr)
		{
			if (Math::NotEquals(parallaxBuffer->GetBufferConst()["parallaxScale"], sourcePixelBuffer->GetBufferConst()["parallaxScale"]))
				parallaxBuffer->GetBuffer()["parallaxScale"] = static_cast<float>(sourcePixelBuffer->GetBufferConst()["parallaxScale"]);
			parallaxBuffer->Bind(gfx);
		}
		if (diffuseTexture != nullptr)
			diffuseTexture->Bind(gfx);
		if (normalMap != nullptr)
			normalMap->Bind(gfx);
		if (parallaxMap != nullptr)
			parallaxMap->Bind(gfx);
	}

	typedef ShadowMapBase<false> ShadowMap;
	typedef ShadowMapBase<true> ShadowMapCube;
}