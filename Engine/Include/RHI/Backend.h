#pragma once
#include "Settings.h"

// Helpers for selecting where coma between implementations will appear
#if _ZE_RHI_DX11 && (_ZE_RHI_DX12 || _ZE_RHI_GL || _ZE_RHI_VK)
#	define ZE_RHI_DX11_COMMA ,
#else
#	define ZE_RHI_DX11_COMMA
#endif
#if _ZE_RHI_DX12 && (_ZE_RHI_GL || _ZE_RHI_VK)
#	define ZE_RHI_DX12_COMMA ,
#else
#	define ZE_RHI_DX12_COMMA
#endif
#if _ZE_RHI_GL && _ZE_RHI_VK
#	define ZE_RHI_GL_COMMA ,
#else
#	define ZE_RHI_GL_COMMA
#endif
#if _ZE_RHI_VK && 0
#	define ZE_RHI_VK_COMMA ,
#else
#	define ZE_RHI_VK_COMMA
#endif

namespace ZE::RHI
{
	// Wrapper for proper graphics API implementations
	template<
#if _ZE_RHI_DX11
		typename D11 ZE_RHI_DX11_COMMA
#endif
#if _ZE_RHI_DX12
		typename D12 ZE_RHI_DX12_COMMA
#endif
#if _ZE_RHI_GL
		typename GL ZE_RHI_GL_COMMA
#endif
#if _ZE_RHI_VK
		typename VK ZE_RHI_VK_COMMA
#endif
	>
	union Backend final
	{
#if _ZE_RHI_DX11
		D11 dx11;
#endif
#if _ZE_RHI_DX12
		D12 dx12;
#endif
#if _ZE_RHI_GL
		GL gl;
#endif
#if _ZE_RHI_VK
		VK vk;
#endif

		constexpr Backend() noexcept { Init(); }
		constexpr Backend(Backend&& b) noexcept
		{
			switch (Settings::GetGfxApi())
			{
			default:
				ZE_ENUM_UNHANDLED();
#if _ZE_RHI_DX11
			case ApiType::DX11:
			{
				new(&dx11) D11(std::move(b.dx11));
				break;
			}
#endif
#if _ZE_RHI_DX12
			case ApiType::DX12:
			{
				new(&dx12) D12(std::move(b.dx12));
				break;
			}
#endif
#if _ZE_RHI_GL
			case ApiType::OpenGL:
			{
				new(&gl) GL(std::move(b.gl));
				break;
			}
#endif
#if _ZE_RHI_VK
			case ApiType::Vulkan:
			{
				new(&vk) VK(std::move(b.vk));
				break;
			}
#endif
			}
		}
		constexpr Backend(const Backend& b) noexcept
		{
			switch (Settings::GetGfxApi())
			{
			default:
				ZE_ENUM_UNHANDLED();
#if _ZE_RHI_DX11
			case ApiType::DX11:
			{
				new(&dx11) D11(b.dx11);
				break;
			}
#endif
#if _ZE_RHI_DX12
			case ApiType::DX12:
			{
				new(&dx12) D12(b.dx12);
				break;
			}
#endif
#if _ZE_RHI_GL
			case ApiType::OpenGL:
			{
				new(&gl) GL(b.gl);
				break;
			}
#endif
#if _ZE_RHI_VK
			case ApiType::Vulkan:
			{
				new(&vk) VK(b.vk);
				break;
			}
#endif
			}
		}
		constexpr Backend& operator=(Backend&& b) noexcept
		{
			switch (Settings::GetGfxApi())
			{
			default:
				ZE_ENUM_UNHANDLED();
#if _ZE_RHI_DX11
			case ApiType::DX11:
			{
				dx11 = std::move(b.dx11);
				break;
			}
#endif
#if _ZE_RHI_DX12
			case ApiType::DX12:
			{
				dx12 = std::move(b.dx12);
				break;
			}
#endif
#if _ZE_RHI_GL
			case ApiType::OpenGL:
			{
				gl = std::move(b.gl);
				break;
			}
#endif
#if _ZE_RHI_VK
			case ApiType::Vulkan:
			{
				vk = std::move(b.vk);
				break;
			}
#endif
			}
			return *this;
		}
		constexpr Backend& operator=(const Backend& b) noexcept
		{
			switch (Settings::GetGfxApi())
			{
			default:
				ZE_ENUM_UNHANDLED();
#if _ZE_RHI_DX11
			case ApiType::DX11:
			{
				dx11 = b.dx11;
				break;
			}
#endif
#if _ZE_RHI_DX12
			case ApiType::DX12:
			{
				dx12 = b.dx12;
				break;
			}
#endif
#if _ZE_RHI_GL
			case ApiType::OpenGL:
			{
				gl = b.gl;
				break;
			}
#endif
#if _ZE_RHI_VK
			case ApiType::Vulkan:
			{
				vk = b.vk;
				break;
			}
#endif
			}
			return *this;
		}
		~Backend() { Delete(); }

