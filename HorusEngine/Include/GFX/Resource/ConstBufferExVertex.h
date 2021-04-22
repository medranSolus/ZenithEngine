#pragma once
#include "ConstBufferEx.h"

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
		ConstBufferExVertex(Graphics& gfx, const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
			U32 slot = 0, const Data::CBuffer::DynamicCBuffer* buffer = nullptr, bool debugName = true);
		virtual ~ConstBufferExVertex() = default;

		static std::string GenerateRID(const std::string & tag, const Data::CBuffer::DCBLayoutElement & root,
			U32 slot = 0, const Data::CBuffer::DynamicCBuffer * buffer = nullptr) noexcept;

		static GfxResPtr<ConstBufferExVertex> Get(Graphics& gfx, const std::string& tag, const Data::CBuffer::DCBLayoutElement& root,
			U32 slot = 0, const Data::CBuffer::DynamicCBuffer* buffer = nullptr);

		void Bind(Graphics& gfx) const override { GetContext(gfx)->VSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf()); }
		std::string GetRID() const noexcept override { return GenerateRID(name, rootLayout, slot); }
	};

	template<>
	struct is_resolvable_by_codex<ConstBufferExVertex>
	{
		static constexpr bool GENERATE_ID{ true };
	};
}