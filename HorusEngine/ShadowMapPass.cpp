#include "ShadowMapPass.h"
#include "RenderPassesBase.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Pipeline::RenderPass
{
	ShadowMapPass::ShadowMapPass(Graphics& gfx, const std::string& name, const DirectX::XMMATRIX& projectionMatrix)
		: QueuePass(name)
	{
		AddBindableSink<GFX::Resource::IBindable>("shadowBias");

		RegisterSink(Base::SinkDirectBuffer<Resource::DepthStencil>::Make("shadowMapDepth", depthStencil));
		RegisterSink(Base::SinkDirectBuffer<Resource::IRenderTarget>::Make("shadowMapTarget", renderTarget));

		RegisterSource(Base::SourceDirectBuffer<Resource::DepthStencil>::Make("depth", depthStencil));
		RegisterSource(Base::SourceDirectBuffer<Resource::IRenderTarget>::Make("scratch", renderTarget));
		RegisterSource(Base::SourceDirectBindable<Resource::IRenderTarget>::Make("shadowMap", renderTarget));

		positionBuffer = GFX::Resource::ConstBufferPixel<DirectX::XMFLOAT4>::Get(gfx, "$shadowMapPass");

		AddBind(positionBuffer);
		AddBind(GFX::Resource::DepthStencilState::Get(gfx, GFX::Resource::DepthStencilState::StencilMode::Off));
		AddBind(GFX::Resource::Blender::Get(gfx, GFX::Resource::Blender::Type::None));
		AddBind(GFX::Resource::Rasterizer::Get(gfx, D3D11_CULL_MODE::D3D11_CULL_BACK, false));

		DirectX::XMStoreFloat4x4(&projection, projectionMatrix);
	}

	void ShadowMapPass::Execute(Graphics& gfx)
	{
		assert(mainCamera);
		assert(shadowSource);

		const auto& pos = shadowSource->GetPos();
		const DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&pos);
		positionBuffer->Update(gfx, { pos.x, pos.y, pos.z, 0.0f });
		mainCamera->BindVS(gfx);

		const DirectX::XMVECTOR direction = DirectX::XMLoadFloat3(&shadowSource->GetBuffer()["direction"]);
		const DirectX::XMVECTOR up = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
			Math::GetVectorRotation(DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f), direction));
		
		gfx.SetView(DirectX::XMMatrixLookAtLH(position, DirectX::XMVectorAdd(position, direction), up));
		gfx.SetProjection(DirectX::XMLoadFloat4x4(&projection));

		renderTarget->Clear(gfx, { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX });
		depthStencil->Clear(gfx);
		QueuePass::Execute(gfx);
	}
}