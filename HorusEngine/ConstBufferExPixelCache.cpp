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

	template<>
	void ConstBufferExPixelCache::Set<DirectX::XMFLOAT4>(const std::string& propertyName, const DirectX::XMFLOAT4& value) noexcept(!IS_DEBUG)
	{
		if (DirectX::XMVector4NotEqual(DirectX::XMLoadFloat4(&value), DirectX::XMLoadFloat4(&buffer[propertyName])))
		{
			buffer[propertyName] = value;
			dirty = true;
		}
	}

	template<>
	void ConstBufferExPixelCache::Set<DirectX::XMFLOAT3>(const std::string& propertyName, const DirectX::XMFLOAT3& value) noexcept(!IS_DEBUG)
	{
		if (DirectX::XMVector3NotEqual(DirectX::XMLoadFloat3(&value), DirectX::XMLoadFloat3(&buffer[propertyName])))
		{
			buffer[propertyName] = value;
			dirty = true;
		}
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