#include "GFX/Pipeline/RenderPass/Base/FullscreenPass.h"
#include "GFX/Resource/GfxResources.h"
#include "GFX/Primitive/Square.h"

namespace ZE::GFX::Pipeline::RenderPass::Base
{
	FullscreenPass::FullscreenPass(Graphics& gfx, std::string&& name, const std::string& vertexShaderName)
		: RenderPass(std::forward<std::string>(name))
	{
		const std::string typeName = Primitive::Square::GetNameNDC2D();
		if (GFX::Resource::VertexBuffer::NotStored(typeName) && GFX::Resource::IndexBuffer::NotStored(typeName))
		{
			auto list = Primitive::Square::MakeNDC2D();
			AddBind(GFX::Resource::VertexBuffer::Get(gfx, typeName, list.Vertices));
			AddBind(GFX::Resource::IndexBuffer::Get(gfx, typeName, list.Indices));
		}
		else
		{
			Primitive::IndexedTriangleList list;
			AddBind(GFX::Resource::VertexBuffer::Get(gfx, typeName, list.Vertices));
			AddBind(GFX::Resource::IndexBuffer::Get(gfx, typeName, list.Indices));
		}

		auto shaderVS = GFX::Resource::VertexShader::Get(gfx, vertexShaderName);
		AddBind(GFX::Resource::InputLayout::Get(gfx, Primitive::Square::GetLayoutNDC2D(), shaderVS));
		AddBind(std::move(shaderVS));
		AddBind(GFX::Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		AddBind(GFX::Resource::Rasterizer::Get(gfx, D3D11_CULL_NONE, false));
	}

	void FullscreenPass::Execute(Graphics& gfx)
	{
		ZE_DRAW_TAG_START(gfx, GetName());
		BindAll(gfx);
		gfx.DrawIndexed(6);
		ZE_DRAW_TAG_END(gfx);
	}
}