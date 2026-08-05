#pragma once
#include "Settings.h"

// Dummy macro to silence warnings about missings macro arguments
#define ZE_AHI_DUMMY 

// Helpers for selecting which implementation will be available
#if _ZE_AHI_OPENAL
#	define ZE_AHI_OPENAL_TYPE(var) OAL var
#	define ZE_AHI_OPENAL_TEMPLATE_TYPE typename ZE_AHI_OPENAL_TYPE(ZE_AHI_DUMMY)
#else
#	define ZE_AHI_OPENAL_TYPE(var)
#	define ZE_AHI_OPENAL_TEMPLATE_TYPE
#endif
#if _ZE_AHI_XAUDIO2
#	define ZE_AHI_XAUDIO2_TYPE(var) XA2 var
#	define ZE_AHI_XAUDIO2_TEMPLATE_TYPE typename ZE_AHI_XAUDIO2_TYPE(ZE_AHI_DUMMY)
#else
#	define ZE_AHI_XAUDIO2_TYPE(var)
#	define ZE_AHI_XAUDIO2_TEMPLATE_TYPE
#endif

// Helpers for selecting where coma between implementations will appear
#if _ZE_AHI_OPENAL && _ZE_AHI_XAUDIO2
#	define ZE_AHI_OPENAL_COMMA ,
#else
#	define ZE_AHI_OPENAL_COMMA
#endif
#if _ZE_AHI_XAUDIO2 && 0
#	define ZE_AHI_XAUDIO2_COMMA ,
#else
#	define ZE_AHI_XAUDIO2_COMMA
#endif

// Helpers for making template functions definitions later
#define ZE_AHI_TEMPLATE_HEADER template<ZE_AHI_OPENAL_TEMPLATE_TYPE ZE_AHI_OPENAL_COMMA ZE_AHI_XAUDIO2_TEMPLATE_TYPE ZE_AHI_XAUDIO2_COMMA>
#define ZE_AHI_TEMPLATE_SPEC ZE_AHI_OPENAL_TYPE(ZE_AHI_DUMMY) ZE_AHI_OPENAL_COMMA ZE_AHI_XAUDIO2_TYPE(ZE_AHI_DUMMY) ZE_AHI_XAUDIO2_COMMA

namespace ZE::AHI
{
	// Wrapper for proper graphics API implementations
	ZE_AHI_TEMPLATE_HEADER
	union Backend final
	{
		ZE_AHI_OPENAL_TYPE(oal);
		ZE_AHI_XAUDIO2_TYPE(xa2);

		constexpr Backend() noexcept;
		constexpr Backend(Backend&& b) noexcept;
		constexpr Backend& operator=(Backend&& b) noexcept;
		ZE_CLASS_NO_COPY(Backend); // Deep copies are complex and need to be performed manually
		~Backend();

		template<typename ...Params>
		static Expected<Backend> Create(Params&& ...p) noexcept;
	};

#pragma region Functions
	ZE_AHI_TEMPLATE_HEADER
	constexpr Backend<ZE_AHI_TEMPLATE_SPEC>::Backend() noexcept
	{
		switch (Settings::GetAudioApi())
		{
		default:
			ZE_ENUM_UNHANDLED();
		case ApiType::None:
			break;
#if _ZE_AHI_OPENAL
		case ApiType::OpenAL:
		{
			new(&oal) OAL();
			break;
		}
#endif
#if _ZE_AHI_XAUDIO2
		case ApiType::XAudio2:
		{
			new(&xa2) XA2();
			break;
		}
#endif
		}
	}

	ZE_AHI_TEMPLATE_HEADER
	constexpr Backend<ZE_AHI_TEMPLATE_SPEC>::Backend(Backend&& b) noexcept
	{
		switch (Settings::GetAudioApi())
		{
		default:
			ZE_ENUM_UNHANDLED();
		case ApiType::None:
			break;
#if _ZE_AHI_OPENAL
		case ApiType::OpenAL:
		{
			new(&oal) OAL(std::move(b.oal));
			break;
		}
#endif
#if _ZE_AHI_XAUDIO2
		case ApiType::XAudio2:
		{
			new(&xa2) XA2(std::move(b.xa2));
			break;
		}
#endif
		}
	}

	ZE_AHI_TEMPLATE_HEADER
	constexpr Backend<ZE_AHI_TEMPLATE_SPEC>& Backend<ZE_AHI_TEMPLATE_SPEC>::operator=(Backend&& b) noexcept
	{
		switch (Settings::GetAudioApi())
		{
		default:
			ZE_ENUM_UNHANDLED();
		case ApiType::None:
			break;
#if _ZE_AHI_OPENAL
		case ApiType::OpenAL:
		{
			oal = std::move(b.oal);
			break;
		}
#endif
#if _ZE_AHI_XAUDIO2
		case ApiType::XAudio2:
		{
			xa2 = std::move(b.xa2);
			break;
		}
#endif
		}
		return *this;
	}

	ZE_AHI_TEMPLATE_HEADER
	Backend<ZE_AHI_TEMPLATE_SPEC>::~Backend()
	{
		switch (Settings::GetAudioApi())
		{
		default:
			ZE_ENUM_UNHANDLED();
		case ApiType::None:
			break;
#if _ZE_AHI_OPENAL
		case ApiType::OpenAL:
		{
			oal.~OAL();
			break;
		}
#endif
#if _ZE_AHI_XAUDIO2
		case ApiType::XAudio2:
		{
			xa2.~XA2();
			break;
		}
#endif
		}
	}

