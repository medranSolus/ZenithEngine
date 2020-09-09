#include "ConstBufferExVertex.h"

namespace GFX::Resource
{
	std::string ConstBufferExVertex::GenerateRID(const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
		UINT slot, const Data::CBuffer::DynamicCBuffer* buffer) noexcept
	{
		return "#" + std::string(typeid(ConstBufferExVertex).name()) + "#" + tag + "#" + std::to_string(slot) + "#" + root.GetSignature() + "#";
	}
}