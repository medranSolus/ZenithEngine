#pragma once
#include "ConstBufferEx.h"

namespace ZE::GFX::Resource
{
	class ConstBufferExGeometry : public ConstBufferEx
	{
		using ConstBufferEx::GetContext;
		using ConstBufferEx::constantBuffer;
		using ConstBufferEx::rootLayout;
		using ConstBufferEx::name;
		using ConstBufferEx::slot;

	public:
		ConstBufferExGeometry(Graphics& gfx, const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
			U32 slot = 0, const Data::CBuffer::DynamicCBuffer* buffer = nullptr, bool debugName = true);
		virtual ~ConstBufferExGeometry() = default;

		static std::string GenerateRID(const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
			U32 slot = 0, const Data::CBuffer::DynamicCBuffer* buffer = nullptr) noexcept;

		static GfxResPtr<ConstBufferExGeometry> Get(Graphics& gfx, const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
			U32 slot = 0, const Data::CBuffer::DynamicCBuffer* buffer = nullptr);

		void Bind(Graphics& gfx) const override { GetContext(gfx)->GSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf()); }
		std::string GetRID() const noexcept override { return GenerateRID(name, rootLayout, slot); }
	};

	template<>
	struct is_resolvable_by_codex<ConstBufferExGeometry>
	{
		static constexpr bool GENERATE_ID{ true };
	};
}