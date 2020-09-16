#pragma once
#include "IObject.h"
#include "JobData.h"

namespace GFX::Light
{
	class ILight : public IObject, public Pipeline::JobData
	{
	public:
		virtual ~ILight() = default;

		inline UINT GetIndexCount() const noexcept override { return 6U; }
		void Bind(Graphics& gfx) override {}
	};
}