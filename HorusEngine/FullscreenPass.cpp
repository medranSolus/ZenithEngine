#include "FullscreenPass.h"
#include "GfxResources.h"

namespace GFX::Pipeline::RenderPass::Base
{
	FullscreenPass::FullscreenPass(Graphics& gfx, const std::string& name, const std::string& vertexShaderName) : BindingPass(name)
	{
		// Fullscreen geometry (2 triangles)
		auto layout = std::make_shared<Data::VertexLayout>(false);
		layout->Append(VertexAttribute::Position2D);
		Data::VertexBufferData vBuffer(layout);
		// NDC rectangle
		vBuffer.EmplaceBack(DirectX::XMFLOAT2(-1.0f, 1.0f));
		vBuffer.EmplaceBack(DirectX::XMFLOAT2(1.0f, 1.0f));
		vBuffer.EmplaceBack(DirectX::XMFLOAT2(-1.0f, -1.0f));
		vBuffer.EmplaceBack(DirectX::XMFLOAT2(1.0f, -1.0f));
		AddBind(GFX::Resource::VertexBuffer::Get(gfx, "$Fullscreen", std::move(vBuffer)));
		AddBind(GFX::Resource::IndexBuffer::Get(gfx, "$Fullscreen", { 0, 1, 2, 1, 3, 2 }));
		// Other needed bindables
		auto shaderVS = GFX::Resource::VertexShader::Get(gfx, vertexShaderName);
		AddBind(GFX::Resource::InputLayout::Get(gfx, layout, shaderVS));
		AddBind(std::move(shaderVS));
		AddBind(GFX::Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		AddBind(GFX::Resource::Rasterizer::Get(gfx, false));
	}

	void FullscreenPass::Execute(Graphics& gfx)
	{
		BindAll(gfx);
		gfx.DrawIndexed(6U);
	}
}