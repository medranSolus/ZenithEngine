#include "TechniqueStep.h"

namespace GFX::Pipeline
{
	void TechniqueStep::Bind(Graphics& gfx) noexcept
	{
		for (auto& bind : binds)
			bind->Bind(gfx);
	}
}