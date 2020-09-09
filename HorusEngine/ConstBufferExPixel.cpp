#include "ConstBufferExPixel.h"

namespace GFX::Resource
{
	std::string ConstBufferExPixel::GenerateRID(const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
		UINT slot, const Data::CBuffer::DynamicCBuffer* buffer) noexcept
	{
		return "#" + std::string(typeid(ConstBufferExPixel).name()) + "#" + tag + "#" + std::to_string(slot) + "#" + root.GetSignature() + "#";
	}
}