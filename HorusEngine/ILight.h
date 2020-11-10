#pragma once
#include "IObject.h"
#include "JobData.h"
#include "IVolume.h"
#include "IShape.h"

namespace GFX::Light
{
	class ILight : public IObject, public Pipeline::JobData
	{
	protected:
		std::unique_ptr<Volume::IVolume> volume = nullptr;
		std::unique_ptr<Shape::IShape> mesh = nullptr;
		std::shared_ptr<Resource::ConstBufferExPixelCache> lightBuffer = nullptr;

		void SetAttenuation(size_t range) noexcept;

	public:
		ILight() = default;
		inline ILight(ILight&& light) noexcept { *this = std::forward<ILight&&>(light); }
		ILight& operator=(ILight&& light) noexcept;
		virtual ~ILight() = default;

		inline const Data::CBuffer::DynamicCBuffer& GetBuffer() const noexcept { return lightBuffer->GetBufferConst(); }

		inline const Data::BoundingBox& GetBoundingBox() const noexcept override { return Data::BoundingBox::GetEmpty(); }
		inline UINT GetIndexCount() const noexcept override { return volume->GetIndexCount(); }

		inline void SetOutline() noexcept override { mesh->SetOutline(); }
		inline void DisableOutline() noexcept override { mesh->DisableOutline(); }

		inline const DirectX::XMFLOAT3& GetAngle() const noexcept override { return mesh->GetAngle(); }
		inline void SetAngle(const DirectX::XMFLOAT3& meshAngle) noexcept override { mesh->SetAngle(meshAngle); }
		inline void UpdateAngle(const DirectX::XMFLOAT3& deltaAngle) noexcept override { mesh->UpdateAngle(deltaAngle); }

		inline float GetScale() const noexcept { return mesh->GetScale(); }
		inline void SetScale(float newScale) noexcept { mesh->SetScale(newScale); }

		inline const DirectX::XMFLOAT3& GetPos() const noexcept override { return mesh->GetPos(); }

		inline const std::string& GetName() const noexcept override { return mesh->GetName(); }
		inline void SetName(const std::string& newName) noexcept override { mesh->SetName(newName); }

		inline bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return lightBuffer->Accept(gfx, probe) || mesh->Accept(gfx, probe); }
		inline void Submit(uint64_t channelFilter) noexcept override { mesh->Submit(channelFilter); JobData::Submit(channelFilter); }

		void Update(const DirectX::XMFLOAT3& delta, const DirectX::XMFLOAT3& deltaAngle) noexcept override;
		void UpdatePos(const DirectX::XMFLOAT3& delta) noexcept override;
		void SetPos(const DirectX::XMFLOAT3& position) noexcept override;

		void Bind(Graphics& gfx) override;
	};
}