#include "SkyboxPass.h"
#include "RenderPassesBase.h"
#include "GfxResources.h"
#include "Cube.h"

namespace GFX::Pipeline::RenderPass
{
	SkyboxPass::SkyboxPass(Graphics& gfx, const std::string& name) : BindingPass(name)
	{
		AddBindableSink<GFX::Resource::TextureCube>("skyboxTexture");
		AddBindableSink<GFX::Resource::ConstBufferExPixelCache>("gammaCorrection");
		RegisterSink(Base::SinkDirectBuffer<Resource::IRenderTarget>::Make("renderTarget", renderTarget));
		RegisterSink(Base::SinkDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));

		RegisterSource(Base::SourceDirectBuffer<Resource::IRenderTarget>::Make("renderTarget", renderTarget));
		RegisterSource(Base::SourceDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));

		AddBind(GFX::Resource::Rasterizer::Get(gfx, D3D11_CULL_MODE::D3D11_CULL_BACK));
		AddBind(GFX::Resource::Sampler::Get(gfx, GFX::Resource::Sampler::Type::Linear, false));
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::DepthFirst));
		AddBind(GFX::Resource::PixelShader::Get(gfx, "SkyboxPS"));
		AddBind(std::make_shared<GFX::Resource::ConstBufferTransformSkybox>(gfx));
		AddBind(GFX::Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		const std::string vertexBufferTag = Primitive::Cube::GetNameSkyboxVertexBuffer();
		const std::string indexBufferTag = Primitive::Cube::GetNameSkyboxIndexBuffer();
		if (GFX::Resource::VertexBuffer::NotStored(vertexBufferTag))
		{
			auto vertices = Primitive::Cube::MakeSkyboxVertex();
			AddBind(GFX::Resource::VertexBuffer::Get(gfx, vertexBufferTag, vertices));
		}
		else
		{
			Data::VertexBufferData vertices;
			AddBind(GFX::Resource::VertexBuffer::Get(gfx, vertexBufferTag, vertices));
		}
		if (GFX::Resource::IndexBuffer::NotStored(indexBufferTag))
		{
			auto indices = Primitive::Cube::MakeSkyboxIndex();
			AddBind(GFX::Resource::IndexBuffer::Get(gfx, indexBufferTag, indices));
		}
		else
		{
			std::vector<unsigned int> indices;
			AddBind(GFX::Resource::IndexBuffer::Get(gfx, indexBufferTag, indices));
		}
		indexCount = static_cast<UINT>(Primitive::Cube::GetSkyboxIndexCount());

		auto vertexShader = GFX::Resource::VertexShader::Get(gfx, "SkyboxVS");
		AddBind(GFX::Resource::InputLayout::Get(gfx, Primitive::Cube::GetLayoutSkybox(), vertexShader));
		AddBind(std::move(vertexShader));
	}

	void SkyboxPass::Execute(Graphics& gfx)
	{
		assert(mainCamera);
		mainCamera->BindCamera(gfx);
		BindAll(gfx);
		gfx.DrawIndexed(indexCount);
	}
}