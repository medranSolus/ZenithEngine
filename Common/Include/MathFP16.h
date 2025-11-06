#pragma once
#include "MathExt.h"
#include <limits>
#include <cstring>

namespace std
{
	template<typename T, typename U>
	T bit_cast(U&& u)
	{
		static_assert(sizeof(T) == sizeof(U));
		union { T t; }; // prevent construction
		std::memcpy(&t, &u, sizeof(T));
		return t;
	}
}

namespace ZE::Math::FP16
{
	// Source: https://stackoverflow.com/a/3542975
	namespace Internal
	{
		// Info about underlying floating point bits type
		template<typename T> struct NativeFloatBits;
		template<> struct NativeFloatBits< float > { using Type = U32; };
		template<> struct NativeFloatBits< double > { using Type = U64; };
		template<typename T> using NativeFloatBitsType = typename NativeFloatBits<T>::Type;

		static_assert(sizeof(float) == sizeof(NativeFloatBitsType<float>));
		static_assert(sizeof(double) == sizeof(NativeFloatBitsType<double>));

		// Information about floating point type bits structure
		template<typename T, U8 SB, U8 EB>
		struct RawFloatTypeInfo
		{
			using RawType = T;

			static constexpr U8 SIG_BITS = SB;
			static constexpr U8 EXP_BITS = EB;
			static constexpr U8 BITS = SIG_BITS + EXP_BITS + 1;

			static_assert(std::is_integral_v<RawType>);
			static_assert(SIG_BITS >= 0);
			static_assert(EXP_BITS >= 0);
			static_assert(BITS <= sizeof(RawType) * 8);

			static constexpr U64 EXP_MAX = (1 << EXP_BITS) - 1;
			static constexpr U64 EXP_BIAS = EXP_MAX >> 1;

			static constexpr RawType SIGN = static_cast<RawType>(1) << (BITS - 1);
			static constexpr RawType INF = static_cast<RawType>(EXP_MAX) << SIG_BITS;
			static constexpr RawType QNAN = INF | (INF >> 1);

			static constexpr RawType Abs(RawType v) { return static_cast<RawType>(v & (SIGN - 1)); }
			static constexpr bool IsNan(RawType v) { return Abs(v) > INF; }
			static constexpr bool IsInf(RawType v) { return Abs(v) == INF; }
			static constexpr bool IsZero(RawType v) { return Abs(v) == 0; }
		};
		using RawFloat16Info = RawFloatTypeInfo<U16, 10, 5>;
		using RawFloat32Info = RawFloatTypeInfo< U32, 23, 8>;
		using RawFloat64Info = RawFloatTypeInfo< U64, 52, 11>;
		//using RawFloat128Info = RawFloatTypeInfo<U128, 112, 15>;

		// Floating point number information
		template<typename T,
			U8 SIG_BITS = std::numeric_limits<T>::digits - 1,
			U8 EXP_BITS = sizeof(T) * 8 - SIG_BITS - 1>
		struct FloatTypeInfo : RawFloatTypeInfo<NativeFloatBitsType<T>, SIG_BITS, EXP_BITS>
		{
			using FloatType = T;
			static_assert(std::is_floating_point_v<FloatType>);
		};

		// Main floating point encoding utilities
		template<typename R>
		struct RawFloatEncoder
		{
			using EncType = R;
			using EncRawType = typename EncType::RawType;

