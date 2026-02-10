#pragma once
#include "Settings.h"

// Dummy macro to silence warnings about missings macro arguments
#define ZE_RHI_DUMMY 

// Helpers for selecting which implementation will be available
#if _ZE_RHI_DX11
#	define ZE_RHI_DX11_TYPE(var) D11 var
#	define ZE_RHI_DX11_TEMPLATE_TYPE typename ZE_RHI_DX11_TYPE(ZE_RHI_DUMMY)
#else
#	define ZE_RHI_DX11_TYPE(var)
#	define ZE_RHI_DX11_TEMPLATE_TYPE
#endif
#if _ZE_RHI_DX12
#	define ZE_RHI_DX12_TYPE(var) D12 var
#	define ZE_RHI_DX12_TEMPLATE_TYPE typename ZE_RHI_DX12_TYPE(ZE_RHI_DUMMY)
#else
#	define ZE_RHI_DX12_TYPE(var)
#	define ZE_RHI_DX12_TEMPLATE_TYPE
#endif
#if _ZE_RHI_GL
#	define ZE_RHI_GL_TYPE(var) GL var
#	define ZE_RHI_GL_TEMPLATE_TYPE typename ZE_RHI_GL_TYPE(ZE_RHI_DUMMY)
#else
#	define ZE_RHI_GL_TYPE(var)
#	define ZE_RHI_GL_TEMPLATE_TYPE
#endif
#if _ZE_RHI_VK
#	define ZE_RHI_VK_TYPE(var) VK var
#	define ZE_RHI_VK_TEMPLATE_TYPE typename ZE_RHI_VK_TYPE(ZE_RHI_DUMMY)
#else
#	define ZE_RHI_VK_TYPE(var)
#	define ZE_RHI_VK_TEMPLATE_TYPE
#endif

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

// Helpers for making template functions definitions later
#define ZE_RHI_TEMPLATE_HEADER template<ZE_RHI_DX11_TEMPLATE_TYPE ZE_RHI_DX11_COMMA ZE_RHI_DX12_TEMPLATE_TYPE ZE_RHI_DX12_COMMA ZE_RHI_GL_TEMPLATE_TYPE ZE_RHI_GL_COMMA ZE_RHI_VK_TEMPLATE_TYPE ZE_RHI_VK_COMMA>
#define ZE_RHI_TEMPLATE_SPEC ZE_RHI_DX11_TYPE(ZE_RHI_DUMMY) ZE_RHI_DX11_COMMA ZE_RHI_DX12_TYPE(ZE_RHI_DUMMY) ZE_RHI_DX12_COMMA ZE_RHI_GL_TYPE(ZE_RHI_DUMMY) ZE_RHI_GL_COMMA ZE_RHI_VK_TYPE(ZE_RHI_DUMMY) ZE_RHI_VK_COMMA

namespace ZE::RHI
{
	// Wrapper for proper graphics API implementations
	ZE_RHI_TEMPLATE_HEADER
	union Backend final
	{
		ZE_RHI_DX11_TYPE(dx11);
		ZE_RHI_DX12_TYPE(dx12)
		ZE_RHI_GL_TYPE(gl);
		ZE_RHI_VK_TYPE(vk);

		constexpr Backend() noexcept;
		constexpr Backend(Backend&& b) noexcept;
		constexpr Backend& operator=(Backend&& b) noexcept;
		ZE_CLASS_NO_COPY(Backend); // Deep copies are complex and need to be performed manually
		~Backend();

		template<typename ...Params>
		static Expected<Backend> Create(Params&& ...p) noexcept;
	};

#pragma region Functions
	ZE_RHI_TEMPLATE_HEADER
	constexpr Backend<ZE_RHI_TEMPLATE_SPEC>::Backend() noexcept
	{
		switch (Settings::GetGfxApi())
		{
		default:
			ZE_ENUM_UNHANDLED();
		case ApiType::None:
			break;
#if _ZE_RHI_DX11
		case ApiType::DX11:
		{
			new(&dx11) D11();
			break;
		}
#endif
#if _ZE_RHI_DX12
		case ApiType::DX12:
		{
			new(&dx12) D12();
			break;
		}
#endif
#if _ZE_RHI_GL
		case ApiType::OpenGL:
		{
			new(&gl) GL();
			break;
		}
#endif
#if _ZE_RHI_VK
		case ApiType::Vulkan:
		{
			new(&vk) VK();
			break;
		}
#endif
		}
	}

