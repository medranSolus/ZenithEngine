#pragma once
#include "Source.h"

namespace ZE::GFX::Pipeline::RenderPass::Base
{
	template<typename T>
	class SourceDirectBindable : public Source
	{
		static_assert(std::is_base_of_v<GFX::Resource::IBindable, T>, "SourceDirectBindable target type must be a IBindable type!");

		GfxResPtr<T>& bind;

	public:
		constexpr SourceDirectBindable(std::string&& name, GfxResPtr<T>& bind)
			: Source(std::forward<std::string>(name)), bind(bind) {}
		SourceDirectBindable(SourceDirectBindable&&) = default;
		SourceDirectBindable(const SourceDirectBindable&) = default;
		SourceDirectBindable& operator=(SourceDirectBindable&&) = default;
		SourceDirectBindable& operator=(const SourceDirectBindable&) = default;
		virtual ~SourceDirectBindable() = default;

		static std::unique_ptr<Source> Make(std::string&& name, GfxResPtr<T>& bind);

		constexpr GfxResPtr<GFX::Resource::IBindable> LinkBindable() override { return bind.CastStatic<GFX::Resource::IBindable>(); }
	};

#pragma region Functions
	template<typename T>
	std::unique_ptr<Source> SourceDirectBindable<T>::Make(std::string&& name, GfxResPtr<T>& bind)
	{
		return std::make_unique<SourceDirectBindable>(std::forward<std::string>(name), bind);
	}
#pragma endregion
}