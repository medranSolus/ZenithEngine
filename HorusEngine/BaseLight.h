#pragma once
#include "IObject.h"
#include "JobData.h"

namespace GFX::Light
{
	class BaseLight : public IObject, public Pipeline::JobData
	{
	protected:
		mutable std::shared_ptr<IObject> mesh = nullptr;

	public:
		inline UINT GetIndexCount() const noexcept override { return 6U; }
		inline void SetOutline() noexcept override { mesh->SetOutline(); }
		inline void DisableOutline() noexcept override { mesh->DisableOutline(); }
		inline void Submit(uint64_t channelFilter) noexcept override { mesh->Submit(channelFilter); }

		inline const DirectX::XMFLOAT3& GetAngle() const noexcept override { return mesh->GetAngle(); }
		inline void SetAngle(const DirectX::XMFLOAT3& meshAngle) noexcept override { mesh->SetAngle(meshAngle); }

		inline float GetScale() const noexcept { return mesh->GetScale(); }
		inline void SetScale(float newScale) noexcept { mesh->SetScale(newScale); }

		inline const DirectX::XMFLOAT3& GetPos() const noexcept override { return mesh->GetPos(); }
		inline void SetPos(const DirectX::XMFLOAT3& position) noexcept override { mesh->SetPos(position); }

		inline const std::string& GetName() const noexcept override { return mesh->GetName(); }
		inline void SetName(const std::string& newName) noexcept override { mesh->SetName(newName); }

		inline void Update(const DirectX::XMFLOAT3& delta, const DirectX::XMFLOAT3& deltaAngle) noexcept override { mesh->Update(delta, deltaAngle); }
		inline void UpdatePos(const DirectX::XMFLOAT3& delta) noexcept override { mesh->UpdatePos(delta); }
		inline void UpdateAngle(const DirectX::XMFLOAT3& deltaAngle) noexcept override { mesh->UpdateAngle(deltaAngle); }

		inline void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { mesh->Accept(gfx, probe); }

		void Bind(Graphics& gfx) override {}
	};
}