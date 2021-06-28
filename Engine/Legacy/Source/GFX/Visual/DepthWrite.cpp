#include "GFX/Visual/DepthWrite.h"
#include "GFX/Resource/InputLayout.h"

namespace ZE::GFX::Visual
{
	DepthWrite::DepthWrite(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout)
	{
		AddBind(Resource::InputLayout::Get(gfx, std::move(vertexLayout), Resource::VertexShader::Get(gfx, "SolidVS")));
	}
}