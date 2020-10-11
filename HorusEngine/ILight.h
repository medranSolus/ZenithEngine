#pragma once
#include "IObject.h"
#include "JobData.h"

namespace GFX::Light
{
	class ILight : public IObject, public Pipeline::JobData
	{
	public:
		virtual ~ILight() = default;
	};
}