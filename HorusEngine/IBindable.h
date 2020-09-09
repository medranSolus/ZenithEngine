#pragma once
#include "IProbeable.h"

namespace GFX::Resource
{
	template<typename T>
	struct is_resolvable_by_codex
	{
		static constexpr bool generate{ false };
	};

	class IBindable : public Probe::IProbeable
	{
	protected:
		static inline ID3D11DeviceContext* GetContext(Graphics& gfx) noexcept { return gfx.context.Get(); }
		static inline ID3D11Device* GetDevice(Graphics& gfx) noexcept { return gfx.device.Get(); }

	public:
		IBindable() = default;
		IBindable(const IBindable&) = delete;
		IBindable& operator=(const IBindable&) = delete;
		virtual ~IBindable() = default;

		template<typename T, typename ...Params>
		static constexpr std::string GenerateRID(Params&& ...p) noexcept
		{
			static_assert(is_resolvable_by_codex<T>::generate == true, "Class does not implement static method GenerateRID(...)!");
			return T::template GenerateRID(std::forward<Params>(p)...);
		}

		inline void Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override {}

		virtual void Bind(Graphics& gfx) = 0;
		virtual std::string GetRID() const noexcept = 0;
	};
}