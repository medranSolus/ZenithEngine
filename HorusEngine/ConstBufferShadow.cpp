#include "ConstBufferShadow.h"

namespace GFX::Resource
{
	void ConstBufferShadow::Update(Graphics& gfx, Light::ILight& shadowSource)
	{
		const auto& pos = shadowSource.GetPos();
		vertexBuffer->Update(gfx, DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(-pos.x, -pos.y, -pos.z)));
	}
}