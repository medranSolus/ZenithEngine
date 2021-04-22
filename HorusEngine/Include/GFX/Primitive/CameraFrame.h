#pragma once
#include "IndexedTriangleList.h"
#include "Camera/ProjectionData.h"

namespace GFX::Primitive
{
	class CameraFrame
	{
	public:
		CameraFrame() = delete;

		static constexpr const char* GetNameFrustum() noexcept { return "F_"; }
		static std::shared_ptr<Data::VertexLayout> GetLayoutFrustum() noexcept { return std::make_shared<Data::VertexLayout>(); }

		static Data::VertexBufferData MakeFrustumVertex(const Camera::ProjectionData& data) noexcept;
		static std::vector<U32> MakeFrustumIndex() noexcept;

		static constexpr const char* GetNameIndicator() noexcept { return "I_"; }
		static std::shared_ptr<Data::VertexLayout> GetLayoutIndicator() noexcept { return std::make_shared<Data::VertexLayout>(); }

		static Data::VertexBufferData MakeIndicatorVertex() noexcept;
		static std::vector<U32> MakeIndicatorIndex() noexcept;
	};
}