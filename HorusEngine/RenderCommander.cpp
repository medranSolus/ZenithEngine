#include "RenderCommander.h"

namespace GFX::Pipeline
{
	RenderCommander::RenderCommander(Graphics& gfx)
		: depthStencil(gfx, gfx.GetWidth(), gfx.GetHeight()), target(gfx, gfx.GetWidth(), gfx.GetHeight())
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
		fullscreenVertexBuffer = Resource::VertexBuffer::Get(gfx, "$Fullscreen", std::move(vBuffer));
		fullscreenIndexBuffer = Resource::IndexBuffer::Get(gfx, "$Fullscreen", { 0, 1, 2, 1, 3, 2 });
		fullscreenVS = Resource::VertexShader::Get(gfx, "FullscreenVS.cso");
		fullscreenInputLayout = Resource::InputLayout::Get(gfx, layout, fullscreenVS->GetBytecode());
		// Negative effect
		fullscreenPS = Resource::PixelShader::Get(gfx, "FullscreenPS.cso");
	}

	void RenderCommander::Render(Graphics& gfx) noexcept(!IS_DEBUG)
	{
		// Draw to texture
		depthStencil.Clear(gfx);
		target.BindTarget(gfx, depthStencil);

		// Classic render pass
		Resource::DepthStencilState::Get(gfx, Resource::DepthStencilState::StencilMode::Off)->Bind(gfx);
		passes.at(0).Execute(gfx);
		// Draw to stencil buffer
		Resource::DepthStencilState::Get(gfx, Resource::DepthStencilState::StencilMode::Write)->Bind(gfx);
		Resource::NullPixelShader::Get(gfx)->Bind(gfx);
		passes.at(1).Execute(gfx);
		// Mask render target with stencil buffer
		Resource::DepthStencilState::Get(gfx, Resource::DepthStencilState::StencilMode::Mask)->Bind(gfx);
		Resource::PixelShader::Get(gfx, "SolidPS.cso")->Bind(gfx);
		passes.at(2).Execute(gfx);

		// Draw previous texture to screen with effect
		gfx.BindSwapBuffer();
		target.BindTexture(gfx, 0U);
		fullscreenVertexBuffer->Bind(gfx);
		fullscreenIndexBuffer->Bind(gfx);
		fullscreenVS->Bind(gfx);
		fullscreenInputLayout->Bind(gfx);
		fullscreenPS->Bind(gfx);
		gfx.DrawIndexed(fullscreenIndexBuffer->GetCount());
	}

	void RenderCommander::Reset() noexcept
	{
		for (auto& pass : passes)
			pass.Reset();
	}
}