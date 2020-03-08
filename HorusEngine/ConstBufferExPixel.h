#pragma once
#include "ConstBufferEx.h"

namespace GFX::Resource
{
	class ConstBufferExPixel : public ConstBufferEx
	{
		using ConstBufferEx::ConstBufferEx;
		using ConstBufferEx::GetContext;
		using ConstBufferEx::constantBuffer;
		using ConstBufferEx::rootLayout;
		using ConstBufferEx::name;
		using ConstBufferEx::slot;

	public:
		ConstBufferExPixel(const ConstBufferExPixel&) = delete;
		ConstBufferExPixel& operator=(const ConstBufferExPixel&) = delete;
		virtual ~ConstBufferExPixel() = default;

		static std::shared_ptr<ConstBufferExPixel> Get(Graphics& gfx, const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
			UINT slot = 0U, const Data::CBuffer::DynamicCBuffer* buffer = nullptr);

		static std::string GenerateRID(const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
			UINT slot = 0U, const Data::CBuffer::DynamicCBuffer* buffer = nullptr) noexcept;

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->PSSetConstantBuffers(slot, 1U, constantBuffer.GetAddressOf()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(name, rootLayout, slot, nullptr); }
	};

	template<>
	struct is_resolvable_by_codex<ConstBufferExPixel>
	{
		static constexpr bool value{ true };
	};
}