#pragma once
#include "ConstBufferVertex.h"
#include "ICamera.h"

namespace GFX::Resource
{
	class ConstBufferShadow : public IBindable
	{
		std::unique_ptr<ConstBufferVertex<DirectX::XMMATRIX>> vertexBuffer;
		Camera::ICamera* camera = nullptr;

	public:
		inline ConstBufferShadow(Graphics& gfx, UINT slot = 1U) : vertexBuffer(std::make_unique<ConstBufferVertex<DirectX::XMMATRIX>>(gfx, "", slot)) {}
		virtual ~ConstBufferShadow() = default;

		inline void SetCamera(Camera::ICamera& shadowCamera) noexcept { camera = &shadowCamera; }
		inline void Update(Graphics& gfx) { assert(camera); vertexBuffer->Update(gfx, DirectX::XMMatrixTranspose(camera->GetView() * camera->GetProjection())); }
		inline void Bind(Graphics& gfx) noexcept override { vertexBuffer->Bind(gfx); }
		inline std::string GetRID() const noexcept override { return "?"; }
	};
}