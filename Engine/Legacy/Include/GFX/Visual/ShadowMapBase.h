#pragma once
#include "Material.h"
#include "GFX/Resource/GfxResources.h"
#include "MathExt.h"

namespace ZE::GFX::Visual
{
	template<bool Cube>
	class ShadowMapBase : public IVisual
	{
		mutable GfxResPtr<Resource::ConstBufferExPixelCache> parallaxBuffer;
		GfxResPtr<Resource::ConstBufferExPixelCache> sourcePixelBuffer;
		GfxResPtr<Resource::Texture> diffuseTexture;
		GfxResPtr<Resource::Texture> normalMap;
		GfxResPtr<Resource::Texture> parallaxMap;

		static Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		ShadowMapBase(Graphics& gfx, const std::shared_ptr<Material>& material);
		ShadowMapBase(ShadowMapBase&&) = default;
		ShadowMapBase(const ShadowMapBase&) = default;
		ShadowMapBase& operator=(ShadowMapBase&&) = default;
		ShadowMapBase& operator=(const ShadowMapBase&) = default;
		virtual ~ShadowMapBase() = default;

		void Bind(Graphics& gfx) const override;
	};

#pragma region Functions
	template<bool Cube>
	Data::CBuffer::DCBLayout ShadowMapBase<Cube>::MakeLayout() noexcept
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

	template<bool Cube>
	ShadowMapBase<Cube>::ShadowMapBase(Graphics& gfx, const std::shared_ptr<Material>& material)
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
				parallaxBuffer = Resource::ConstBufferExPixelCache::Get(gfx, sourcePixelBuffer->GetRID() + "_shadowPax", MakeLayout(), 1);
				parallaxBuffer->GetBuffer()["parallaxScale"] = static_cast<float>(sourcePixelBuffer->GetBufferConst()["parallaxScale"]);
			}
		}
		AddBind(Resource::PixelShader::Get(gfx, "ShadowPS" + shaderType));
		GfxResPtr<Resource::VertexShader> vertexShader;
		if constexpr (Cube)
		{
			AddBind(Resource::GeometryShader::Get(gfx, "ShadowCubeGS" + shaderType));
			vertexShader = Resource::VertexShader::Get(gfx, "ShadowCubeVS" + shaderType);
		}
		else
			vertexShader = Resource::VertexShader::Get(gfx, "ShadowVS" + shaderType);
		AddBind(Resource::InputLayout::Get(gfx, material->GerVertexLayout(), vertexShader));
		AddBind(std::move(vertexShader));
	}

	template<bool Cube>
	void ShadowMapBase<Cube>::Bind(Graphics& gfx) const
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
#pragma endregion

	typedef ShadowMapBase<false> ShadowMap;
	typedef ShadowMapBase<true> ShadowMapCube;
}