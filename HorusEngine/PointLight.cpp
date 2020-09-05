#include "PointLight.h"
#include "PersonCamera.h"

namespace GFX::Light
{
	Data::CBuffer::DCBLayout PointLight::MakeLayout() noexcept
	{
		static Data::CBuffer::DCBLayout layout;
		static bool initNeeded = true;
		if (initNeeded)
		{
			layout.Add(DCBElementType::Color3, "ambientColor");
			layout.Add(DCBElementType::Float, "atteuationConst");
			layout.Add(DCBElementType::Color3, "lightColor");
			layout.Add(DCBElementType::Float, "atteuationLinear");
			layout.Add(DCBElementType::Float3, "lightPos");
			layout.Add(DCBElementType::Float, "attenuationQuad");
			layout.Add(DCBElementType::Float, "lightIntensity");
			initNeeded = false;
		}
		return layout;
	}

	PointLight::PointLight(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position, const std::string& name, float radius)
		: mesh(gfx, graph, position, name, { 1.0f, 1.0f, 1.0f }, 3, 3, radius, radius, radius)
	{
		Data::CBuffer::DynamicCBuffer buffer(MakeLayout());
		buffer["ambientColor"] = std::move(Data::ColorFloat3(0.05f, 0.05f, 0.05f));
		buffer["atteuationConst"] = 1.0f;
		buffer["lightColor"] = std::move(Data::ColorFloat3(1.0f, 1.0f, 1.0f));
		buffer["atteuationLinear"] = 0.045f;
		buffer["lightPos"] = position;
		buffer["attenuationQuad"] = 0.0075f;
		buffer["lightIntensity"] = 5.0f;
		lightBuffer = Resource::ConstBufferExPixelCache::Get(gfx, name, std::move(buffer));
		lightCamera = std::make_shared<Camera::PersonCamera>(gfx, graph, name + "Cam", 1.047f, 0.01f, 500.0f, 0, 0, position);
	}

	void PointLight::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		lightBuffer->Accept(gfx, probe);
		mesh.Accept(gfx, probe);
	}

	void PointLight::Bind(Graphics& gfx, const Camera::ICamera& camera) const noexcept
	{
		DirectX::XMStoreFloat3(&lightBuffer->GetBuffer()["lightPos"], DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&mesh.GetPos()), camera.GetView()));
		lightCamera->SetPos(mesh.GetPos());
		lightBuffer->Bind(gfx);
	}
}