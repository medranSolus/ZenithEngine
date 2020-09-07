#pragma once
#include "IndexedTriangleList.h"

namespace GFX::Primitive
{
	class Cube
	{
	public:
		Cube() = delete;

		static inline std::string GetNameSkyboxVertexBuffer() noexcept { return GetNameSolid(); }
		static inline std::string GetNameSkyboxIndexBuffer() noexcept { return std::string(typeid(Cube).name()) + "IX"; }

		static std::string GetNameSolid(const std::vector<VertexAttribute>&& attributes = {}) noexcept;
		static std::string GetName(const std::vector<VertexAttribute>&& attributes = {}) noexcept;

		static inline std::shared_ptr<Data::VertexLayout> GetLayoutSkybox() noexcept { return std::make_shared<Data::VertexLayout>(); }
		static std::shared_ptr<Data::VertexLayout> GetLayoutSolid(const std::vector<VertexAttribute>&& attributes = {}) noexcept;
		static std::shared_ptr<Data::VertexLayout> GetLayout(const std::vector<VertexAttribute>&& attributes = {}) noexcept;

		static IndexedTriangleList MakeSkybox();
		static IndexedTriangleList MakeSolid(const std::vector<VertexAttribute>&& attributes = {});
		static IndexedTriangleList Make(const std::vector<VertexAttribute>&& attributes = {});
	};
}