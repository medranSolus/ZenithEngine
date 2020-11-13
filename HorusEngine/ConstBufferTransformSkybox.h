#pragma once
#include "VertexBuffer.h"

namespace GFX::Resource
{
	class ConstBufferTransformSkybox : public IBindable
	{
		ConstBufferVertex<DirectX::XMMATRIX> vertexBuffer;

	public:
		inline ConstBufferTransformSkybox(Graphics& gfx, UINT slot = 0U) : vertexBuffer(gfx, "", slot) {}
		virtual ~ConstBufferTransformSkybox() = default;

		inline void Update(Graphics& gfx) { vertexBuffer.Update(gfx, DirectX::XMMatrixTranspose(gfx.GetView() * gfx.GetProjection())); }
		inline void Bind(Graphics& gfx) override { Update(gfx); vertexBuffer.Bind(gfx); }
		inline std::string GetRID() const noexcept override { return IBindable::GetNoCodexRID(); }
	};
}