#pragma once
#include "ConstBufferEx.h"

namespace GFX::Resource
{
	class ConstBufferExGeometry : public ConstBufferEx
	{
		using ConstBufferEx::ConstBufferEx;
		using ConstBufferEx::GetContext;
		using ConstBufferEx::constantBuffer;
		using ConstBufferEx::rootLayout;
		using ConstBufferEx::name;
		using ConstBufferEx::slot;

	public:
		virtual ~ConstBufferExGeometry() = default;

		static inline std::shared_ptr<ConstBufferExGeometry> Get(Graphics& gfx, const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
			UINT slot = 0U, const Data::CBuffer::DynamicCBuffer* buffer = nullptr);

		static std::string GenerateRID(const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
			UINT slot = 0U, const Data::CBuffer::DynamicCBuffer* buffer = nullptr) noexcept;

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->GSSetConstantBuffers(slot, 1U, constantBuffer.GetAddressOf()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(name, rootLayout, slot, nullptr); }
	};

	template<>
	struct is_resolvable_by_codex<ConstBufferExGeometry>
	{
		static constexpr bool generate{ true };
	};

	std::shared_ptr<ConstBufferExGeometry> ConstBufferExGeometry::Get(Graphics& gfx, const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
		UINT slot, const Data::CBuffer::DynamicCBuffer* buffer)
	{
		return Codex::Resolve<ConstBufferExGeometry>(gfx, tag, root, slot, buffer);
	}
}