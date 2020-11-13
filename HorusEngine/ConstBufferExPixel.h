#pragma once
#include "ConstBufferEx.h"
#include "GfxExceptionMacros.h"

namespace GFX::Resource
{
	class ConstBufferExPixel : public ConstBufferEx
	{
		using ConstBufferEx::GetContext;
		using ConstBufferEx::constantBuffer;
		using ConstBufferEx::rootLayout;
		using ConstBufferEx::name;
		using ConstBufferEx::slot;

	public:
		inline ConstBufferExPixel(Graphics& gfx, const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
			UINT slot = 0U, const Data::CBuffer::DynamicCBuffer* buffer = nullptr, bool debugName = true);
		virtual ~ConstBufferExPixel() = default;

		static inline GfxResPtr<ConstBufferExPixel> Get(Graphics& gfx, const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
			UINT slot = 0U, const Data::CBuffer::DynamicCBuffer* buffer = nullptr);

		static inline std::string GenerateRID(const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
			UINT slot = 0U, const Data::CBuffer::DynamicCBuffer* buffer = nullptr) noexcept;

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->PSSetConstantBuffers(slot, 1U, constantBuffer.GetAddressOf()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(name, rootLayout, slot, nullptr); }
	};

	template<>
	struct is_resolvable_by_codex<ConstBufferExPixel>
	{
		static constexpr bool generate{ true };
	};

	inline ConstBufferExPixel::ConstBufferExPixel(Graphics& gfx, const std::string& tag,
		const Data::CBuffer::DCBLayoutElement& root, UINT slot,
		const Data::CBuffer::DynamicCBuffer* buffer, bool debugName)
		: ConstBufferEx(gfx, tag, root, slot, buffer)
	{
#ifdef _DEBUG
		if (debugName)
		{
			GFX_ENABLE_ALL(gfx);
			SET_DEBUG_NAME_RID(constantBuffer.Get());
		}
#endif
	}

	inline GfxResPtr<ConstBufferExPixel> ConstBufferExPixel::Get(Graphics& gfx, const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
		UINT slot, const Data::CBuffer::DynamicCBuffer* buffer)
	{
		return Codex::Resolve<ConstBufferExPixel>(gfx, tag, root, slot, buffer);
	}

	inline std::string ConstBufferExPixel::GenerateRID(const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
		UINT slot, const Data::CBuffer::DynamicCBuffer* buffer) noexcept
	{
		return "CEP" + std::to_string(slot) + root.GetSignature() + "#" + tag;
	}
}