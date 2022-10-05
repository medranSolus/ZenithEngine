#pragma once
#include "Settings.h"

namespace ZE::GFX::API
{
	// Wrapper for proper graphics API implementations
	template<typename D11, typename D12, typename GL, typename VK>
	union Backend final
	{
		D11 dx11;
		D12 dx12;
		GL gl;
		VK vk;

		constexpr Backend() noexcept { Init(); }
		constexpr Backend(Backend&& b) noexcept;
		constexpr Backend(const Backend& b) noexcept;
		constexpr Backend& operator=(Backend&& b) noexcept;
		constexpr Backend& operator=(const Backend& b) noexcept;
		~Backend() { Delete(); }

		template<typename ...Params>
		constexpr void Init(Params&& ...p) { Init(Settings::GetGfxApi(), std::forward<Params>(p)...); }
		template<typename ...Params>
		constexpr void Switch(ApiType nextApi, Params&& ...p) { Delete(); Init(nextApi, std::forward<Params>(p)...); }

		template<typename ...Params>
		constexpr void Init(ApiType type, Params&& ...p);
		constexpr void Delete() noexcept;
	};

#pragma region Functions
	template<typename D11, typename D12, typename GL, typename VK>
	constexpr Backend<D11, D12, GL, VK>::Backend(Backend&& b) noexcept
	{
		switch (Settings::GetGfxApi())
		{
		default:
			ZE_ASSERT(false, "Unhandled enum value!");
		case ApiType::DX11:
		{
			new(&dx11) D11(std::move(b.dx11));
			break;
		}
		case ApiType::DX12:
		{
			new(&dx12) D12(std::move(b.dx12));
			break;
		}
		case ApiType::OpenGL:
		{
			new(&gl) GL(std::move(b.gl));
			break;
		}
		case ApiType::Vulkan:
		{
			new(&vk) VK(std::move(b.vk));
			break;
		}
		}
	}

	template<typename D11, typename D12, typename GL, typename VK>
	constexpr Backend<D11, D12, GL, VK>::Backend(const Backend& b) noexcept
	{
		switch (Settings::GetGfxApi())
		{
		default:
			ZE_ASSERT(false, "Unhandled enum value!");
		case ApiType::DX11:
		{
			new(&dx11) D11(b.dx11);
			break;
		}
		case ApiType::DX12:
		{
			new(&dx12) D12(b.dx12);
			break;
		}
		case ApiType::OpenGL:
		{
			new(&gl) GL(b.gl);
			break;
		}
		case ApiType::Vulkan:
		{
			new(&vk) VK(b.vk);
			break;
		}
		}
	}

	template<typename D11, typename D12, typename GL, typename VK>
	constexpr Backend<D11, D12, GL, VK>& Backend<D11, D12, GL, VK>::operator=(Backend&& b) noexcept
	{
		switch (Settings::GetGfxApi())
		{
		default:
			ZE_ASSERT(false, "Unhandled enum value!");
		case ApiType::DX11:
		{
			dx11 = std::move(b.dx11);
			break;
		}
		case ApiType::DX12:
		{
			dx12 = std::move(b.dx12);
			break;
		}
		case ApiType::OpenGL:
		{
			gl = std::move(b.gl);
			break;
		}
		case ApiType::Vulkan:
		{
			vk = std::move(b.vk);
			break;
		}
		}
		return *this;
	}

	template<typename D11, typename D12, typename GL, typename VK>
	constexpr Backend<D11, D12, GL, VK>& Backend<D11, D12, GL, VK>::operator=(const Backend& b) noexcept
	{
		switch (Settings::GetGfxApi())
		{
		default:
			ZE_ASSERT(false, "Unhandled enum value!");
		case ApiType::DX11:
		{
			dx11 = b.dx11;
			break;
		}
		case ApiType::DX12:
		{
			dx12 = b.dx12;
			break;
		}
		case ApiType::OpenGL:
		{
			gl = b.gl;
			break;
		}
		case ApiType::Vulkan:
		{
			vk = b.vk;
			break;
		}
		}
		return *this;
	}

