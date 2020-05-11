#include "OutlineWrite.h"

namespace GFX::Visual
{
	OutlineWrite::OutlineWrite(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout)
	{
		auto vertexShader = Resource::VertexShader::Get(gfx, "SolidVS.cso");
		AddBind(Resource::InputLayout::Get(gfx, vertexLayout, vertexShader->GetBytecode()));
		AddBind(std::move(vertexShader));
	}
}