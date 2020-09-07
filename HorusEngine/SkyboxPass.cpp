#include "SkyboxPass.h"
#include "RenderPassesBase.h"
#include "PipelineResources.h"
#include "GfxResources.h"
#include "Cube.h"

namespace GFX::Pipeline::RenderPass
{
	SkyboxPass::SkyboxPass(Graphics& gfx, const std::string& name) : BindingPass(name)
	{
		AddBindableSink<GFX::Resource::TextureCube>("skyboxTexture");
		RegisterSink(Base::SinkDirectBuffer<Resource::RenderTarget>::Make("renderTarget", renderTarget));
		RegisterSink(Base::SinkDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));
		RegisterSource(Base::SourceDirectBuffer<Resource::RenderTarget>::Make("renderTarget", renderTarget));
		RegisterSource(Base::SourceDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));

		AddBind(GFX::Resource::Rasterizer::Get(gfx, true));
		AddBind(GFX::Resource::Sampler::Get(gfx, GFX::Resource::Sampler::Type::Linear, false));
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::DepthFirst));
		AddBind(GFX::Resource::PixelShader::Get(gfx, "SkyboxPS"));
		AddBind(std::make_shared<GFX::Resource::ConstBufferTransformSkybox>(gfx));
		AddBind(GFX::Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		auto model = Primitive::Cube::MakeSkybox();
		indexCount = static_cast<UINT>(model.indices.size());
		AddBind(GFX::Resource::VertexBuffer::Get(gfx, Primitive::Cube::GetNameSkyboxVertexBuffer(), model.vertices));
		AddBind(GFX::Resource::IndexBuffer::Get(gfx, Primitive::Cube::GetNameSkyboxIndexBuffer(), model.indices));
		auto vertexShader = GFX::Resource::VertexShader::Get(gfx, "SkyboxVS");
		AddBind(GFX::Resource::InputLayout::Get(gfx, model.vertices.GetLayout(), vertexShader));
		AddBind(std::move(vertexShader));
	}

	void SkyboxPass::Execute(Graphics& gfx) noexcept(!IS_DEBUG)
	{
		assert(mainCamera);
		mainCamera->Bind(gfx);
		BindAll(gfx);
		gfx.DrawIndexed(indexCount);
	}
}