	template<typename D11, typename D12, typename GL, typename VK>
	template<typename ...Params>
	constexpr void Backend<D11, D12, GL, VK>::Init(ApiType type, Params&& ...p)
	{
		switch (type)
		{
		default:
			ZE_ASSERT(false, "Unhandled enum value!");
		case ApiType::DX11:
		{
			new(&dx11) D11(std::forward<Params>(p)...);
			break;
		}
		case ApiType::DX12:
		{
			new(&dx12) D12(std::forward<Params>(p)...);
			break;
		}
		case ApiType::OpenGL:
		{
			new(&gl) GL(std::forward<Params>(p)...);
			break;
		}
		case ApiType::Vulkan:
		{
			new(&vk) VK(std::forward<Params>(p)...);
			break;
		}
		}
	}

	template<typename D11, typename D12, typename GL, typename VK>
	constexpr void Backend<D11, D12, GL, VK>::Delete() noexcept
	{
		switch (Settings::GetGfxApi())
		{
		default:
			ZE_ASSERT(false, "Unhandled enum value!");
		case ApiType::DX11:
		{
			dx11.~D11();
			break;
		}
		case ApiType::DX12:
		{
			dx12.~D12();
			break;
		}
		case ApiType::OpenGL:
		{
			gl.~GL();
			break;
		}
		case ApiType::Vulkan:
		{
			vk.~VK();
			break;
		}
		}
	}
#pragma endregion
}

// Type for proper graphics API implementations for all current APIs
#define ZE_API_BACKEND_TYPE(type) ZE::GFX::API::Backend< \
	ZE::GFX::API::DX11::##type##, \
	ZE::GFX::API::DX12::##type##, \
	ZE::GFX::API::DX11::##type##, \
	ZE::GFX::API::VK::##type##>

// Name of backend variable
#define ZE_API_BACKEND_VAR backend
// Wrapper adding backend implementation variable
#define ZE_API_BACKEND(type) ZE_API_BACKEND_TYPE(type) ZE_API_BACKEND_VAR

// Adds ability to access underlying graphics backend implementation via Get() method
#define ZE_API_BACKEND_GET(type) \
	constexpr ZE_API_BACKEND_TYPE(type)& Get() noexcept { return ZE_API_BACKEND_VAR; } \
	constexpr const ZE_API_BACKEND_TYPE(type)& Get() const noexcept { return ZE_API_BACKEND_VAR; }

// Extended wrapper for calling methods on currently active API implementation
#define ZE_API_BACKEND_CALL_EX(variable, ret, function, ...) \
	switch (Settings::GetGfxApi()) \
	{ \
	case ZE::GFX::API::ApiType::DX11: \
	{ \
		##ret## ##variable##.dx11.##function##(__VA_ARGS__); \
		break; \
	} \
	case ZE::GFX::API::ApiType::DX12: \
	{ \
		##ret## ##variable##.dx12.##function##(__VA_ARGS__); \
		break; \
	} \
	case ZE::GFX::API::ApiType::OpenGL: \
	{ \
		##ret## ##variable##.gl.##function##(__VA_ARGS__); \
		break; \
	} \
	case ZE::GFX::API::ApiType::Vulkan: \
	{ \
		##ret## ##variable##.vk.##function##(__VA_ARGS__); \
		break; \
	} \
	default: \
	{ \
		ZE_ASSERT(false, "Unhandled enum value!"); \
		##ret## {}; \
		break; \
	} \
	}

// Wrapper for calling methods on currently active API implementation
#define ZE_API_BACKEND_CALL(function, ...) ZE_API_BACKEND_CALL_EX(ZE_API_BACKEND_VAR, , function, __VA_ARGS__)
// Wrapper for calling methods on currently active API implementation and getting return value
#define ZE_API_BACKEND_CALL_RET(returnVariable, function, ...) ZE_API_BACKEND_CALL_EX(ZE_API_BACKEND_VAR, returnVariable=, function, __VA_ARGS__)