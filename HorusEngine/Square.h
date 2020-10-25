#pragma once
#include "IndexedTriangleList.h"

namespace GFX::Primitive
{
	class Square
	{
	public:
		Square() = delete;

		static std::string GetName(const std::vector<VertexAttribute>& attributes = {}) noexcept;
		static inline std::string GetNameNDC2D() noexcept { return std::string(typeid(Square).name()) + "$NDC2D"; }

		static std::shared_ptr<Data::VertexLayout> GetLayout(const std::vector<VertexAttribute>& attributes = {}) noexcept;
		static std::shared_ptr<Data::VertexLayout> GetLayoutNDC2D() noexcept;

		static IndexedTriangleList Make(const std::vector<VertexAttribute>& attributes = {}) noexcept;
		static IndexedTriangleList MakeNDC2D() noexcept;
	};
}