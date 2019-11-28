#pragma once
#include "IBindable.h"

namespace GFX::Resource
{
	class IndexBuffer : public IBindable
	{
	protected:
		unsigned int count;
		Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	public:
		IndexBuffer(Graphics & gfx, const std::vector<unsigned int> & indices);

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0U); }
		constexpr unsigned int GetCount() const noexcept { return count; }
	};
}
