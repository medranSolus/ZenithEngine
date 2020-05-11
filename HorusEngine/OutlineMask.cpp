#include "OutlineMask.h"

namespace GFX::Visual
{
	OutlineMask::OutlineMask(Graphics& gfx, Data::ColorFloat3 color, std::shared_ptr<Data::VertexLayout> vertexLayout)
	{
		auto vertexShader = Resource::VertexShader::Get(gfx, "SolidVS.cso");
		AddBind(Resource::InputLayout::Get(gfx, vertexLayout, vertexShader->GetBytecode()));
		AddBind(std::move(vertexShader));
	}
}