#pragma once
#include "CBuffer.h"

namespace ZE::RHI::DX11::Resource
{
	template<typename T>
	class Constant final
	{
		CBufferInternal<true> buffer;

	public:
		Constant() = default;
		Constant(GFX::Device& dev, const T& value) : buffer(dev.Get().dx11, &value, sizeof(T), true) {}
		ZE_CLASS_MOVE(Constant);
		~Constant() = default;

		static Expected<Constant> Create(GFX::Device& dev, const T& value) noexcept;

		constexpr Status Set(GFX::Device& dev, const T& value) const { return buffer.Update(dev.Get().dx11, { INVALID_EID, &value, nullptr, sizeof(T) }); }

		void Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx) const noexcept { buffer.Bind(cl, bindCtx); }
	};

#pragma region Functions
	template<typename T>
	Expected<Constant<T>> Constant<T>::Create(GFX::Device& dev, const T& value) noexcept
	{
		Constant constant = {};
		ZE_EXPECT_RET_FAILED(constant.buffer, CBufferInternal<true>::Create(dev.Get().dx11, &value, sizeof(T)));
		return constant;
	}
#pragma endregion
}