#pragma once
#include "Camera.h"
#include "Light.h"
#include "MaterialPBR.h"
#include "Transform.h"
#include "AssetsStreamer.h"

namespace ZE::Data
{
	// Proxy empty types used in grouping entities
	template<typename T>
	concept EmptyType = std::is_empty_v<T>;

	// Enables 3D rendering of entity
	struct RenderLambertian {};
	// Draws outline on the entity
	struct RenderOutline {};
	// Renders wireframe mesh of the entity's geometry
	struct RenderWireframe {};
	// Enables entity to cast shadows
	struct ShadowCaster {};

	// Enables entity to emit directional light
	struct LightDirectional {};
	// Enables entity to emit spot light
	struct LightSpot {};
	// Enables entity to emit point light
	struct LightPoint {};

	// Indicates that material contains transparent or translucent elements
	struct MaterialNotSolid {};

	template<EmptyType T>
	constexpr auto GetRenderGroup() noexcept { return Settings::Data.group<T>(entt::get<TransformGlobal, MaterialID, MeshID>); }
	template<EmptyType T, typename Visibility>
	constexpr auto GetVisibleRenderGroup() noexcept { return Settings::Data.group<Visibility>(entt::get<T, TransformGlobal, MaterialID, MeshID>); }

	inline auto GetDirectionalLightGroup() noexcept { return Settings::Data.group<LightDirectional, DirectionalLight, Direction, DirectionalLightBuffer>(); }
	inline auto GetSpotLightGroup() noexcept { return Settings::Data.group<LightSpot, SpotLight, SpotLightBuffer>(entt::get<TransformGlobal>); }
	inline auto GetPointLightGroup() noexcept { return Settings::Data.group<LightPoint, PointLight, PointLightBuffer>(entt::get<TransformGlobal>); }
}