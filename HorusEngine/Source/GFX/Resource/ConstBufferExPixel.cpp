#include "GFX/Resource/ConstBufferExPixel.h"
#include "GFX/Resource/GfxDebugName.h"
#include "GFX/Graphics.h"

namespace GFX::Resource
{
	ConstBufferExPixel::ConstBufferExPixel(Graphics& gfx, const std::string& tag,
		const Data::CBuffer::DCBLayoutElement& root, U32 slot,
		const Data::CBuffer::DynamicCBuffer* buffer, bool debugName)
		: ConstBufferEx(gfx, tag, root, slot, buffer)
	{
#ifdef _MODE_DEBUG
		if (debugName)
		{
			GFX_ENABLE_ALL(gfx);
			GFX_SET_RID(constantBuffer.Get());
		}
#endif
	}

	std::string ConstBufferExPixel::GenerateRID(const std::string& tag,
		const Data::CBuffer::DCBLayoutElement& root, U32 slot,
		const Data::CBuffer::DynamicCBuffer* buffer) noexcept
	{
		return "E" + std::to_string(slot) + "P" + std::to_string(root.GetByteSize()) + "#" + tag;
	}

	GfxResPtr<ConstBufferExPixel> ConstBufferExPixel::Get(Graphics& gfx, const std::string& tag,
		const Data::CBuffer::DCBLayoutElement& root, U32 slot,
		const Data::CBuffer::DynamicCBuffer* buffer)
	{
		return Codex::Resolve<ConstBufferExPixel>(gfx, tag, root, slot, buffer);
	}
}