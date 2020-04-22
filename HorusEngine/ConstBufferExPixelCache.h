#pragma once
#include "ConstBufferExPixel.h"

namespace GFX::Resource
{
	class ConstBufferExPixelCache : public ConstBufferExPixel
	{
		using ConstBufferEx::GetContext;
		using ConstBufferEx::constantBuffer;
		using ConstBufferEx::rootLayout;
		using ConstBufferEx::name;
		using ConstBufferEx::slot;

		bool dirty = false;
		Data::CBuffer::DynamicCBuffer buffer;

		static inline std::string GenerateRID(const std::string& tag,
			const Data::CBuffer::DCBLayoutElement& root, UINT slot = 0U) noexcept;

	public:
		ConstBufferExPixelCache(Graphics& gfx, const std::string& tag,
			const Data::CBuffer::DCBLayoutFinal& layout, UINT slot = 0U)
			: ConstBufferExPixel(gfx, tag, *layout.GetRoot(), slot, nullptr), buffer(layout) {}
		ConstBufferExPixelCache(Graphics& gfx, const std::string& tag,
			const Data::CBuffer::DynamicCBuffer& buffer, UINT slot = 0U)
			: ConstBufferExPixel(gfx, tag, buffer.GetRootElement(), slot, &buffer), buffer(buffer) {}
		virtual ~ConstBufferExPixelCache() = default;

		static std::shared_ptr<ConstBufferExPixelCache> Get(Graphics& gfx, const std::string& tag,
			const Data::CBuffer::DCBLayoutFinal& layout, UINT slot = 0U);
		static std::shared_ptr<ConstBufferExPixelCache> Get(Graphics& gfx, const std::string& tag,
			const Data::CBuffer::DynamicCBuffer& buffer, UINT slot = 0U);

		static std::string GenerateRID(const std::string& tag,
			const Data::CBuffer::DCBLayoutFinal& layout, UINT slot = 0U) noexcept;
		static std::string GenerateRID(const std::string& tag,
			const Data::CBuffer::DynamicCBuffer& buffer, UINT slot = 0U) noexcept;

		void Bind(Graphics& gfx) noexcept override;
		inline std::string GetRID() const noexcept override { return GenerateRID(name, buffer, slot); }
	};

	template<>
	struct is_resolvable_by_codex<ConstBufferExPixelCache>
	{
		static constexpr bool generate{ true };
	};
}