		template<typename ...Params>
		constexpr void Init(Params&& ...p) { Init(Settings::GetGfxApi(), std::forward<Params>(p)...); }
		template<typename ...Params>
		constexpr void Switch(ApiType nextApi, Params&& ...p) { Delete(); Init(nextApi, std::forward<Params>(p)...); }

		template<typename ...Params>
		constexpr void Init(ApiType type, Params&& ...p)
		{
			switch (type)
			{
			default:
				ZE_ENUM_UNHANDLED();
#if _ZE_RHI_DX11
			case ApiType::DX11:
			{
				new(&dx11) D11(std::forward<Params>(p)...);
				break;
			}
#endif
#if _ZE_RHI_DX12
			case ApiType::DX12:
			{
				new(&dx12) D12(std::forward<Params>(p)...);
				break;
			}
#endif
#if _ZE_RHI_GL
			case ApiType::OpenGL:
			{
				new(&gl) GL(std::forward<Params>(p)...);
				break;
			}
#endif
#if _ZE_RHI_VK
			case ApiType::Vulkan:
			{
				new(&vk) VK(std::forward<Params>(p)...);
				break;
			}
#endif
			}
		}
		constexpr void Delete() noexcept
		{
			switch (Settings::GetGfxApi())
			{
			default:
				ZE_ENUM_UNHANDLED();
#if _ZE_RHI_DX11
			case ApiType::DX11:
			{
				dx11.~D11();
				break;
			}
#endif
#if _ZE_RHI_DX12
			case ApiType::DX12:
			{
				dx12.~D12();
				break;
			}
#endif
#if _ZE_RHI_GL
			case ApiType::OpenGL:
			{
				gl.~GL();
				break;
			}
#endif
#if _ZE_RHI_VK
			case ApiType::Vulkan:
			{
				vk.~VK();
				break;
			}
#endif
			}
		}
	};
}

