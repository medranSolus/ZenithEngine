#pragma once
#include "Source.h"

namespace GFX::Pipeline::RenderPass::Base
{
	template<typename T>
	class SourceDirectBindable : public Source
	{
		static_assert(std::is_base_of_v<GFX::Resource::IBindable, T>, "SourceDirectBindable target type must be a IBindable type!");

		GfxResPtr<T>& bind;

	public:
		inline SourceDirectBindable(const std::string& name, GfxResPtr<T>& bind) : Source(name), bind(bind) {}
		virtual ~SourceDirectBindable() = default;

		static inline std::unique_ptr<Source> Make(const std::string& name, GfxResPtr<T>& bind) { return std::make_unique<SourceDirectBindable>(name, bind); }

		inline GfxResPtr<GFX::Resource::IBindable> LinkBindable() override { return bind.CastStatic<GFX::Resource::IBindable>(); }
	};
}