	// Helper for creating backend specific objects
#define ZE_AHI_CREATE_BACKEND_EX(Impl, ...) \
	Expected<Impl> __impl = Impl##::Create(__VA_ARGS__); \
	if (!__impl) return std::unexpected(__impl.error());

	ZE_AHI_TEMPLATE_HEADER
	template<typename ...Params>
	Expected<Backend<ZE_AHI_TEMPLATE_SPEC>> Backend<ZE_AHI_TEMPLATE_SPEC>::Create(Params&& ...p) noexcept
	{
#define ZE_AHI_CREATE_BACKEND_IMPL(Impl, var) ZE_AHI_CREATE_BACKEND_EX(Impl, std::forward<Params>(p)...) backend.var = std::move(*__impl)

		Backend backend = {};
		switch (Settings::GetAudioApi())
		{
		default:
			ZE_ENUM_UNHANDLED();
		case ApiType::None:
			break;
#if _ZE_AHI_OPENAL
		case ApiType::OpenAL:
		{
			ZE_AHI_CREATE_BACKEND_IMPL(OAL, oal);
			break;
		}
#endif
#if _ZE_AHI_XAUDIO2
		case ApiType::XAudio2:
		{
			ZE_AHI_CREATE_BACKEND_IMPL(XA2, xa2);
			break;
		}
#endif
		}
		return backend;
#undef ZE_AHI_CREATE_BACKEND_IMPL
	}
#pragma endregion
}

#undef ZE_AHI_DUMMY

// Helpers for manipulating with active AHI implementations
#if _ZE_AHI_OPENAL
#	define ZE_GET_OPENAL_AHI_TYPE(type) ZE::AHI::OpenAL::##type ZE_AHI_OPENAL_COMMA
#	define ZE_AHI_OPENAL_SWITCH_CALL(variable, ret, function, ...) ret## (##variable##.oal.##function##(__VA_ARGS__))
#else
#	define ZE_GET_OPENAL_AHI_TYPE(type)
#	define ZE_AHI_OPENAL_SWITCH_CALL(variable, ret, function, ...) ZE_FAIL("OpenAL has been disabled!"); ret## {}
#endif
#if _ZE_AHI_XAUDIO2
#	define ZE_GET_XAUDIO2_AHI_TYPE(type) ZE::AHI::XAudio2::##type ZE_AHI_XAUDIO2_COMMA
#	define ZE_AHI_XAUDIO2_SWITCH_CALL(variable, ret, function, ...) ret## (##variable##.xa2.##function##(__VA_ARGS__))
#else
#	define ZE_GET_XAUDIO2_RHI_TYPE(type)
#	define ZE_AHI_XAUDIO2_SWITCH_CALL(variable, ret, function, ...) ZE_FAIL("XAudio2 has been disabled!"); ret## {}
#endif

// Type for proper graphics API implementations for all current APIs
#define ZE_AHI_BACKEND_TYPE(type) ZE::AHI::Backend<ZE_GET_OPENAL_AHI_TYPE(type) ZE_GET_XAUDIO2_AHI_TYPE(type)>

// Name of backend variable
#define ZE_AHI_BACKEND_VAR backend
// Wrapper adding backend implementation variable
#define ZE_AHI_BACKEND(type) ZE_AHI_BACKEND_TYPE(type) ZE_AHI_BACKEND_VAR

// Adds ability to access underlying audio backend implementation via Get() method
#define ZE_AHI_BACKEND_GET(type) \
	constexpr ZE_AHI_BACKEND_TYPE(type)& Get() noexcept { return ZE_AHI_BACKEND_VAR; } \
	constexpr const ZE_AHI_BACKEND_TYPE(type)& Get() const noexcept { return ZE_AHI_BACKEND_VAR; }

// Wrapper for creating backend objects
#define ZE_AHI_BACKEND_CREATE(type, ...) \
	ZE_AHI_CREATE_BACKEND_EX(ZE_AHI_BACKEND_TYPE(type), __VA_ARGS__); \
	type __rhi = {}; \
	__rhi.ZE_AHI_BACKEND_VAR = std::move(*__impl); \
	return __rhi

// Extended wrapper for calling methods on currently active API implementation
#define ZE_AHI_BACKEND_CALL_EX(variable, ret, function, ...) \
	switch (Settings::GetAudioApi()) \
	{ \
	case ZE::AHI::ApiType::None: \
	{ \
		##ret## {}; \
		break; \
	} \
	case ZE::AHI::ApiType::OpenAL: \
	{ \
		ZE_AHI_OPENAL_SWITCH_CALL(variable, ret, function, __VA_ARGS__); \
		break; \
	} \
	case ZE::AHI::ApiType::XAudio2: \
	{ \
		ZE_AHI_XAUDIO2_SWITCH_CALL(variable, ret, function, __VA_ARGS__); \
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
#define ZE_AHI_BACKEND_CALL(function, ...) ZE_AHI_BACKEND_CALL_EX(ZE_AHI_BACKEND_VAR, , function, __VA_ARGS__)
// Wrapper for calling methods on currently active API implementation and returning with it's value
#define ZE_AHI_BACKEND_CALL_RET(function, ...) ZE_AHI_BACKEND_CALL_EX(ZE_AHI_BACKEND_VAR, return, function, __VA_ARGS__)
// Wrapper for calling methods on currently active API implementation and getting return value
#define ZE_AHI_BACKEND_CALL_RET_VAR(returnVariable, function, ...) ZE_AHI_BACKEND_CALL_EX(ZE_RHI_BACKEND_VAR, returnVariable=, function, __VA_ARGS__)