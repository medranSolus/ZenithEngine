#pragma once
#include "IndexedTriangleList.h"

namespace GFX::Primitive
{
	class Cube
	{
	public:
		Cube() = delete;

		static constexpr U64 GetSkyboxIndexCount() noexcept { return GetSolidIndexCount(); }
		static constexpr U64 GetSolidIndexCount() noexcept { return 36; }

		static std::string GetNameSkyboxVertexBuffer() noexcept { return GetNameSolid(); }
		static std::string GetNameSkyboxIndexBuffer() noexcept { return "QB"; }

		static std::string GetNameSolid(const std::vector<VertexAttribute>& attributes = {}) noexcept;
		static std::string GetName(const std::vector<VertexAttribute>& attributes = {}) noexcept;

		static std::shared_ptr<Data::VertexLayout> GetLayoutSkybox() noexcept { return std::make_shared<Data::VertexLayout>(); }
		static std::shared_ptr<Data::VertexLayout> GetLayoutSolid(const std::vector<VertexAttribute>& attributes = {}) noexcept;
		static std::shared_ptr<Data::VertexLayout> GetLayout(const std::vector<VertexAttribute>& attributes = {}) noexcept;

		static Data::VertexBufferData MakeSolidVertex(const std::vector<VertexAttribute>& attributes = {}) noexcept;
		static std::vector<U32> MakeSolidIndex() noexcept;
		static IndexedTriangleList MakeSolid(const std::vector<VertexAttribute>& attributes = {}) noexcept;

		static Data::VertexBufferData MakeSkyboxVertex() noexcept { return MakeSolidVertex(); }
		static std::vector<U32> MakeSkyboxIndex() noexcept;
		static IndexedTriangleList MakeSkybox() noexcept;

		static IndexedTriangleList Make(const std::vector<VertexAttribute>& attributes = {}) noexcept;
	};
}