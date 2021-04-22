#include "GFX/Pipeline/RenderPass/ShadowMapPass.h"
#include "GFX/Pipeline/RenderPass/Base/RenderPassesBase.h"
#include "GFX/Resource/GfxResources.h"

namespace GFX::Pipeline::RenderPass
{
	ShadowMapPass::ShadowMapPass(Graphics& gfx, std::string&& name, const Matrix& projectionMatrix)
		: BindingPass(std::forward<std::string>(name)), QueuePass(std::forward<std::string>(name))
	{
		AddBindableSink<GFX::Resource::IBindable>("shadowBias");

		RegisterSink(Base::SinkDirectBuffer<Resource::DepthStencil>::Make("shadowMapDepth", depthStencil));
		RegisterSink(Base::SinkDirectBuffer<Resource::IRenderTarget>::Make("shadowMapTarget", renderTarget));

		RegisterSource(Base::SourceDirectBuffer<Resource::DepthStencil>::Make("depth", depthStencil));
		RegisterSource(Base::SourceDirectBuffer<Resource::IRenderTarget>::Make("scratch", renderTarget));
		RegisterSource(Base::SourceDirectBindable<Resource::IRenderTarget>::Make("shadowMap", renderTarget));

		positionBuffer = GFX::Resource::ConstBufferPixel<Float4>::Get(gfx, "$SM");

		AddBind(positionBuffer);
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::Off));
		AddBind(GFX::Resource::Blender::Get(gfx, GFX::Resource::Blender::Type::None));
		AddBind(GFX::Resource::Rasterizer::Get(gfx, D3D11_CULL_BACK, false));

		Math::XMStoreFloat4x4(&projection, projectionMatrix);
	}

	void ShadowMapPass::Execute(Graphics& gfx)
	{
		assert(mainCamera);
		assert(shadowSource);

		const auto& pos = shadowSource->GetPos();
		const Vector position = Math::XMLoadFloat3(&pos);
		positionBuffer->Update(gfx, { pos.x, pos.y, pos.z, 0.0f });
		mainCamera->BindVS(gfx);

		const Vector direction = Math::XMLoadFloat3(&shadowSource->GetBuffer()["direction"]);
		const Vector up = Math::XMVector3TransformNormal(Math::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
			Math::GetVectorRotation(Math::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f), direction));

		gfx.SetView(Math::XMMatrixLookAtLH(position, Math::XMVectorAdd(position, direction), up));
		gfx.SetProjection(Math::XMLoadFloat4x4(&projection));

		renderTarget->Clear(gfx, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX });
		depthStencil->Clear(gfx);
		QueuePass::Execute(gfx);
	}
}