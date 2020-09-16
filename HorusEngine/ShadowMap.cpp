#include "ShadowMap.h"
#include "GfxResources.h"

namespace GFX::Visual
{
	ShadowMap::ShadowMap(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout)
	{
		auto vertexShader = Resource::VertexShader::Get(gfx, "ShadowVS");
		AddBind(Resource::InputLayout::Get(gfx, vertexLayout, vertexShader));
		AddBind(std::move(vertexShader));
	}
}