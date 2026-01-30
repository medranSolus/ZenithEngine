#pragma once
#include "Settings.h"

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

namespace ZE::AHI
{
	// Wrapper for proper graphics API implementations
	template<
#if _ZE_AHI_OPENAL
		typename OAL ZE_AHI_OPENAL_COMMA
#endif
#if _ZE_AHI_XAUDIO2
		typename XA2 ZE_AHI_XAUDIO2_COMMA
#endif
	>
	union Backend final
	{
#if _ZE_AHI_OPENAL
		OAL oal;
#endif
#if _ZE_AHI_XAUDIO2
		XA2 xa2;
#endif

		constexpr Backend() noexcept { Init(); }
		constexpr Backend(Backend&& b) noexcept
		{
			switch (Settings::GetAudioApi())
			{
			default:
				ZE_ENUM_UNHANDLED();
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
		constexpr Backend(const Backend& b) noexcept
		{
			switch (Settings::GetAudioApi())
			{
			default:
				ZE_ENUM_UNHANDLED();
#if _ZE_AHI_OPENAL
			case ApiType::OpenAL:
			{
				new(&oal) OAL(b.oal);
				break;
			}
#endif
#if _ZE_AHI_XAUDIO2
			case ApiType::XAudio2:
			{
				new(&xa2) XA2(b.xa2);
				break;
			}
#endif
			}
		}
		constexpr Backend& operator=(Backend&& b) noexcept
		{
			switch (Settings::GetAudioApi())
			{
			default:
				ZE_ENUM_UNHANDLED();
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
		constexpr Backend& operator=(const Backend& b) noexcept
		{
			switch (Settings::GetAudioApi())
			{
			default:
				ZE_ENUM_UNHANDLED();
#if _ZE_AHI_OPENAL
			case ApiType::OpenAL:
			{
				oal = b.oal;
				break;
			}
#endif
#if _ZE_AHI_XAUDIO2
			case ApiType::XAudio2:
			{
				xa2 = b.xa2;
				break;
			}
#endif
			}
			return *this;
		}
		~Backend() { Delete(); }

		template<typename ...Params>
		constexpr void Init(Params&& ...p) { Init(Settings::GetAudioApi(), std::forward<Params>(p)...); }
		template<typename ...Params>
		constexpr void Switch(ApiType nextApi, Params&& ...p) { Delete(); Init(nextApi, std::forward<Params>(p)...); }

		template<typename ...Params>
		constexpr void Init(ApiType type, Params&& ...p)
		{
			switch (type)
			{
			default:
				ZE_ENUM_UNHANDLED();
#if _ZE_AHI_OPENAL
			case ApiType::OpenAL:
			{
				new(&oal) OAL(std::forward<Params>(p)...);
				break;
			}
#endif
#if _ZE_AHI_XAUDIO2
			case ApiType::XAudio2:
			{
				new(&xa2) XA2(std::forward<Params>(p)...);
				break;
			}
#endif
			}
		}
		constexpr void Delete() noexcept
		{
			switch (Settings::GetAudioApi())
			{
			default:
				ZE_ENUM_UNHANDLED();
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
	};
}

// Helpers for manipulating with active AHI implementations
#if _ZE_AHI_OPENAL
#	define ZE_GET_OPENAL_AHI_TYPE(type) ZE::AHI::OpenAL::##type ZE_AHI_OPENAL_COMMA
#	define ZE_AHI_OPENAL_SWITCH_CALL(variable, ret, function, ...) ret## ##variable##.oal.##function##(__VA_ARGS__); break
#else
#	define ZE_GET_OPENAL_AHI_TYPE(type)
#	define ZE_AHI_OPENAL_SWITCH_CALL(variable, ret, function, ...) ZE_FAIL("OpenAL has been disabled!")
#endif
#if _ZE_AHI_XAUDIO2
#	define ZE_GET_XAUDIO2_AHI_TYPE(type) ZE::AHI::XAudio2::##type ZE_AHI_XAUDIO2_COMMA
#	define ZE_AHI_XAUDIO2_SWITCH_CALL(variable, ret, function, ...) ret## ##variable##.xa2.##function##(__VA_ARGS__); break
#else
#	define ZE_GET_XAUDIO2_RHI_TYPE(type)
#	define ZE_AHI_XAUDIO2_SWITCH_CALL(variable, ret, function, ...) ZE_FAIL("XAudio2 has been disabled!")
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

// Extended wrapper for calling methods on currently active API implementation
#define ZE_AHI_BACKEND_CALL_EX(variable, ret, function, ...) \
	switch (Settings::GetAudioApi()) \
	{ \
	case ZE::AHI::ApiType::OpenAL: \
	{ \
		ZE_AHI_OPENAL_SWITCH_CALL(variable, ret, function, __VA_ARGS__); \
	} \
	case ZE::AHI::ApiType::XAudio2: \
	{ \
		ZE_AHI_XAUDIO2_SWITCH_CALL(variable, ret, function, __VA_ARGS__); \
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
// Wrapper for calling methods on currently active API implementation and getting return value
#define ZE_AHI_BACKEND_CALL_RET(returnVariable, function, ...) ZE_AHI_BACKEND_CALL_EX(ZE_AHI_BACKEND_VAR, returnVariable=, function, __VA_ARGS__)