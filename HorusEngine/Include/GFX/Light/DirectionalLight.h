#pragma once
#include "ILight.h"
#include "GFX/Resource/ConstBufferExCache.h"
#include "GFX/Pipeline/RenderGraph.h"

namespace GFX::Light
{
	class DirectionalLight : public ILight
	{
		static inline Float3 dummyData = { 0.0f,0.0f,0.0f };
		std::string name;

		static Data::CBuffer::DCBLayout MakeLayout() noexcept;

	public:
		DirectionalLight(Graphics& gfx, Pipeline::RenderGraph& graph, std::string&& name,
			float intensity, const ColorF3& color, const Float3& direction);
		DirectionalLight(DirectionalLight&&) = default;
		DirectionalLight& operator=(DirectionalLight&&) = default;
		virtual ~DirectionalLight() = default;

		constexpr U32 GetIndexCount() const noexcept override { return 6; }

		constexpr void SetOutline() noexcept override {}
		constexpr void DisableOutline() noexcept override {}

		constexpr const Float3& GetAngle() const noexcept override { return dummyData; }
		constexpr void SetAngle(const Float3& meshAngle) noexcept override {}
		constexpr void UpdateAngle(const Float3& deltaAngle) noexcept override {}

		constexpr float GetScale() const noexcept { return 1.0f; }
		constexpr void SetScale(float newScale) noexcept {}

		constexpr const Float3& GetPos() const noexcept override { return dummyData; }

		constexpr const std::string& GetName() const noexcept override { return name; }
		void SetName(const std::string& newName) noexcept override { name = newName; }

		constexpr void Update(const Float3& delta, const Float3& deltaAngle) noexcept override {}
		constexpr void UpdatePos(const Float3& delta) noexcept override {}
		constexpr void SetPos(const Float3& position) noexcept override {}

		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return lightBuffer->Accept(gfx, probe); }
		void Submit(U64 channelFilter) const noexcept override { JobData::Submit(channelFilter); }
		void Bind(Graphics& gfx) const noexcept override { lightBuffer->Bind(gfx); }
	};
}