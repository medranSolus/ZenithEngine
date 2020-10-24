#include "ShadowMap.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Visual
{
	inline GFX::Data::CBuffer::DCBLayout ShadowMap::MakeLayout() noexcept
	{
		static GFX::Data::CBuffer::DCBLayout layout;
		static bool initNeeded = true;
		if (initNeeded)
		{
			layout.Add(DCBElementType::Float, "parallaxScale");
			initNeeded = false;
		}
		return layout;
	}

	ShadowMap::ShadowMap(Graphics& gfx, std::shared_ptr<Visual::Material> material)
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
		auto vertexShader = Resource::VertexShader::Get(gfx, "ShadowVS" + shaderType);
		AddBind(Resource::InputLayout::Get(gfx, material->GerVertexLayout(), vertexShader));
		AddBind(std::move(vertexShader));
	}

	void ShadowMap::Bind(Graphics& gfx)
	{
		IVisual::Bind(gfx);
		if (parallaxBuffer)
		{
			if (Math::NotEquals(parallaxBuffer->GetBufferConst()["parallaxScale"], sourcePixelBuffer->GetBufferConst()["parallaxScale"]))
				parallaxBuffer->GetBuffer()["parallaxScale"] = static_cast<float>(sourcePixelBuffer->GetBufferConst()["parallaxScale"]);
			parallaxBuffer->Bind(gfx);
		}
		if (diffuseTexture)
			diffuseTexture->Bind(gfx);
		if (normalMap)
			normalMap->Bind(gfx);
		if (parallaxMap)
			parallaxMap->Bind(gfx);
	}
}