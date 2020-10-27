#pragma once
#include <DirectXMath.h>
#include <DirectXCollision.h>

namespace GFX::Data
{
	class BoundingBox
	{
		static BoundingBox dummy;

		// Center+ Extents-
		DirectX::BoundingBox box;

	public:
		constexpr BoundingBox() noexcept : box({ -FLT_MAX, -FLT_MAX, -FLT_MAX }, { FLT_MAX, FLT_MAX, FLT_MAX }) {}
		inline BoundingBox(float up, float down, float left, float right, float front, float back)
			: box({ right, up, back }, { left, down, front })
		{
			Finalize();
		}

		static constexpr BoundingBox& GetEmpty() noexcept { return dummy; }

		constexpr DirectX::XMFLOAT3& GetPositive() noexcept { return box.Center; }
		constexpr DirectX::XMFLOAT3& GetNegative() noexcept { return box.Extents; }

		void Finalize() noexcept;
		bool Intersects(const DirectX::BoundingFrustum& frustum, const DirectX::XMMATRIX& transform) const noexcept;
	};
}