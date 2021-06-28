#pragma once
#include "IndexedTriangleList.h"

namespace ZE::GFX::Primitive
{
	class Cone
	{
	public:
		Cone() = delete;

		static std::string GetNameSolid(U32 density, const std::vector<VertexAttribute>& attributes = {}) noexcept;
		static std::shared_ptr<Data::VertexLayout> GetLayoutSolid(const std::vector<VertexAttribute>& attributes = {}) noexcept;
		static IndexedTriangleList MakeSolid(U32 density, const std::vector<VertexAttribute>& attributes = {}) noexcept;
	};
}