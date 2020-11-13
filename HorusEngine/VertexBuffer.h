#pragma once
#include "GfxResPtr.h"
#include "VertexBufferData.h"

namespace GFX::Resource
{
	class VertexBuffer : public IBindable
	{
		UINT stride;
		std::string name;
		Data::BoundingBox boundingBox;
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;

	public:
		VertexBuffer(Graphics& gfx, const std::string& tag, const Data::VertexBufferData& buffer);
		virtual ~VertexBuffer() = default;

		static inline bool NotStored(const std::string& tag) noexcept { return Codex::NotStored<VertexBuffer>(tag); }
		static inline GfxResPtr<VertexBuffer> Get(Graphics& gfx, const std::string& tag, const Data::VertexBufferData& buffer);
		template<typename ...Ignore>
		static inline std::string GenerateRID(const std::string& tag, Ignore&& ...ignore) noexcept { return "VB#" + tag; }

		constexpr const Data::BoundingBox& GetBox() const noexcept { return boundingBox; }
		inline void Bind(Graphics& gfx) override;
		inline std::string GetRID() const noexcept override { return GenerateRID(name); }
	};

	template<>
	struct is_resolvable_by_codex<VertexBuffer>
	{
		static constexpr bool generate{ true };
	};

	inline GfxResPtr<VertexBuffer> VertexBuffer::Get(Graphics& gfx, const std::string& tag, const Data::VertexBufferData& buffer)
	{
		return Codex::Resolve<VertexBuffer>(gfx, tag, buffer);
	}

	inline void VertexBuffer::Bind(Graphics& gfx)
	{
		const UINT offset = 0U;
		GetContext(gfx)->IASetVertexBuffers(0U, 1U, vertexBuffer.GetAddressOf(), &stride, &offset);
	}
}