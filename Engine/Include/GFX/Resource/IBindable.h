#pragma once
#include "GFX/Probe/IProbeable.h"
#include "WinAPI/WinAPI.h"
#pragma warning(disable:4265)
#include <wrl.h>
#pragma warning(default:4265)
#include <d3d11.h>

namespace ZE::GFX::Resource
{
	template<typename T>
	struct is_resolvable_by_codex
	{
		static constexpr bool GENERATE_ID{ false };
	};

	enum ShaderType : char { Vertex = 'V', Geometry = 'G', Pixel = 'P', Compute = 'C' };

	class IBindable : public Probe::IProbeable
	{
	protected:
		static ID3D11DeviceContext* GetContext(Graphics& gfx) noexcept;
		static ID3D11Device* GetDevice(Graphics& gfx) noexcept;

	public:
		IBindable() = default;
		IBindable(IBindable&&) = default;
		IBindable(const IBindable&) = delete;
		IBindable& operator=(IBindable&&) = default;
		IBindable& operator=(const IBindable&) = delete;
		virtual ~IBindable() = default;

		static constexpr const char* GetNoCodexRID() noexcept { return "?"; }

		constexpr bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return false; }
		virtual std::string GetRID() const noexcept { return GetNoCodexRID(); }

		template<typename T, typename ...Params>
		static constexpr std::string GenerateRID(Params&& ...p) noexcept;

		virtual void Bind(Graphics& gfx) const = 0;
	};

#pragma region Functions
	template<typename T, typename ...Params>
	constexpr std::string IBindable::GenerateRID(Params&& ...p) noexcept
	{
		static_assert(is_resolvable_by_codex<T>::GENERATE_ID == true, "Class does not implement static method GenerateRID(...)!");
		return T::template GenerateRID(std::forward<Params>(p)...);
	}
#pragma endregion
}