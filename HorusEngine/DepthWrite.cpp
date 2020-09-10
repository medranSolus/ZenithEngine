#include "DepthWrite.h"

namespace GFX::Visual
{
	DepthWrite::DepthWrite(Graphics& gfx, std::shared_ptr<Material> material)
	{
		auto vertexShader = Resource::VertexShader::Get(gfx, "SolidVS");
		AddBind(Resource::InputLayout::Get(gfx, material->GerVertexLayout(), vertexShader));
		AddBind(std::move(vertexShader));
	}
}