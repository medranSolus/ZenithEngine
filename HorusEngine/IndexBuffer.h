#pragma once
#include "GfxResPtr.h"

namespace GFX::Resource
{
	class IndexBuffer : public IBindable
	{
		unsigned int count;
		std::string name;
		Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	public:
		IndexBuffer(Graphics& gfx, const std::string& tag, const std::vector<unsigned int>& indices);
		virtual ~IndexBuffer() = default;

		static inline bool NotStored(const std::string& tag) noexcept { return Codex::NotStored<IndexBuffer>(tag); }
		static inline GfxResPtr<IndexBuffer> Get(Graphics& gfx, const std::string& tag, const std::vector<unsigned int>& indices);
		template<typename ...Ignore>
		static inline std::string GenerateRID(const std::string& tag, Ignore&& ...ignore) noexcept { return "IB#" + tag; }

		constexpr unsigned int GetCount() const noexcept { return count; }

		inline void Bind(Graphics& gfx) override { GetContext(gfx)->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0U); }
		inline std::string GetRID() const noexcept override { return GenerateRID(name); }
	};

	template<>
	struct is_resolvable_by_codex<IndexBuffer>
	{
		static constexpr bool generate{ true };
	};

	inline GfxResPtr<IndexBuffer> IndexBuffer::Get(Graphics& gfx, const std::string& tag, const std::vector<unsigned int>& indices)
	{
		return Codex::Resolve<IndexBuffer>(gfx, tag, indices);
	}
}