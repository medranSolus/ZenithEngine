#pragma once
#include "Camera.h"
#include "Geometry.h"
#include "Light.h"
#include "MaterialPBR.h"
#include "Transform.h"

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
	constexpr auto GetRenderGroup(Storage& registry) noexcept { return registry.group<T>(entt::get<TransformGlobal, MaterialID, MeshID>); }
	template<EmptyType T, EmptyType Visibility>
	constexpr auto GetVisibleRenderGroup(Storage& registry) noexcept { return registry.group<Visibility>(entt::get<T, TransformGlobal, MaterialID, MeshID>); }

	inline auto GetDirectionalLightGroup(Storage& registry) noexcept { return registry.group<LightDirectional, DirectionalLight, Direction, DirectionalLightBuffer>(); }
	inline auto GetSpotLightGroup(Storage& registry) noexcept { return registry.group<LightSpot, SpotLight, SpotLightBuffer>(entt::get<TransformGlobal>); }
	inline auto GetPointLightGroup(Storage& registry) noexcept { return registry.group<LightPoint, PointLight, PointLightBuffer>(entt::get<TransformGlobal>); }
}