#pragma once
#include "SolidGlobe.h"
#include "ConstBufferPixel.h"
#include "CBuffers.h"
#include "ICamera.h"

namespace GFX::Light
{
	class PointLight : public IObject
	{
		mutable Data::CBuffer::Light lightBuffer;
		mutable Shape::SolidGlobe mesh;
		mutable Resource::ConstBufferPixel<Data::CBuffer::Light> buffer;

	public:
		PointLight(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, float radius = 0.5f);
		PointLight(const PointLight&) = delete;
		PointLight& operator=(const PointLight&) = delete;
		virtual ~PointLight() = default;

		inline void Submit(Pipeline::RenderCommander& renderer) noexcept(!IS_DEBUG) override { mesh.Submit(renderer); }

		inline const DirectX::XMFLOAT3& GetAngle() const noexcept override { return mesh.GetAngle(); }
		inline void SetAngle(const DirectX::XMFLOAT3& meshAngle) noexcept override { mesh.SetAngle(meshAngle); }

		inline float GetScale() const noexcept { return mesh.GetScale(); }
		inline void SetScale(float newScale) noexcept { mesh.SetScale(newScale); }

		inline const DirectX::XMFLOAT3& GetPos() const noexcept override { return mesh.GetPos(); }
		inline void SetPos(const DirectX::XMFLOAT3& position) noexcept override { mesh.SetPos(position); }

		inline const std::string& GetName() const noexcept override { return mesh.GetName(); }
		inline void SetName(const std::string& newName) noexcept override { mesh.SetName(newName); }

		inline void Update(const DirectX::XMFLOAT3& delta, const DirectX::XMFLOAT3& deltaAngle = { 0.0f,0.0f,0.0f }) noexcept override { mesh.Update(delta, deltaAngle); }

		void ShowWindow(Graphics& gfx) noexcept override;
		void Bind(Graphics& gfx, const Camera::ICamera& camera) const noexcept;
	};
}