			template<bool ENABLE_ROUNDING, typename F>
			static EncRawType Encode(F value)
			{
				using FloatInfo = FloatTypeInfo<F>;
				using FloatRawType = typename FloatInfo::RawType;

				static constexpr U8 SIG_DIFF = FloatInfo::SIG_BITS - EncType::SIG_BITS;
				static constexpr U8 BIT_DIFF = FloatInfo::BITS - EncType::BITS;
				static constexpr bool DO_ROUNDING = ENABLE_ROUNDING && SIG_DIFF > 0;
				static constexpr FloatRawType BIAS_MUL = static_cast<FloatRawType>(EncType::EXP_BIAS) << FloatInfo::SIG_BITS;

				if constexpr (!DO_ROUNDING)
				{ // fix exp bias
					// when not rounding, fix exp first to avoid mixing float and binary ops
					value *= std::bit_cast<F>(BIAS_MUL);
				}
				FloatRawType bits = std::bit_cast<FloatRawType>(value);
				FloatRawType sign = bits & FloatInfo::SIGN; // save sign
				bits ^= sign; // clear sign
				bool notNan = FloatInfo::INF >= bits; // compare before rounding!!

				if constexpr (DO_ROUNDING)
				{
					static constexpr FloatRawType MIN_NORM = static_cast<FloatRawType>(FloatInfo::EXP_BIAS - EncType::EXP_BIAS + 1) << FloatInfo::SIG_BITS;
					static constexpr FloatRawType SUB_RND = EncType::EXP_BIAS < SIG_DIFF
						? static_cast<FloatRawType>(1) << (FloatInfo::SIG_BITS - 1 + EncType::EXP_BIAS - SIG_DIFF)
						: static_cast<FloatRawType>(EncType::EXP_BIAS - SIG_DIFF) << FloatInfo::SIG_BITS;
					static constexpr FloatRawType SUB_MUL = static_cast<FloatRawType>(FloatInfo::EXP_BIAS + SIG_DIFF) << FloatInfo::SIG_BITS;

					bool notSub = bits >= MIN_NORM;
					F norm = std::bit_cast<F>(bits);
					F subn = norm;
					subn *= std::bit_cast<F>(SUB_RND); // round subnormals
					subn *= std::bit_cast<F>(SUB_MUL); // correct subnormal exp
					norm *= std::bit_cast<F>(BIAS_MUL); // fix exp bias
					bits = std::bit_cast<FloatRawType>(norm);
					bits += (bits >> SIG_DIFF) & 1; // add tie breaking bias
					bits += (static_cast<FloatRawType>(1) << (SIG_DIFF - 1)) - 1; // round up to half
					bits ^= -!notSub & (std::bit_cast<FloatRawType>(subn) ^ bits);
				}
				bits >>= SIG_DIFF; // truncate
				bits ^= -(EncType::INF < bits) & (EncType::INF ^ bits); // fix overflow
				bits ^= -!notNan & (EncType::QNAN ^ bits);
				bits |= sign >> BIT_DIFF; // restore sign
				return static_cast<EncRawType>(bits);
			}

			template<typename F>
			static F Decode(EncRawType value)
			{
				using FloatInfo = FloatTypeInfo<F>;
				using FloatRawType = typename FloatInfo::RawType;

				static constexpr U8 SIG_DIFF = FloatInfo::SIG_BITS - EncType::SIG_BITS;
				static constexpr U8 BIT_DIFF = FloatInfo::BITS - EncType::BITS;
				static constexpr FloatRawType BIAS_MUL = static_cast<FloatRawType>(2 * FloatInfo::EXP_BIAS - EncType::EXP_BIAS) << FloatInfo::SIG_BITS;

				FloatRawType bits = value;
				FloatRawType sign = bits & EncType::SIGN; // save sign
				bits ^= sign; // clear sign
				bool isNorm = bits < EncType::INF;
				bits = (sign << BIT_DIFF) | (bits << SIG_DIFF);
				F val = std::bit_cast<F>(bits) * std::bit_cast<F>(BIAS_MUL);
				bits = std::bit_cast<FloatRawType>(val);
				bits |= -!isNorm & FloatInfo::INF;
				return std::bit_cast<F>(bits);
			}
		};

		using Float16Encoder = RawFloatEncoder<RawFloat16Info>;
	}

	template<typename F>
	auto EncodeFloat16Fast(F&& value)
	{
		return Internal::Float16Encoder::Encode<false>(std::forward<F>(value));
	}

	template<typename F>
	auto EncodeFloat16(F&& value)
	{
		return Internal::Float16Encoder::Encode<true>(std::forward<F>(value));
	}

	template<typename F = float, typename X>
	auto DecodeFloat16(X&& value)
	{
		return Internal::Float16Encoder::Decode<F>(std::forward<X>(value));
	}
}