#include "ConstBufferExPixelCache.h"

namespace GFX::Resource
{
	inline std::string ConstBufferExPixelCache::GenerateRID(const std::string& tag,
		const Data::CBuffer::DCBLayoutElement& root, UINT slot) noexcept
	{
		return "#" + std::string(typeid(ConstBufferExPixelCache).name()) + "#" + tag + "#" + std::to_string(slot) + "#" + root.GetSignature() + "#";
	}

	std::shared_ptr<ConstBufferExPixelCache> ConstBufferExPixelCache::Get(Graphics& gfx, const std::string& tag,
		const Data::CBuffer::DCBLayoutFinal& layout, UINT slot)
	{
		return Codex::Resolve<ConstBufferExPixelCache>(gfx, tag, layout, slot);
	}

	std::shared_ptr<ConstBufferExPixelCache> ConstBufferExPixelCache::Get(Graphics& gfx, const std::string& tag,
		const Data::CBuffer::DynamicCBuffer& buffer, UINT slot)
	{
		return Codex::Resolve<ConstBufferExPixelCache>(gfx, tag, buffer, slot);
	}

	std::string ConstBufferExPixelCache::GenerateRID(const std::string& tag,
		const Data::CBuffer::DCBLayoutFinal& layout, UINT slot) noexcept
	{
		return GenerateRID(tag, *layout.GetRoot(), slot);
	}

	std::string ConstBufferExPixelCache::GenerateRID(const std::string& tag,
		const Data::CBuffer::DynamicCBuffer& buffer, UINT slot) noexcept
	{
		return GenerateRID(tag, buffer.GetRootElement(), slot);
	}

	void ConstBufferExPixelCache::Bind(Graphics& gfx) noexcept
	{
		if (dirty)
		{
			Update(gfx, buffer);
			dirty = false;
		}
		ConstBufferExPixel::Bind(gfx);
	}
}