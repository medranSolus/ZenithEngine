#pragma once
#include "Graphics.h"

namespace GFX::Pipeline
{
	class Job
	{
		class JobData* data = nullptr;
		class TechniqueStep* step = nullptr;

	public:
		constexpr Job(class JobData* data, class TechniqueStep* step) noexcept : data(data), step(step) {}
		Job(const Job&) = default;
		Job& operator=(const Job&) = default;
		~Job() = default;

		void Execute(Graphics& gfx);
	};
}