#pragma once
#include "IndexedTriangleList.h"

namespace ZE::GFX::Primitive
{
	class Sphere
	{
	public:
		Sphere() = delete;

		static std::string GetNameUVSolid(U32 latitudeDensity, U32 longitudeDensity, const std::vector<VertexAttribute>& attributes = {}) noexcept;
		static std::string GetNameUV(U32 latitudeDensity, U32 longitudeDensity, const std::vector<VertexAttribute>& attributes = {}) noexcept;

		static std::shared_ptr<Data::VertexLayout> GetLayoutUVSolid(const std::vector<VertexAttribute>& attributes = {}) noexcept;
		static std::shared_ptr<Data::VertexLayout> GetLayoutUV(const std::vector<VertexAttribute>& attributes = {}) noexcept;

		//latitudeDensity: N-S, longitudeDensity: W-E
		static IndexedTriangleList MakeUVSolid(U32 latitudeDensity, U32 longitudeDensity, const std::vector<VertexAttribute>& attributes = {}) noexcept;
		//latitudeDensity: N-S, longitudeDensity: W-E
		static IndexedTriangleList MakeUV(U32 latitudeDensity, U32 longitudeDensity, const std::vector<VertexAttribute>& attributes = {}) noexcept;

		static std::string GetNameIcoSolid(U32 density, const std::vector<VertexAttribute>& attributes = {}) noexcept;
		static std::string GetNameIco(U32 density, const std::vector<VertexAttribute>& attributes = {}) noexcept;

		static std::shared_ptr<Data::VertexLayout> GetLayoutIcoSolid(const std::vector<VertexAttribute>& attributes = {}) noexcept { return GetLayoutUVSolid(attributes); }
		static std::shared_ptr<Data::VertexLayout> GetLayoutIco(const std::vector<VertexAttribute>& attributes = {}) noexcept { return GetLayoutUV(attributes); }

		static IndexedTriangleList MakeIcoSolid(U32 density, const std::vector<VertexAttribute>& attributes = {}) noexcept;
		static IndexedTriangleList MakeIco(U32 density, const std::vector<VertexAttribute>& attributes = {}) noexcept;
	};
}