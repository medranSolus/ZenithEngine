#pragma once
#include "ILight.h"
#include "ConstBufferExCache.h"
#include "RenderGraph.h"

namespace GFX::Light
{
	class DirectionalLight : public ILight
	{
		std::string name;

		static inline Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		DirectionalLight(Graphics& gfx, Pipeline::RenderGraph& graph, const std::string& name, const DirectX::XMFLOAT3& direction,
			float intensity = 1.0f, const Data::ColorFloat3& color = { 1.0f, 1.0f, 1.0f }, float size = 1.0f);
		inline DirectionalLight(DirectionalLight&& light) noexcept { *this = std::forward<DirectionalLight&&>(light); }
		inline DirectionalLight& operator=(DirectionalLight&& light) noexcept { this->ILight::operator=(std::forward<ILight&&>(light)); return *this; }
		virtual ~DirectionalLight() = default;

		inline UINT GetIndexCount() const noexcept override { return 6U; }

		inline void SetOutline() noexcept override {}
		inline void DisableOutline() noexcept override {}

		inline const DirectX::XMFLOAT3& GetAngle() const noexcept override { return { 0.0f, 0.0f, 0.0f }; }
		inline void SetAngle(const DirectX::XMFLOAT3& meshAngle) noexcept override {}
		inline void UpdateAngle(const DirectX::XMFLOAT3& deltaAngle) noexcept override {}

		inline float GetScale() const noexcept { return 1.0f; }
		inline void SetScale(float newScale) noexcept {}

		inline const DirectX::XMFLOAT3& GetPos() const noexcept override { return { 0.0f, 0.0f, 0.0f }; }

		inline const std::string& GetName() const noexcept override { return name; }
		inline void SetName(const std::string& newName) noexcept override { name = newName; }

		inline void Update(const DirectX::XMFLOAT3& delta, const DirectX::XMFLOAT3& deltaAngle) noexcept override {}
		inline void UpdatePos(const DirectX::XMFLOAT3& delta) noexcept override {}
		inline void SetPos(const DirectX::XMFLOAT3& position) noexcept override {}

		inline bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return lightBuffer->Accept(gfx, probe); }
		inline void Submit(uint64_t channelFilter) noexcept override { JobData::Submit(channelFilter); }
		inline void Bind(Graphics& gfx) override { lightBuffer->Bind(gfx); }
	};
}