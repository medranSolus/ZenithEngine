#pragma once
#include "Types.h"

namespace ZE::GFX::Data
{
	class BoundingBox
	{
		static BoundingBox dummy;

		// Center+ Extents-
		Math::BoundingBox box;

	public:
		constexpr BoundingBox() noexcept : box({ -FLT_MAX, -FLT_MAX, -FLT_MAX }, { FLT_MAX, FLT_MAX, FLT_MAX }) {}
		BoundingBox(float up, float down, float left, float right, float front, float back)
			: box({ right, up, back }, { left, down, front }) { Finalize(); }

		static constexpr BoundingBox& GetEmpty() noexcept { return dummy; }

		constexpr Float3& GetPositive() noexcept { return box.Center; }
		constexpr Float3& GetNegative() noexcept { return box.Extents; }

		void Finalize() noexcept;
		bool Intersects(const Math::BoundingFrustum& frustum, const Matrix& transform) const noexcept;
	};
}