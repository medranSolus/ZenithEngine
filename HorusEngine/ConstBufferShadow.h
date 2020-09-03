#pragma once
#include "ConstBufferPixel.h"
#include "ICamera.h"

namespace GFX::Resource
{
	class ConstBufferShadow : public IBindable
	{
		std::unique_ptr<ConstBufferPixel<DirectX::XMMATRIX>> pixelBuffer;
		Camera::ICamera* camera = nullptr;

	public:
		inline ConstBufferShadow(Graphics& gfx, UINT slot = 2U) : pixelBuffer(std::make_unique<ConstBufferPixel<DirectX::XMMATRIX>>(gfx, "", slot)) {}
		ConstBufferShadow(const ConstBufferShadow&) = delete;
		ConstBufferShadow& operator=(const ConstBufferShadow&) = delete;
		virtual ~ConstBufferShadow() = default;

		inline void SetCamera(Camera::ICamera& shadowCamera) noexcept { camera = &shadowCamera; }
		inline void Update(Graphics& gfx) { assert(camera); pixelBuffer->Update(gfx, DirectX::XMMatrixTranspose(camera->GetView() * camera->GetProjection())); }
		inline void Bind(Graphics& gfx) noexcept override { pixelBuffer->Bind(gfx); }
		inline std::string GetRID() const noexcept override { return "?"; }
	};
}