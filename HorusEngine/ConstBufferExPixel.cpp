#include "ConstBufferExPixel.h"

namespace GFX::Resource
{
	std::shared_ptr<ConstBufferExPixel> ConstBufferExPixel::Get(Graphics& gfx, const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
		UINT slot, const Data::CBuffer::DynamicCBuffer* buffer)
	{
		return Codex::Resolve<ConstBufferExPixel>(gfx, tag, root, slot, buffer);
	}

	std::string ConstBufferExPixel::GenerateRID(const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
		UINT slot, const Data::CBuffer::DynamicCBuffer* buffer) noexcept
	{
		return "#" + std::string(typeid(ConstBufferExPixel).name()) + "#" + tag + "#" + std::to_string(slot) + "#" + root.GetSignature() + "#";
	}
}