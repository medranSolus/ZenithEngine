#include "GFX/BasicObject.h"

namespace ZE::GFX
{
	Data::CBuffer::DCBLayout BasicObject::MakeLayout() noexcept
	{
		static Data::CBuffer::DCBLayout layout;
		static bool initNeeded = true;
		if (initNeeded)
		{
			layout.Add(DCBElementType::Float4, "rotation");
			layout.Add(DCBElementType::Float3, "position");
			layout.Add(DCBElementType::Float, "scale");
			initNeeded = false;
		}
		return layout;
	}

	BasicObject::BasicObject(const Float3& position, std::string&& name, float scale) noexcept
		: buffer(MakeLayout()), name(std::move(name))
	{
		buffer["position"] = position;
		buffer["rotation"] = Float4(0.0f, 0.0f, 0.0f, 1.0f);
		buffer["scale"] = scale;
	}

	void BasicObject::UpdatePos(const Float3& delta) noexcept
	{
		Math::XMStoreFloat3(&buffer["position"],
			Math::XMVectorAdd(Math::XMLoadFloat3(&buffer["position"]), Math::XMLoadFloat3(&delta)));
	}

	void BasicObject::UpdateAngle(const Vector& rotor) noexcept
	{
		Float4& rotation = buffer["rotation"];
		Math::XMStoreFloat4(&rotation, Math::XMQuaternionMultiply(Math::XMLoadFloat4(&rotation), rotor));
	}
}