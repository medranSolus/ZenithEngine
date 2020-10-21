#pragma once
#include "IndexedTriangleList.h"

namespace GFX::Primitive
{
	class Cone
	{
	public:
		Cone() = delete;

		static std::string GetNameSolid(unsigned int density, const std::vector<VertexAttribute>& attributes = {}) noexcept;
		static std::shared_ptr<Data::VertexLayout> GetLayoutSolid(const std::vector<VertexAttribute>& attributes = {}) noexcept;
		static IndexedTriangleList MakeSolid(unsigned int density, const std::vector<VertexAttribute>& attributes = {}) noexcept;
	};
}