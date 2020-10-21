#pragma once
#include "IndexedTriangleList.h"

namespace GFX::Primitive
{
	class Sphere
	{
	public:
		Sphere() = delete;

		static std::string GetNameUVSolid(unsigned int latitudeDensity, unsigned int longitudeDensity, const std::vector<VertexAttribute>& attributes = {}) noexcept;
		static std::string GetNameUV(unsigned int latitudeDensity, unsigned int longitudeDensity, const std::vector<VertexAttribute>& attributes = {}) noexcept;

		static std::shared_ptr<Data::VertexLayout> GetLayoutUVSolid(const std::vector<VertexAttribute>& attributes = {}) noexcept;
		static std::shared_ptr<Data::VertexLayout> GetLayoutUV(const std::vector<VertexAttribute>& attributes = {}) noexcept;

		//latitudeDensity: N-S, longitudeDensity: W-E
		static IndexedTriangleList MakeUVSolid(unsigned int latitudeDensity, unsigned int longitudeDensity, const std::vector<VertexAttribute>& attributes = {}) noexcept;
		//latitudeDensity: N-S, longitudeDensity: W-E
		static IndexedTriangleList MakeUV(unsigned int latitudeDensity, unsigned int longitudeDensity, const std::vector<VertexAttribute>& attributes = {}) noexcept;

		static std::string GetNameIcoSolid(unsigned int density, const std::vector<VertexAttribute>& attributes = {}) noexcept;
		static std::string GetNameIco(unsigned int density, const std::vector<VertexAttribute>& attributes = {}) noexcept;

		static inline std::shared_ptr<Data::VertexLayout> GetLayoutIcoSolid(const std::vector<VertexAttribute>& attributes = {}) noexcept { return GetLayoutUVSolid(attributes); }
		static inline std::shared_ptr<Data::VertexLayout> GetLayoutIco(const std::vector<VertexAttribute>& attributes = {}) noexcept { return GetLayoutUV(attributes); }

		static IndexedTriangleList MakeIcoSolid(unsigned int density, const std::vector<VertexAttribute>& attributes = {}) noexcept;
		static IndexedTriangleList MakeIco(unsigned int density, const std::vector<VertexAttribute>& attributes = {}) noexcept;
	};
}