#pragma once
#include "IndexedTriangleList.h"

namespace GFX::Primitive
{
	class Cube
	{
	public:
		Cube() = delete;

		static std::string GetNameSolid(const std::vector<VertexAttribute>&& attributes = {}) noexcept;
		static std::string GetName(const std::vector<VertexAttribute>&& attributes = {}) noexcept;

		static std::shared_ptr<Data::VertexLayout> GetLayoutSolid(const std::vector<VertexAttribute>&& attributes = {}) noexcept;
		static std::shared_ptr<Data::VertexLayout> GetLayout(const std::vector<VertexAttribute>&& attributes = {}) noexcept;

		static IndexedTriangleList MakeSolid(const std::vector<VertexAttribute>&& attributes = {});
		static IndexedTriangleList Make(const std::vector<VertexAttribute>&& attributes = {});
	};
}