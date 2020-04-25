#pragma once
#include "Graphics.h"

namespace GFX::Shape
{
	class BaseShape;
}
namespace GFX::Pipeline
{
	class Job
	{
		Shape::BaseShape* shape = nullptr;
		class TechniqueStep* step = nullptr;

	public:
		constexpr Job(Shape::BaseShape* shape, class TechniqueStep* step) noexcept : shape(shape), step(step) {}
		Job(const Job&) = default;
		Job& operator=(const Job&) = default;
		~Job() = default;

		void Execute(Graphics& gfx) noexcept(!IS_DEBUG);
	};
}