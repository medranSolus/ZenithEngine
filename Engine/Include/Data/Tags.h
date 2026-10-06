#pragma once

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
	struct MaterialTransparent {};
	// Indicates that material requires blending on already rendered geometry
	struct MaterialBlend {};
}