	ZE_RHI_TEMPLATE_HEADER
	constexpr Backend<ZE_RHI_TEMPLATE_SPEC>::Backend(Backend&& b) noexcept
	{
		switch (Settings::GetGfxApi())
		{
		default:
			ZE_ENUM_UNHANDLED();
		case ApiType::None:
			break;
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

	ZE_RHI_TEMPLATE_HEADER
	constexpr Backend<ZE_RHI_TEMPLATE_SPEC>& Backend<ZE_RHI_TEMPLATE_SPEC>::operator=(Backend&& b) noexcept
	{
		switch (Settings::GetGfxApi())
		{
		default:
			ZE_ENUM_UNHANDLED();
		case ApiType::None:
			break;
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

	ZE_RHI_TEMPLATE_HEADER
	Backend<ZE_RHI_TEMPLATE_SPEC>::~Backend()
	{
		switch (Settings::GetGfxApi())
		{
		default:
			ZE_ENUM_UNHANDLED();
		case ApiType::None:
			break;
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

// Helper for creating backend specific objects
#define ZE_RHI_CREATE_BACKEND_EX(Impl, ...) \
	Expected<Impl> __impl = Impl##::Create(__VA_ARGS__); \
	if (!__impl) return std::unexpected(__impl.error());

	ZE_RHI_TEMPLATE_HEADER
	template<typename ...Params>
	Expected<Backend<ZE_RHI_TEMPLATE_SPEC>> Backend<ZE_RHI_TEMPLATE_SPEC>::Create(Params&& ...p) noexcept
	{
#define ZE_RHI_CREATE_BACKEND_IMPL(Impl, var) ZE_RHI_CREATE_BACKEND_EX(Impl, std::forward<Params>(p)...) backend.var = std::move(*__impl)

		Backend backend = {};
		switch (Settings::GetGfxApi())
		{
		default:
			ZE_ENUM_UNHANDLED();
		case ApiType::None:
			break;
#if _ZE_RHI_DX11
		case ApiType::DX11:
		{
			ZE_RHI_CREATE_BACKEND_IMPL(D11, dx11);
			break;
		}
#endif
#if _ZE_RHI_DX12
		case ApiType::DX12:
		{
			ZE_RHI_CREATE_BACKEND_IMPL(D12, dx12);
			break;
		}
#endif
#if _ZE_RHI_GL
		case ApiType::OpenGL:
		{
			ZE_RHI_CREATE_BACKEND_IMPL(GL, gl);
			break;
		}
#endif
#if _ZE_RHI_VK
		case ApiType::Vulkan:
		{
			ZE_RHI_CREATE_BACKEND_IMPL(VK, vk);
			break;
		}
#endif
		}
		return backend;
#undef ZE_RHI_CREATE_BACKEND_IMPL
	}
#pragma endregion
}

#undef ZE_RHI_DUMMY

// Helpers for manipulating with active RHI implementations
#if _ZE_RHI_DX11
#	define ZE_GET_DX11_RHI_TYPE(type) ZE::RHI::DX11::##type ZE_RHI_DX11_COMMA
#	define ZE_RHI_DX11_SWITCH_CALL(variable, ret, function, ...) ret## (##variable##.dx11.##function##(__VA_ARGS__))
#else
#	define ZE_GET_DX11_RHI_TYPE(type)
#	define ZE_RHI_DX11_SWITCH_CALL(variable, ret, function, ...) ZE_FAIL("DirectX 11 has been disabled!")
#endif
#if _ZE_RHI_DX12
#	define ZE_GET_DX12_RHI_TYPE(type) ZE::RHI::DX12::##type ZE_RHI_DX12_COMMA
#	define ZE_RHI_DX12_SWITCH_CALL(variable, ret, function, ...) ret## (##variable##.dx12.##function##(__VA_ARGS__))
#else
#	define ZE_GET_DX12_RHI_TYPE(type)
#	define ZE_RHI_DX12_SWITCH_CALL(variable, ret, function, ...) ZE_FAIL("DirectX 12 has been disabled!")
#endif
#if _ZE_RHI_GL
#	define ZE_GET_GL_RHI_TYPE(type) ZE::RHI::GL::##type ZE_RHI_GL_COMMA
#	define ZE_RHI_GL_SWITCH_CALL(variable, ret, function, ...) ret## (##variable##.gl.##function##(__VA_ARGS__))
#else
#	define ZE_GET_GL_RHI_TYPE(type)
#	define ZE_RHI_GL_SWITCH_CALL(variable, ret, function, ...) ZE_FAIL("OpenGL has been disabled!")
#endif
#if _ZE_RHI_VK
#	define ZE_GET_VK_RHI_TYPE(type) ZE::RHI::VK::##type ZE_RHI_VK_COMMA
#	define ZE_RHI_VK_SWITCH_CALL(variable, ret, function, ...) ret## (##variable##.vk.##function##(__VA_ARGS__))
#else
#	define ZE_GET_VK_RHI_TYPE(type)
#	define ZE_RHI_VK_SWITCH_CALL(variable, ret, function, ...) ZE_FAIL("Vulkan has been disabled!")
#endif

// Type for proper graphics API implementations for all current APIs
#define ZE_RHI_BACKEND_TYPE(type) ZE::RHI::Backend<ZE_GET_DX11_RHI_TYPE(type) ZE_GET_DX12_RHI_TYPE(type) ZE_GET_GL_RHI_TYPE(type) ZE_GET_VK_RHI_TYPE(type)>

// Name of backend variable
#define ZE_RHI_BACKEND_VAR __backend
// Wrapper adding backend implementation variable
#define ZE_RHI_BACKEND(type) ZE_RHI_BACKEND_TYPE(type) ZE_RHI_BACKEND_VAR

// Adds ability to access underlying graphics backend implementation via Get() method
#define ZE_RHI_BACKEND_GET(type) \
	constexpr ZE_RHI_BACKEND_TYPE(type)& Get() noexcept { return ZE_RHI_BACKEND_VAR; } \
	constexpr const ZE_RHI_BACKEND_TYPE(type)& Get() const noexcept { return ZE_RHI_BACKEND_VAR; }

// Wrapper for creating backend objects
#define ZE_RHI_BACKEND_CREATE(type, ...) \
	ZE_RHI_CREATE_BACKEND_EX(ZE_RHI_BACKEND_TYPE(type), __VA_ARGS__); \
	type __rhi = {}; \
	__rhi.ZE_RHI_BACKEND_VAR = std::move(*__impl); \
	return __rhi

// Extended wrapper for calling methods on currently active API implementation
#define ZE_RHI_BACKEND_CALL_EX(variable, ret, function, ...) \
	switch (Settings::GetGfxApi()) \
	{ \
	case ZE::RHI::ApiType::None: \
	{ \
		##ret## {}; \
		break; \
	} \
	case ZE::RHI::ApiType::DX11: \
	{ \
		ZE_RHI_DX11_SWITCH_CALL(variable, ret, function, __VA_ARGS__); \
		break; \
	} \
	case ZE::RHI::ApiType::DX12: \
	{ \
		ZE_RHI_DX12_SWITCH_CALL(variable, ret, function, __VA_ARGS__); \
		break; \
	} \
	case ZE::RHI::ApiType::OpenGL: \
	{ \
		ZE_RHI_GL_SWITCH_CALL(variable, ret, function, __VA_ARGS__); \
		break; \
	} \
	case ZE::RHI::ApiType::Vulkan: \
	{ \
		ZE_RHI_VK_SWITCH_CALL(variable, ret, function, __VA_ARGS__); \
		break; \
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
// Wrapper for calling methods on currently active API implementation and returning with it's value
#define ZE_RHI_BACKEND_CALL_RET(function, ...) ZE_RHI_BACKEND_CALL_EX(ZE_RHI_BACKEND_VAR, return, function, __VA_ARGS__)
// Wrapper for calling methods on currently active API implementation and getting return value
#define ZE_RHI_BACKEND_CALL_RET_VAR(returnVariable, function, ...) ZE_RHI_BACKEND_CALL_EX(ZE_RHI_BACKEND_VAR, returnVariable=, function, __VA_ARGS__)