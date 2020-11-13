#pragma once
#include "ConstBufferTransform.h"
#include "VertexLayout.h"
#include "RenderChannels.h"

namespace GFX::Visual
{
	class IVisual : public Probe::IProbeable
	{
		std::vector<GfxResPtr<Resource::IBindable>> binds;

	protected:
		GfxResPtr<Resource::ConstBufferTransform> transformBuffer;

		IVisual() = default;
		IVisual(const IVisual&) = default;
		IVisual& operator=(const IVisual&) = default;

	public:
		virtual ~IVisual() = default;

		inline void AddBind(GfxResPtr<Resource::IBindable>&& bind) noexcept { binds.emplace_back(std::forward<GfxResPtr<Resource::IBindable>&&>(bind)); }
		inline DirectX::XMFLOAT3 GetTransformPos() const noexcept { return transformBuffer->GetPos(); }
		inline DirectX::XMMATRIX GetTransform() const noexcept { return transformBuffer->GetTransform(); }
		virtual inline void SetTransformBuffer(Graphics& gfx, const GfxObject& parent) { transformBuffer = GfxResPtr<Resource::ConstBufferTransform>(gfx, parent); }
		virtual inline void Bind(Graphics& gfx, RenderChannel mode) { Bind(gfx); }

		template<typename R>
		R* GetResource() noexcept;
		template<typename R>
		void SetResource(GfxResPtr<R>&& resource) noexcept;

		virtual void Bind(Graphics& gfx);
	};

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
		for (size_t i = 0; i < binds.size(); ++i)
		{
			if (binds.at(i).CastDynamic<R>() != nullptr)
			{
				binds.at(i) = std::move(resource);
				return;
			}
		}
	}
}