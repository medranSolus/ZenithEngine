#include "BasicObject.h"

namespace GFX
{
	inline Data::CBuffer::DCBLayout BasicObject::MakeLayout() noexcept
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

	BasicObject::BasicObject(const DirectX::XMFLOAT3& position, const std::string& name, float scale) noexcept
		: buffer(MakeLayout()), name(name)
	{
		buffer["position"] = position;
		buffer["angle"] = std::move(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
		buffer["scale"] = scale;
	}

	void BasicObject::UpdatePos(const DirectX::XMFLOAT3& delta) noexcept
	{
		DirectX::XMStoreFloat3(&buffer["position"], DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&buffer["position"]), DirectX::XMLoadFloat3(&delta)));
	}

	void BasicObject::UpdateAngle(const DirectX::XMFLOAT3& deltaAngle) noexcept
	{
		DirectX::XMStoreFloat3(&buffer["angle"],
			DirectX::XMVectorModAngles(DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&buffer["angle"]),
				DirectX::XMLoadFloat3(&deltaAngle))));
	}
}