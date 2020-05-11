#include "OutlineDraw.h"

namespace GFX::Visual
{
	OutlineDraw::OutlineDraw(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout)
	{
		auto vertexShader = Resource::VertexShader::Get(gfx, "SolidVS.cso");
		AddBind(Resource::InputLayout::Get(gfx, vertexLayout, vertexShader->GetBytecode()));
		AddBind(std::move(vertexShader));
	}
}