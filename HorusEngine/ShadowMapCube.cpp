#include "ShadowMapCube.h"
#include "GfxResources.h"

namespace GFX::Visual
{
	ShadowMapCube::ShadowMapCube(Graphics& gfx, std::shared_ptr<Visual::Material> material)
	{
		std::string shaderType = "";
		if (material->IsTexture())
		{
			shaderType = "Texture";
			diffuseTexture = material->GetTexture();
			if (material->IsParallax())
			{
				shaderType += "Parallax";
				//normalMap = material->GetNormalMap();
				parallaxMap = material->GetParallaxMap();
			}
		}
		AddBind(Resource::PixelShader::Get(gfx, "ShadowPS" + shaderType));
		AddBind(Resource::GeometryShader::Get(gfx, "ShadowCubeGS" + shaderType));
		auto vertexShader = Resource::VertexShader::Get(gfx, "ShadowCubeVS" + shaderType);
		AddBind(Resource::InputLayout::Get(gfx, material->GerVertexLayout(), vertexShader));
		AddBind(std::move(vertexShader));
	}

	void ShadowMapCube::Bind(Graphics& gfx)
	{
		IVisual::Bind(gfx);
		if (diffuseTexture)
			diffuseTexture->Bind(gfx);
		if (normalMap)
			normalMap->Bind(gfx);
		if (parallaxMap)
			parallaxMap->Bind(gfx);
	}
}