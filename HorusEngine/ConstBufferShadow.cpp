#include "ConstBufferShadow.h"

namespace GFX::Resource
{
	void ConstBufferShadow::Update(Graphics& gfx)
	{
		assert(camera);
		const auto& pos = camera->GetPos();
		vertexBuffer->Update(gfx, DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(-pos.x, -pos.y, -pos.z)));
	}
}