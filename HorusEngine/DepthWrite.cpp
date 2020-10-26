#include "DepthWrite.h"
#include "GfxResources.h"

namespace GFX::Visual
{
	DepthWrite::DepthWrite(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout)
	{
		AddBind(Resource::InputLayout::Get(gfx, vertexLayout, Resource::VertexShader::Get(gfx, "SolidVS")));
	}
}