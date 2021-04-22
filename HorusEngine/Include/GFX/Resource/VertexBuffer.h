#pragma once
#include "GfxResPtr.h"
#include "GFX/Data/VertexBufferData.h"

namespace GFX::Resource
{
	class VertexBuffer : public IBindable
	{
		U32 stride;
		std::string name;
		Data::BoundingBox boundingBox;
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;

	public:
		VertexBuffer(Graphics& gfx, const std::string& tag, const Data::VertexBufferData& buffer);
		virtual ~VertexBuffer() = default;
		
		static bool NotStored(const std::string& tag) noexcept { return Codex::NotStored<VertexBuffer>(tag); }
		template<typename ...Ignore>
		static std::string GenerateRID(const std::string& tag, Ignore&& ...ignore) noexcept { return "V#" + tag; }

		static GfxResPtr<VertexBuffer> Get(Graphics& gfx, const std::string& tag, const Data::VertexBufferData& buffer);

		constexpr const Data::BoundingBox& GetBox() const noexcept { return boundingBox; }

		void Bind(Graphics& gfx) const override;
		std::string GetRID() const noexcept override { return GenerateRID(name); }
	};

	template<>
	struct is_resolvable_by_codex<VertexBuffer>
	{
		static constexpr bool GENERATE_ID{ true };
	};
}