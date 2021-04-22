#pragma once
#include "GFX/IObject.h"
#include "GFX/Pipeline/JobData.h"
#include "Volume/IVolume.h"
#include "GFX/Shape/IShape.h"

namespace GFX::Light
{
	class ILight : public IObject, public virtual Pipeline::JobData
	{
	protected:
		std::unique_ptr<Volume::IVolume> volume = nullptr;
		std::unique_ptr<Shape::IShape> mesh = nullptr;
		GfxResPtr<Resource::ConstBufferExPixelCache> lightBuffer;

		void SetAttenuation(U64 range) noexcept;

	public:
		ILight() = default;
		ILight(ILight&&) = default;
		ILight(const ILight&) = delete;
		ILight& operator=(ILight&&) = default;
		ILight& operator=(const ILight&) = delete;
		virtual ~ILight() = default;

		constexpr const Data::CBuffer::DynamicCBuffer& GetBuffer() const noexcept { return lightBuffer->GetBufferConst(); }

		constexpr const Data::BoundingBox& GetBoundingBox() const noexcept override { return Data::BoundingBox::GetEmpty(); }
		constexpr U32 GetIndexCount() const noexcept override { return volume->GetIndexCount(); }

		void SetOutline() noexcept override { mesh->SetOutline(); }
		void DisableOutline() noexcept override { mesh->DisableOutline(); }

		constexpr const Float3& GetAngle() const noexcept override { return mesh->GetAngle(); }
		constexpr void SetAngle(const Float3& meshAngle) noexcept override { mesh->SetAngle(meshAngle); }
		void UpdateAngle(const Float3& deltaAngle) noexcept override { mesh->UpdateAngle(deltaAngle); }

		constexpr float GetScale() const noexcept { return mesh->GetScale(); }
		constexpr void SetScale(float newScale) noexcept { mesh->SetScale(newScale); }

		constexpr const Float3& GetPos() const noexcept override { return mesh->GetPos(); }

		constexpr const std::string& GetName() const noexcept override { return mesh->GetName(); }
		constexpr void SetName(const std::string& newName) noexcept override { mesh->SetName(newName); }

		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return lightBuffer->Accept(gfx, probe) || mesh->Accept(gfx, probe); }
		void Submit(U64 channelFilter) const noexcept override { mesh->Submit(channelFilter); JobData::Submit(channelFilter); }

		void Update(const Float3& delta, const Float3& deltaAngle) noexcept override;
		void UpdatePos(const Float3& delta) noexcept override;
		void SetPos(const Float3& position) noexcept override;

		void Bind(Graphics& gfx) const noexcept override;
	};
}