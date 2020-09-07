#pragma once
#include "VertexBuffer.h"

namespace GFX::Resource
{
	class ConstBufferTransformSkybox : public IBindable
	{
		std::unique_ptr<ConstBufferVertex<DirectX::XMMATRIX>> vertexBuffer;

	public:
		inline ConstBufferTransformSkybox(Graphics& gfx, UINT slot = 0U) : vertexBuffer(std::make_unique<ConstBufferVertex<DirectX::XMMATRIX>>(gfx, "", slot)) {}
		virtual ~ConstBufferTransformSkybox() = default;

		inline void Update(Graphics& gfx) { vertexBuffer->Update(gfx, DirectX::XMMatrixTranspose(gfx.GetCamera() * gfx.GetProjection())); }
		inline void Bind(Graphics& gfx) noexcept override { Update(gfx); vertexBuffer->Bind(gfx); }
		inline std::string GetRID() const noexcept override { return "?"; }
	};
}