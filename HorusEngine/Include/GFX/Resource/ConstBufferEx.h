#pragma once
#include "GfxResPtr.h"

namespace GFX::Resource
{
	class ConstBufferEx : public IBindable
	{
	protected:
		U32 slot;
		std::string name;
		const Data::CBuffer::DCBLayoutElement& rootLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

	public:
		ConstBufferEx(Graphics& gfx, const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
			U32 slot = 0, const Data::CBuffer::DynamicCBuffer* buffer = nullptr);
		virtual ~ConstBufferEx() = default;

		void Update(Graphics& gfx, const Data::CBuffer::DynamicCBuffer& buffer) const;
	};
}