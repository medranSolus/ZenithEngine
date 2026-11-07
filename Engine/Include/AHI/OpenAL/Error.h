#pragma once
#include "OAL.h"

namespace ZE::AHI::OpenAL
{
	// Main handler for OpenAL al-related errors
	class AlError : public std::error_category
	{
	protected:
		AlError() = default;

	public:
		ZE_CLASS_MOVE(AlError);
		virtual ~AlError() = default;

		static constexpr const std::error_category& GetCategory() noexcept { static constexpr AlError CATEGORY; return CATEGORY; }
		static Status Make(ALenum error) noexcept { return { static_cast<int>(error), GetCategory() }; }

		const char* name() const noexcept override { return "OpenAL AL Error"; }

		std::string message(int condition) const override;
	};

	// Main handler for OpenAL alc-related errors
	class AlcError : public std::error_category
	{
	protected:
		AlcError() = default;

	public:
		ZE_CLASS_MOVE(AlcError);
		virtual ~AlcError() = default;

		static constexpr const std::error_category& GetCategory() noexcept { static constexpr AlcError CATEGORY; return CATEGORY; }
		static Status Make(ALCenum error) noexcept { return { static_cast<int>(error), GetCategory()}; }

		const char* name() const noexcept override { return "OpenAL ALC Error"; }

		std::string message(int condition) const override;
	};
}

#define ZE_AL_ERROR(error) ZE::AHI::OpenAL::AlError::Make(error)
// Returns true if the al-error occured
#define ZE_AL_FAILED(error) (error) != AL_NO_ERROR
// Perform assert check the previous AL call
#define ZE_AL_CHECK(msg) do { ALenum __alerror = alGetError(); ZE_ASSERT(!ZE_AL_FAILED(__alerror), msg); } while (false)
// Check result of the previous AL call and return said error if failed
#define ZE_AL_RET_FAILED() do { ALenum __alerror = alGetError(); if (ZE_AL_FAILED(__alerror)) { ZE_BREAK(); return ZE_AL_ERROR(__alerror); } } while (false)
// Check result of the previous AL call and return said error if failed  in coroutine
#define ZE_AL_RET_FAILED_CORO() do { ALenum __alerror = alGetError(); if (ZE_AL_FAILED(__alerror)) { ZE_BREAK(); co_return ZE_AL_ERROR(__alerror); } } while (false)
// Check result of the previous AL call and return said error if failed (wrapped in std::unexpected)
#define ZE_AL_RET_FAILED_EXPECT() do { ALenum __alerror = alGetError(); if (ZE_AL_FAILED(__alerror)) { ZE_BREAK(); return std::unexpected(ZE_AL_ERROR(__alerror)); } } while (false)
// Check result of the previous AL call and return said error if failed in coroutine (wrapped in std::unexpected)
#define ZE_AL_RET_FAILED_EXPECT_CORO() do { ALenum __alerror = alGetError(); if (ZE_AL_FAILED(__alerror)) { ZE_BREAK(); co_return std::unexpected(ZE_AL_ERROR(__alerror)); } } while (false)

#define ZE_ALC_ERROR(error) ZE::AHI::OpenAL::AlcError::Make(error)
// Returns true if the alc-error occured
#define ZE_ALC_FAILED(error) (error) != ALC_NO_ERROR
// Perform assert check the previous ALC call
#define ZE_ALC_CHECK(device, msg) do { ALCenum __alcerror = alcGetError(device); ZE_ASSERT(!ZE_ALC_FAILED(__alcerror), msg); } while (false)
// Check result of the previous ALC call and return said error if failed
#define ZE_ALC_RET_FAILED(device) do { ALCenum __alcerror = alcGetError(device); if (ZE_ALC_FAILED(__alcerror)) { ZE_BREAK(); return ZE_ALC_ERROR(__alcerror); } } while (false)
// Check result of the previous ALC call and return said error if failed in coroutine
#define ZE_ALC_RET_FAILED_CORO(device) do { ALCenum __alcerror = alcGetError(device); if (ZE_ALC_FAILED(__alcerror)) { ZE_BREAK(); co_return ZE_ALC_ERROR(__alcerror); } } while (false)
// Check result of the previous ALC call and return said error if failed (wrapped in std::unexpected)
#define ZE_ALC_RET_FAILED_EXPECT(device) do { ALCenum __alcerror = alcGetError(device); if (ZE_ALC_FAILED(__alcerror)) { ZE_BREAK(); return std::unexpected(ZE_ALC_ERROR(__alcerror)); } } while (false)
// Check result of the previous ALC call and return said error if failed in coroutine (wrapped in std::unexpected)
#define ZE_ALC_RET_FAILED_EXPECT_CORO(device) do { ALCenum __alcerror = alcGetError(device); if (ZE_ALC_FAILED(__alcerror)) { ZE_BREAK(); co_return std::unexpected(ZE_ALC_ERROR(__alcerror)); } } while (false)