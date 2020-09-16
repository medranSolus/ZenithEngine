#include "DepthWrite.h"

namespace GFX::Visual
{
	DepthWrite::DepthWrite(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout)
	{
		auto vertexShader = Resource::VertexShader::Get(gfx, "SolidVS");
		AddBind(Resource::InputLayout::Get(gfx, vertexLayout, vertexShader));
		AddBind(std::move(vertexShader));
	}
}