#pragma once
#include "Source.h"

namespace GFX::Pipeline::RenderPass::Base
{
	template<typename T>
	class SourceDirectBindabke : public Source
	{
		static_assert(std::is_base_of_v<GFX::Resource::IBindable, T>, "SourceDirectBindabke target type must be a IBindable type!");

		std::shared_ptr<T>& bind;
		bool linked = false;

	public:
		inline SourceDirectBindabke(const std::string& name, std::shared_ptr<T>& bind) : Source(name), bind(bind) {}
		virtual ~SourceDirectBindabke() = default;

		static inline std::unique_ptr<Source> Make(const std::string& name, std::shared_ptr<T>& bind) { return std::make_unique<SourceDirectBindabke>(name, bind); }

		inline std::shared_ptr<GFX::Resource::IBindable> LinkBindable() override { return bind; }
	};
}