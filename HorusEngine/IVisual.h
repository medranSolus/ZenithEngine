#pragma once
#include "IBindable.h"

namespace GFX::Visual
{
	class IVisual : public IProbeable
	{
		std::vector<std::shared_ptr<Resource::IBindable>> binds;

	public:
		IVisual() = default;
		IVisual(const IVisual&) = default;
		IVisual& operator=(const IVisual&) = default;
		virtual ~IVisual() = default;

		inline void AddBind(std::shared_ptr<Resource::IBindable> bind) noexcept { binds.emplace_back(std::move(bind)); }

		template<typename R>
		R* GetResource() noexcept;
		template<typename R>
		void SetResource(std::shared_ptr<R> resource) noexcept;

		virtual void Bind(Graphics& gfx) noexcept;
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
	void IVisual::SetResource(std::shared_ptr<R> resource) noexcept
	{
		for (size_t i = 0; i < binds.size(); ++i)
		{
			if (dynamic_cast<R*>(binds.at(i).get()) != nullptr)
			{
				binds.at(i) = resource;
				return;
			}
		}
	}
}