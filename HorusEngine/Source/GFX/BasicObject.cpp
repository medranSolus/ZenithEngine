#include "GFX/BasicObject.h"

namespace GFX
{
	Data::CBuffer::DCBLayout BasicObject::MakeLayout() noexcept
	{
		static Data::CBuffer::DCBLayout layout;
		static bool initNeeded = true;
		if (initNeeded)
		{
			layout.Add(DCBElementType::Float3, "position");
			layout.Add(DCBElementType::Float, "scale");
			layout.Add(DCBElementType::Float3, "angle");
			initNeeded = false;
		}
		return layout;
	}

	BasicObject::BasicObject(const Float3& position, std::string&& name, float scale) noexcept
		: buffer(MakeLayout()), name(std::move(name))
	{
		buffer["position"] = position;
		buffer["angle"] = Float3(0.0f, 0.0f, 0.0f);
		buffer["scale"] = scale;
	}

	void BasicObject::UpdatePos(const Float3& delta) noexcept
	{
		Math::XMStoreFloat3(&buffer["position"],
			Math::XMVectorAdd(Math::XMLoadFloat3(&buffer["position"]), Math::XMLoadFloat3(&delta)));
	}

	void BasicObject::UpdateAngle(const Float3& deltaAngle) noexcept
	{
		Math::XMStoreFloat3(&buffer["angle"],
			Math::XMVectorModAngles(Math::XMVectorAdd(Math::XMLoadFloat3(&buffer["angle"]),
				Math::XMLoadFloat3(&deltaAngle))));
	}
}