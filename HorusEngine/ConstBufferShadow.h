#pragma once
#include "ConstBufferVertex.h"
#include "ILight.h"

namespace GFX::Resource
{
	class ConstBufferShadow : public IBindable
	{
		std::unique_ptr<ConstBufferVertex<DirectX::XMMATRIX>> vertexBuffer;
		Light::ILight* light = nullptr;

	public:
		inline ConstBufferShadow(Graphics& gfx, UINT slot = 1U) : vertexBuffer(std::make_unique<ConstBufferVertex<DirectX::XMMATRIX>>(gfx, "", slot)) {}
		virtual ~ConstBufferShadow() = default;

		constexpr void SetLight(Light::ILight& shadowSource) noexcept { light = &shadowSource; }
		inline void Bind(Graphics& gfx) override { vertexBuffer->Bind(gfx); }
		inline std::string GetRID() const noexcept override { return "?"; }

		void Update(Graphics& gfx);
	};
}