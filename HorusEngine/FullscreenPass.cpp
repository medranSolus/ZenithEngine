#include "FullscreenPass.h"
#include "GfxResources.h"
#include "Primitives.h"

namespace GFX::Pipeline::RenderPass::Base
{
	FullscreenPass::FullscreenPass(Graphics& gfx, const std::string& name, const std::string& vertexShaderName)
		: BindingPass(name)
	{
		const std::string typeName = Primitive::Square::GetNameNDC2D();
		if (GFX::Resource::VertexBuffer::NotStored(typeName) && GFX::Resource::IndexBuffer::NotStored(typeName))
		{
			auto list = Primitive::Square::MakeNDC2D();
			AddBind(GFX::Resource::VertexBuffer::Get(gfx, typeName, list.vertices));
			AddBind(GFX::Resource::IndexBuffer::Get(gfx, typeName, list.indices));
		}
		else
		{
			Primitive::IndexedTriangleList list;
			AddBind(GFX::Resource::VertexBuffer::Get(gfx, typeName, list.vertices));
			AddBind(GFX::Resource::IndexBuffer::Get(gfx, typeName, list.indices));
		}

		auto shaderVS = GFX::Resource::VertexShader::Get(gfx, vertexShaderName);
		AddBind(GFX::Resource::InputLayout::Get(gfx, Primitive::Square::GetLayoutNDC2D(), shaderVS));
		AddBind(std::move(shaderVS));
		AddBind(GFX::Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		AddBind(GFX::Resource::Rasterizer::Get(gfx, D3D11_CULL_MODE::D3D11_CULL_NONE, false));
	}

	void FullscreenPass::Execute(Graphics& gfx)
	{
		BindAll(gfx);
		gfx.DrawIndexed(6U);
	}
}