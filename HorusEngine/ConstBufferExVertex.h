#pragma once
#include "ConstBufferEx.h"
#include "GfxExceptionMacros.h"

namespace GFX::Resource
{
	class ConstBufferExVertex : public ConstBufferEx
	{
		using ConstBufferEx::GetContext;
		using ConstBufferEx::constantBuffer;
		using ConstBufferEx::rootLayout;
		using ConstBufferEx::name;
		using ConstBufferEx::slot;

	public:
		inline ConstBufferExVertex(Graphics& gfx, const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
			UINT slot = 0U, const Data::CBuffer::DynamicCBuffer* buffer = nullptr, bool debugName = true);
		virtual ~ConstBufferExVertex() = default;

		static inline GfxResPtr<ConstBufferExVertex> Get(Graphics& gfx, const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
			UINT slot = 0U, const Data::CBuffer::DynamicCBuffer* buffer = nullptr);

		static inline std::string GenerateRID(const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
			UINT slot = 0U, const Data::CBuffer::DynamicCBuffer* buffer = nullptr) noexcept;

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->VSSetConstantBuffers(slot, 1U, constantBuffer.GetAddressOf()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(name, rootLayout, slot, nullptr); }
	};

	template<>
	struct is_resolvable_by_codex<ConstBufferExVertex>
	{
		static constexpr bool generate{ true };
	};

	inline ConstBufferExVertex::ConstBufferExVertex(Graphics& gfx, const std::string& tag,
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

	inline GfxResPtr<ConstBufferExVertex> ConstBufferExVertex::Get(Graphics& gfx, const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
		UINT slot, const Data::CBuffer::DynamicCBuffer* buffer)
	{
		return Codex::Resolve<ConstBufferExVertex>(gfx, tag, root, slot, buffer);
	}

	inline std::string ConstBufferExVertex::GenerateRID(const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
		UINT slot, const Data::CBuffer::DynamicCBuffer* buffer) noexcept
	{
		return "CEV" + std::to_string(slot) + root.GetSignature() + "#" + tag;
	}
}