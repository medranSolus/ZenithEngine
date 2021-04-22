#pragma once
#include "GFX/Resource/ConstBufferTransform.h"
#include "GFX/Data/VertexLayout.h"
#include "GFX/Pipeline/RenderChannels.h"

namespace GFX::Visual
{
	class IVisual : public Probe::IProbeable
	{
		std::vector<GfxResPtr<Resource::IBindable>> binds;

	protected:
		mutable GfxResPtr<Resource::ConstBufferTransform> transformBuffer;

		IVisual() = default;
		IVisual(IVisual&&) = default;
		IVisual(const IVisual&) = default;
		IVisual& operator=(IVisual&&) = default;
		IVisual& operator=(const IVisual&) = default;

	public:
		virtual ~IVisual() = default;

		constexpr bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return false; }
		void AddBind(GfxResPtr<Resource::IBindable>&& bind) noexcept { binds.emplace_back(std::forward<GfxResPtr<Resource::IBindable>&&>(bind)); }
		Float3 GetTransformPos() const noexcept { return transformBuffer->GetPos(); }
		Matrix GetTransform() const noexcept { return transformBuffer->GetTransform(); }

		virtual void SetTransformBuffer(Graphics& gfx, const GfxObject& parent) const { transformBuffer = GfxResPtr<Resource::ConstBufferTransform>(gfx, parent); }
		virtual void Bind(Graphics& gfx, RenderChannel mode) const { Bind(gfx); }

		template<typename R>
		R* GetResource() noexcept;
		template<typename R>
		void SetResource(GfxResPtr<R>&& resource) noexcept;

		virtual void Bind(Graphics& gfx) const;
	};

#pragma region Functions
	template<typename R>
	R* IVisual::GetResource() noexcept
	{
		for (auto& bind : binds)
			if (auto res = dynamic_cast<R*>(&bind))
				return res;
		return nullptr;
	}

	template<typename R>
	void IVisual::SetResource(GfxResPtr<R>&& resource) noexcept
	{
		for (U64 i = 0; i < binds.size(); ++i)
		{
			if (binds.at(i).CastDynamic<R>() != nullptr)
			{
				binds.at(i) = std::move(resource);
				return;
			}
		}
	}
#pragma endregion
}