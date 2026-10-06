#pragma once
#include "Data/CubemapSource.h"
#include "Data/Transform.h"
#include "Data/Tags.h"
#include "Settings.h"
#include <type_traits>

namespace ZE::GFX::Pipeline::RenderPass::Utils
{
	// Order for view sorting objects
	enum class Sort : bool { Ascending, Descending };

	// Perform frustum culling on entities in a view and emplace `Visibility` components on those inside camera frustum.
	// `VisibilitySolid` component is added only to entities which material is not transparent,
	// to other ones `VisibilityTransparent` is added. Specify both as same component to avoid whole material check.
	template<typename VisibilitySolid, typename VisibilityTransparent>
	constexpr void FrustumCulling(const auto& view, const Math::BoundingFrustum& frustum) noexcept;

	// Sort entities according to distance from camera
	// WARNING: Can change order of entities based on Data::TransformGlobal component so all references to it will be invalidated!
	template<Sort ORDER>
	constexpr void ViewSort(auto& view, const Vector& cameraPos) noexcept;
	// Sort entities front-back according to distance from camera
	// WARNING: Can change order of entities based on Data::TransformGlobal component so all references to it will be invalidated!
	constexpr void ViewSortAscending(auto& view, const Vector& cameraPos) noexcept { ViewSort<Sort::Ascending>(view, cameraPos); }
	// Sort entities back-front according to distance from camera
	// WARNING: Can change order of entities based on Data::TransformGlobal component so all references to it will be invalidated!
	constexpr void ViewSortDescending(auto& view, const Vector& cameraPos) noexcept { ViewSort<Sort::Descending>(view, cameraPos); }

	// Display information about current cubemap source in debug UI
	void ShowCubemapDebugUI(const char* title, const Data::CubemapSource& source, const char* newSourceDir, Data::CubemapSource& newSource, bool& updateData, bool& updateError) noexcept;

#pragma region Functions
	template<typename VisibilitySolid, typename VisibilityTransparent>
	constexpr void FrustumCulling(const auto& view, const Math::BoundingFrustum& frustum) noexcept
	{
		for (EID entity : view)
		{
			const auto& transform = view.get<Data::TransformGlobal>(entity);

			Math::BoundingBox box = Settings::DataBank.GetAssetsData().get<Math::BoundingBox>(view.get<Data::MeshID>(entity).ID);
			box.Transform(box, Math::GetTransform(transform.Position, transform.Rotation, transform.Scale));

			// Mark entity as visible
			if (frustum.Intersects(box)
#if !_ZE_MODE_RELEASE
				|| Settings::IsEnabledNoCulling()
#endif
				)
			{
				if constexpr (std::is_same_v<VisibilitySolid, VisibilityTransparent>)
					Settings::DataBank.GetWorldData().emplace<VisibilitySolid>(entity);
				else
				{
					if (Settings::DataBank.GetAssetsData().all_of<Data::MaterialTransparent>(view.get<Data::MaterialID>(entity).ID))
						Settings::DataBank.GetWorldData().emplace<VisibilityTransparent>(entity);
					else
						Settings::DataBank.GetWorldData().emplace<VisibilitySolid>(entity);
				}
			}
		}
	}

	template<Sort ORDER>
	constexpr void ViewSort(auto& view, const Vector& cameraPos) noexcept
	{
		Settings::DataBank.GetWorldData().sort<Data::TransformGlobal>([&cameraPos](const Data::TransformGlobal& t1, const Data::TransformGlobal& t2) -> bool
			{
				const float len1 = Math::XMVectorGetX(Math::XMVector3Length(Math::XMVectorSubtract(Math::XMLoadFloat3(&t1.Position), cameraPos)));
				const float len2 = Math::XMVectorGetX(Math::XMVector3Length(Math::XMVectorSubtract(Math::XMLoadFloat3(&t2.Position), cameraPos)));
				if constexpr (ORDER == Sort::Ascending)
					return len1 < len2;
				else if constexpr (ORDER == Sort::Descending)
					return len1 > len2;
			});
		view.use<Data::TransformGlobal>();
	}
#pragma endregion
}