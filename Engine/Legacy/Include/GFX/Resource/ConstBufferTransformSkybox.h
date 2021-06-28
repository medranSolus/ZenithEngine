#pragma once
#include "VertexBuffer.h"

namespace ZE::GFX::Resource
{
	class ConstBufferTransformSkybox : public IBindable
	{
		mutable ConstBufferVertex<Matrix> vertexBuffer;

	public:
		ConstBufferTransformSkybox(Graphics& gfx, U32 slot = 0) : vertexBuffer(gfx, "", slot) {}
		virtual ~ConstBufferTransformSkybox() = default;

		void Update(Graphics& gfx) const { vertexBuffer.Update(gfx, Math::XMMatrixTranspose(gfx.GetView() * gfx.GetProjection())); }
		void Bind(Graphics& gfx) const override { Update(gfx); vertexBuffer.Bind(gfx); }
	};
}