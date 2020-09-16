#pragma once
#include "Graphics.h"

namespace GFX::Pipeline
{
	class JobData
	{
	public:
		virtual ~JobData() = default;

		virtual UINT GetIndexCount() const noexcept = 0;
		virtual void Bind(Graphics& gfx) = 0;
	};
}