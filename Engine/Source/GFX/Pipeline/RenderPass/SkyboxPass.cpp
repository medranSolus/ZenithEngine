#include "GFX/Pipeline/RenderPass/SkyboxPass.h"
#include "GFX/Pipeline/RenderPass/Base/RenderPassesBase.h"
#include "GFX/Resource/GfxResources.h"
#include "GFX/Primitive/Cube.h"

namespace ZE::GFX::Pipeline::RenderPass
{
	SkyboxPass::SkyboxPass(Graphics& gfx, std::string&& name)
		: BindingPass(std::forward<std::string>(name))
	{
		AddBindableSink<GFX::Resource::TextureCube>("skyboxTexture");
		AddBindableSink<GFX::Resource::ConstBufferExPixelCache>("gammaCorrection");
		RegisterSink(Base::SinkDirectBuffer<Resource::IRenderTarget>::Make("renderTarget", renderTarget));
		RegisterSink(Base::SinkDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));

		RegisterSource(Base::SourceDirectBuffer<Resource::IRenderTarget>::Make("renderTarget", renderTarget));
		RegisterSource(Base::SourceDirectBuffer<Resource::DepthStencil>::Make("depthStencil", depthStencil));

		AddBind(GFX::Resource::Rasterizer::Get(gfx, D3D11_CULL_BACK));
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::DepthFirst));
		AddBind(GFX::Resource::PixelShader::Get(gfx, "SkyboxPS"));
		AddBind(GfxResPtr<GFX::Resource::ConstBufferTransformSkybox>(gfx));
		AddBind(GFX::Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

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
			std::vector<U32> indices;
			AddBind(GFX::Resource::IndexBuffer::Get(gfx, indexBufferTag, indices));
		}
		indexCount = static_cast<U32>(Primitive::Cube::GetSkyboxIndexCount());

		auto vertexShader = GFX::Resource::VertexShader::Get(gfx, "SkyboxVS");
		AddBind(GFX::Resource::InputLayout::Get(gfx, Primitive::Cube::GetLayoutSkybox(), vertexShader));
		AddBind(std::move(vertexShader));
	}

	void SkyboxPass::Execute(Graphics& gfx)
	{
		assert(mainCamera);
		ZE_DRAW_TAG_START(gfx, GetName());
		mainCamera->BindCamera(gfx);
		BindAll(gfx);
		gfx.DrawIndexed(indexCount);
		ZE_DRAW_TAG_END(gfx);
	}
}