#pragma once
#include "GraphicsResource.h"
#include "IProbeable.h"

namespace GFX::Resource
{
	template<typename T>
	struct is_resolvable_by_codex
	{
		static constexpr bool generate{ false };
	};

	class IBindable : public GraphicsResource, public Probe::IProbeable
	{
	public:
		virtual ~IBindable() = default;

		template<typename T, typename ...Params>
		static constexpr std::string GenerateRID(Params&& ...p) noexcept
		{
			static_assert(is_resolvable_by_codex<T>::generate == true, "Class does not implement static method GenerateRID(...)!");
			return T::template GenerateRID(std::forward<Params>(p)...);
		}

		virtual void Bind(Graphics& gfx) noexcept = 0;
		virtual std::string GetRID() const noexcept = 0;
		void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override {}
	};
}