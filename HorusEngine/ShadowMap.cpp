#include "ShadowMap.h"
#include "GfxResources.h"

namespace GFX::Visual
{
	ShadowMap::ShadowMap(Graphics& gfx, std::shared_ptr<Visual::Material> material)
	{
		std::string shaderType = "";
		if (material->IsTexture())
		{
			shaderType = "Texture";
			diffuseTexture = material->GetTexture();
			AddBind(Resource::Sampler::Get(gfx, Resource::Sampler::Type::Anisotropic, false));
		}
		AddBind(GFX::Resource::PixelShader::Get(gfx, "ShadowPS" + shaderType));
		auto vertexShader = Resource::VertexShader::Get(gfx, "ShadowVS" + shaderType);
		AddBind(Resource::InputLayout::Get(gfx, material->GerVertexLayout(), vertexShader));
		AddBind(std::move(vertexShader));
	}

	void ShadowMap::Bind(Graphics& gfx)
	{
		IVisual::Bind(gfx);
		if (diffuseTexture)
			diffuseTexture->Bind(gfx);
	}
}