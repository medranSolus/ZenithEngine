#include "ConstBufferShadow.h"

namespace GFX::Resource
{
	void ConstBufferShadow::Update(Graphics& gfx)
	{
		assert(light);
		const auto& pos = light->GetPos();
		vertexBuffer->Update(gfx, DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(-pos.x, -pos.y, -pos.z)));
	}
}