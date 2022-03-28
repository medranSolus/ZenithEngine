#pragma once
#include "GFX/Resource/CBuffer.h"
#include "GFX/TransformBuffer.h"
#include "Data/Tags.h"
#include  <type_traits>

namespace ZE::GFX::Pipeline::RenderPass::Utils
{
	// Order for view sorting objects
	enum class Sort : bool { Ascending, Descending };

	// Resizes vector of temporary transform buffers
	template<typename SingleTransform, typename TransformBuffer, U64 ShrinkStepOffset = 0>
	constexpr void ResizeTransformBuffers(Device& dev, std::vector<Resource::CBuffer>& transformBuffers, U64 count);

	// Perform frustum culling on entities in a group and emplace `Visibility` components on those inside camera frustum.
	// `VisibilitySolid` component is added only to entities which material is not transparent,
	// to other ones `VisibilityTransparent` is added. Specify both as same component to avoid whole material check.
	template<Data::EmptyType VisibilitySolid, Data::EmptyType VisibilityTransparent>
	constexpr void FrustumCulling(Data::Storage& registry, const Data::Storage& resources,
		const auto& group, const Math::BoundingFrustum& frustum) noexcept;

	// Sort entities according to distance from camera
	template<Sort Order>
	constexpr void ViewSort(auto& group, const Vector& cameraPos) noexcept;
	// Sort entities front-back according to distance from camera
	constexpr void ViewSortAscending(auto& group, const Vector& cameraPos) noexcept { ViewSort<Sort::Ascending>(group, cameraPos); }
	// Sort entities back-front according to distance from camera
	constexpr void ViewSortDescending(auto& group, const Vector& cameraPos) noexcept { ViewSort<Sort::Descending>(group, cameraPos); }

#pragma region Functions
	template<typename SingleTransform, typename TransformBuffer, U64 ShrinkStepOffset>
	constexpr void ResizeTransformBuffers(Device& dev, std::vector<Resource::CBuffer>& transformBuffers, U64 count)
	{
		U64 buffCount = Math::DivideRoundUp(count * sizeof(SingleTransform), sizeof(TransformBuffer));
		if (buffCount + ShrinkStepOffset < transformBuffers.size())
		{
			for (U64 i = buffCount; i < transformBuffers.size(); ++i)
				transformBuffers.at(i).Free(dev);
			transformBuffers.resize(buffCount);
		}
		else if (buffCount > transformBuffers.size())
		{
			U64 i = transformBuffers.size();
			transformBuffers.resize(buffCount);
			for (; i < buffCount; ++i)
				transformBuffers.at(i).Init(dev, nullptr, sizeof(TransformBuffer), true);
		}
	}

	template<Data::EmptyType VisibilitySolid, Data::EmptyType VisibilityTransparent>
	constexpr void FrustumCulling(Data::Storage& registry, const Data::Storage& resources,
		const auto& group, const Math::BoundingFrustum& frustum) noexcept
	{
		for (EID entity : group)
		{
			const auto& transform = group.get<Data::TransformGlobal>(entity);

			Math::BoundingBox box = resources.get<Math::BoundingBox>(group.get<Data::MeshID>(entity).ID);
			box.Transform(box, Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));

			// Mark entity as visible
			if (frustum.Intersects(box))
			{
				if constexpr (std::is_same_v<VisibilitySolid, VisibilityTransparent>)
					registry.emplace<VisibilitySolid>(entity);
				else
				{
					if (resources.all_of<Data::MaterialNotSolid>(group.get<Data::MaterialID>(entity).ID))
						registry.emplace<VisibilityTransparent>(entity);
					else
						registry.emplace<VisibilitySolid>(entity);
				}
			}
		}
	}

	template<Sort Order>
	constexpr void ViewSort(auto& group, const Vector& cameraPos) noexcept
	{
		group.sort<Data::TransformGlobal>([&cameraPos](const Data::TransformGlobal& t1, const Data::TransformGlobal& t2) -> bool
			{
				const float len1 = Math::XMVectorGetX(Math::XMVector3Length(Math::XMVectorSubtract(Math::XMLoadFloat3(&t1.Position), cameraPos)));
				const float len2 = Math::XMVectorGetX(Math::XMVector3Length(Math::XMVectorSubtract(Math::XMLoadFloat3(&t2.Position), cameraPos)));
				if constexpr (Order == Sort::Ascending)
					return len1 < len2;
				else if constexpr (Order == Sort::Descending)
					return len1 > len2;
			});
	}
#pragma endregion
}