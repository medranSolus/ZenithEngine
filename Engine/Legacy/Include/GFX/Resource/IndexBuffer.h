#pragma once
#include "GfxResPtr.h"

namespace ZE::GFX::Resource
{
	class IndexBuffer : public IBindable
	{
		U32 count;
		std::string name;
		Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	public:
		IndexBuffer(Graphics& gfx, const std::string& tag, const std::vector<U32>& indices);
		virtual ~IndexBuffer() = default;

		static bool NotStored(const std::string& tag) noexcept { return Codex::NotStored<IndexBuffer>(tag); }
		template<typename ...Ignore>
		static std::string GenerateRID(const std::string& tag, Ignore&& ...ignore) noexcept { return "I#" + tag; }

		static GfxResPtr<IndexBuffer> Get(Graphics& gfx, const std::string& tag, const std::vector<U32>& indices);

		constexpr U32 GetCount() const noexcept { return count; }

		void Bind(Graphics& gfx) const override { GetContext(gfx)->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0); }
		std::string GetRID() const noexcept override { return GenerateRID(name); }
	};

	template<>
	struct is_resolvable_by_codex<IndexBuffer>
	{
		static constexpr bool GENERATE_ID{ true };
	};
}