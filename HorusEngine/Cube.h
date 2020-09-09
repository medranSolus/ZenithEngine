#pragma once
#include "IndexedTriangleList.h"

namespace GFX::Primitive
{
	class Cube
	{
	public:
		Cube() = delete;

		static constexpr size_t GetSkyboxIndexCount() noexcept { return GetSolidIndexCount(); }
		static constexpr size_t GetSolidIndexCount() noexcept { return 36U; }

		static inline std::string GetNameSkyboxVertexBuffer() noexcept { return GetNameSolid(); }
		static inline std::string GetNameSkyboxIndexBuffer() noexcept { return std::string(typeid(Cube).name()) + "IX"; }

		static std::string GetNameSolid(const std::vector<VertexAttribute>&& attributes = {}) noexcept;
		static std::string GetName(const std::vector<VertexAttribute>&& attributes = {}) noexcept;

		static inline std::shared_ptr<Data::VertexLayout> GetLayoutSkybox() noexcept { return std::make_shared<Data::VertexLayout>(); }
		static std::shared_ptr<Data::VertexLayout> GetLayoutSolid(const std::vector<VertexAttribute>&& attributes = {}) noexcept;
		static std::shared_ptr<Data::VertexLayout> GetLayout(const std::vector<VertexAttribute>&& attributes = {}) noexcept;

		static Data::VertexBufferData MakeSolidVertex(const std::vector<VertexAttribute>&& attributes = {}) noexcept;
		static std::vector<unsigned int> MakeSolidIndex() noexcept;
		static IndexedTriangleList MakeSolid(const std::vector<VertexAttribute>&& attributes = {}) noexcept;

		static inline Data::VertexBufferData MakeSkyboxVertex() noexcept { return std::move(MakeSolidVertex()); }
		static std::vector<unsigned int> MakeSkyboxIndex() noexcept;
		static IndexedTriangleList MakeSkybox() noexcept;

		static IndexedTriangleList Make(const std::vector<VertexAttribute>&& attributes = {}) noexcept;
	};
}