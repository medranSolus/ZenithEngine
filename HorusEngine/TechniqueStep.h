#pragma once
#include "RenderCommander.h"
#include "IBindable.h"

namespace GFX::Pipeline
{
	class TechniqueStep
	{
		size_t targetPass;
		std::vector<std::shared_ptr<Resource::IBindable>> binds;

	public:
		inline TechniqueStep(size_t targetPass) noexcept : targetPass(targetPass) {}

		inline void AddBind(std::shared_ptr<Resource::IBindable> bind) noexcept { binds.emplace_back(std::move(bind)); }
		inline void Submit(RenderCommander& renderer, Shape::BaseShape& shape) noexcept { renderer.Add({ &shape, this }, targetPass); }

		template<typename R>
		R* GetResource() noexcept;
		template<typename R>
		void SetResource(std::shared_ptr<R> resource) noexcept;

		void Bind(Graphics& gfx) noexcept;
	};

	template<typename R>
	R* TechniqueStep::GetResource() noexcept
	{
		for (auto& bind : binds)
			if (auto res = dynamic_cast<R*>(&bind))
				return res;
		return nullptr;
	}

	template<typename R>
	void TechniqueStep::SetResource(std::shared_ptr<R> resource) noexcept
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