// Helpers for manipulating with active RHI implementations
#if _ZE_RHI_DX11
#	define ZE_GET_DX11_RHI_TYPE(type) ZE::RHI::DX11::##type ZE_RHI_DX11_COMMA
#	define ZE_RHI_DX11_SWITCH_CALL(variable, ret, function, ...) ret## ##variable##.dx11.##function##(__VA_ARGS__); break
#else
#	define ZE_GET_DX11_RHI_TYPE(type)
#	define ZE_RHI_DX11_SWITCH_CALL(variable, ret, function, ...) ZE_FAIL("DirectX 11 has been disabled!")
#endif
#if _ZE_RHI_DX12
#	define ZE_GET_DX12_RHI_TYPE(type) ZE::RHI::DX12::##type ZE_RHI_DX12_COMMA
#	define ZE_RHI_DX12_SWITCH_CALL(variable, ret, function, ...) ret## ##variable##.dx12.##function##(__VA_ARGS__); break
#else
#	define ZE_GET_DX12_RHI_TYPE(type)
#	define ZE_RHI_DX12_SWITCH_CALL(variable, ret, function, ...) ZE_FAIL("DirectX 12 has been disabled!")
#endif
#if _ZE_RHI_GL
#	define ZE_GET_GL_RHI_TYPE(type) ZE::RHI::GL::##type ZE_RHI_GL_COMMA
#	define ZE_RHI_GL_SWITCH_CALL(variable, ret, function, ...) ret## ##variable##.gl.##function##(__VA_ARGS__); break
#else
#	define ZE_GET_GL_RHI_TYPE(type)
#	define ZE_RHI_GL_SWITCH_CALL(variable, ret, function, ...) ZE_FAIL("OpenGL has been disabled!")
#endif
#if _ZE_RHI_VK
#	define ZE_GET_VK_RHI_TYPE(type) ZE::RHI::VK::##type ZE_RHI_VK_COMMA
#	define ZE_RHI_VK_SWITCH_CALL(variable, ret, function, ...) ret## ##variable##.vk.##function##(__VA_ARGS__); break
#else
#	define ZE_GET_VK_RHI_TYPE(type)
#	define ZE_RHI_VK_SWITCH_CALL(variable, ret, function, ...) ZE_FAIL("Vulkan has been disabled!")
#endif

// Type for proper graphics API implementations for all current APIs
#define ZE_RHI_BACKEND_TYPE(type) ZE::RHI::Backend<ZE_GET_DX11_RHI_TYPE(type) ZE_GET_DX12_RHI_TYPE(type) ZE_GET_GL_RHI_TYPE(type) ZE_GET_VK_RHI_TYPE(type)>

// Name of backend variable
#define ZE_RHI_BACKEND_VAR backend
// Wrapper adding backend implementation variable
#define ZE_RHI_BACKEND(type) ZE_RHI_BACKEND_TYPE(type) ZE_RHI_BACKEND_VAR

// Adds ability to access underlying graphics backend implementation via Get() method
#define ZE_RHI_BACKEND_GET(type) \
	constexpr ZE_RHI_BACKEND_TYPE(type)& Get() noexcept { return ZE_RHI_BACKEND_VAR; } \
	constexpr const ZE_RHI_BACKEND_TYPE(type)& Get() const noexcept { return ZE_RHI_BACKEND_VAR; }

// Extended wrapper for calling methods on currently active API implementation
#define ZE_RHI_BACKEND_CALL_EX(variable, ret, function, ...) \
	switch (Settings::GetGfxApi()) \
	{ \
	case ZE::RHI::ApiType::DX11: \
	{ \
		ZE_RHI_DX11_SWITCH_CALL(variable, ret, function, __VA_ARGS__); \
	} \
	case ZE::RHI::ApiType::DX12: \
	{ \
		ZE_RHI_DX12_SWITCH_CALL(variable, ret, function, __VA_ARGS__); \
	} \
	case ZE::RHI::ApiType::OpenGL: \
	{ \
		ZE_RHI_GL_SWITCH_CALL(variable, ret, function, __VA_ARGS__); \
	} \
	case ZE::RHI::ApiType::Vulkan: \
	{ \
		ZE_RHI_VK_SWITCH_CALL(variable, ret, function, __VA_ARGS__); \
	} \
	default: \
	{ \
		ZE_FAIL("Using not supported API!"); \
		##ret## {}; \
		break; \
	} \
	}

// Wrapper for calling methods on currently active API implementation
#define ZE_RHI_BACKEND_CALL(function, ...) ZE_RHI_BACKEND_CALL_EX(ZE_RHI_BACKEND_VAR, , function, __VA_ARGS__)
// Wrapper for calling methods on currently active API implementation and getting return value
#define ZE_RHI_BACKEND_CALL_RET(returnVariable, function, ...) ZE_RHI_BACKEND_CALL_EX(ZE_RHI_BACKEND_VAR, returnVariable=, function, __VA_ARGS__)