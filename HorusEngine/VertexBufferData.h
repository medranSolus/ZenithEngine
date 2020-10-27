#pragma once
#include "Vertex.h"
#include "BoundingBox.h"
#include <memory>

namespace GFX::Data
{
	class VertexBufferData
	{
		template<VertexAttribute Type>
		struct AttributeFill
		{
			static constexpr void Exec(VertexBufferData& buffer, const aiMesh& mesh, size_t index) noexcept
			{
				for (auto size = mesh.mNumVertices, i = 0U; i < size; ++i)
					buffer[i].SetByIndex(index, VertexLayout::Desc<Type>::Extract(mesh, i));
			}
		};
		template<>
		struct AttributeFill<VertexAttribute::Position3D>
		{
			static void Exec(VertexBufferData& buffer, const aiMesh& mesh, size_t index) noexcept
			{
				DirectX::XMVECTOR min = DirectX::XMLoadFloat3(&buffer.boundingBox.GetNegative());
				DirectX::XMVECTOR max = DirectX::XMLoadFloat3(&buffer.boundingBox.GetPositive());
				for (auto size = mesh.mNumVertices, i = 0U; i < size; ++i)
				{
					const auto& position = VertexLayout::Desc<VertexAttribute::Position3D>::Extract(mesh, i);
					const DirectX::XMVECTOR& pos = DirectX::XMLoadFloat3(&position);
					min = DirectX::XMVectorMin(min, pos);
					max = DirectX::XMVectorMax(max, pos);
					buffer[i].SetByIndex(index, position);
				}
				DirectX::XMStoreFloat3(&buffer.boundingBox.GetNegative(), min);
				DirectX::XMStoreFloat3(&buffer.boundingBox.GetPositive(), max);
				buffer.boundingBox.Finalize();
			}
		};

		std::vector<char> buffer;
		BoundingBox boundingBox;
		std::shared_ptr<VertexLayout> layout = nullptr;

	public:
		inline VertexBufferData() noexcept {}
		VertexBufferData(std::shared_ptr<VertexLayout> layout, size_t size = 0U) noexcept(!IS_DEBUG);
		VertexBufferData(std::shared_ptr<VertexLayout> layout, const aiMesh& mesh) noexcept(!IS_DEBUG);
		VertexBufferData(const VertexBufferData&) = default;
		VertexBufferData(VertexBufferData&&) = default;
		VertexBufferData& operator=(const VertexBufferData&) = default;
		VertexBufferData& operator=(VertexBufferData&&) = default;
		~VertexBufferData() = default;

		constexpr const BoundingBox& GetBox() const noexcept { return boundingBox; }
		constexpr void SetBox(const BoundingBox& box) noexcept(!IS_DEBUG) { boundingBox = box; }
		inline std::shared_ptr<VertexLayout> GetLayout() const noexcept { return layout; }
		inline const char* GetData() const noexcept { return buffer.data(); }
		inline size_t Bytes() const noexcept { return buffer.size(); }
		inline size_t Size() const noexcept(!IS_DEBUG) { return buffer.size() / layout->Size(); }
		inline void Reserve(size_t capacity) noexcept(!IS_DEBUG) { buffer.reserve(capacity * layout->Size()); }

		inline const Vertex Back() const noexcept(!IS_DEBUG) { const_cast<VertexBufferData*>(this)->Back(); }
		inline const Vertex Front() const noexcept(!IS_DEBUG) { const_cast<VertexBufferData*>(this)->Front(); }
		inline const Vertex operator[](size_t i) const noexcept(!IS_DEBUG) { const_cast<VertexBufferData&>(*this)[i]; }

		template<typename ...Params>
		void EmplaceBack(Params&&... params) noexcept(!IS_DEBUG);

		void SetBoundingBox() noexcept(!IS_DEBUG);
		void Resize(size_t newSize) noexcept(!IS_DEBUG);
		Vertex Back() noexcept(!IS_DEBUG);
		Vertex Front() noexcept(!IS_DEBUG);
		Vertex operator[](size_t i) noexcept(!IS_DEBUG);
	};

	template<typename ...Params>
	void VertexBufferData::EmplaceBack(Params&& ...params) noexcept(!IS_DEBUG)
	{
		assert(sizeof...(params) <= layout->GetElementCount() && "Param count doesn't match number of vertex elements!");
		buffer.resize(buffer.size() + layout->Size());
		Back().SetByIndex(0U, std::forward<Params>(params)...);
	}
}