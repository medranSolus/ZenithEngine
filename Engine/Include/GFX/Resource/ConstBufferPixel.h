#pragma once
#include "ConstBuffer.h"

namespace ZE::GFX::Resource
{
	template<typename T>
	class ConstBufferPixel : public ConstBuffer<T>
	{
		using ConstBuffer<T>::ConstBuffer;
		using ConstBuffer<T>::GetContext;
		using ConstBuffer<T>::constantBuffer;
		using ConstBuffer<T>::name;
		using ConstBuffer<T>::slot;

	public:
		ConstBufferPixel(Graphics& gfx, const std::string& tag, const T& values, U32 slot = 0);
		ConstBufferPixel(Graphics& gfx, const std::string& tag, U32 slot = 0);
		virtual ~ConstBufferPixel() = default;

		static std::string GenerateRID(const std::string& tag, const T& values, U32 slot = 0) noexcept { return GenerateRID(tag, slot); }
		static std::string GenerateRID(const std::string& tag, U32 slot = 0) noexcept;

		static GfxResPtr<ConstBufferPixel> Get(Graphics& gfx, const std::string& tag, const T& values, U32 slot = 0);
		static GfxResPtr<ConstBufferPixel> Get(Graphics& gfx, const std::string& tag, U32 slot = 0);

		void Bind(Graphics& gfx) const override { GetContext(gfx)->PSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf()); }
		std::string GetRID() const noexcept override { return GenerateRID(name, slot); }
	};

	template<typename T>
	struct is_resolvable_by_codex<ConstBufferPixel<T>>
	{
		static constexpr bool GENERATE_ID{ true };
	};

#pragma region Functions
	template<typename T>
	ConstBufferPixel<T>::ConstBufferPixel(Graphics& gfx, const std::string& tag, const T& values, U32 slot)
		: ConstBuffer<T>(gfx, tag, values, slot)
	{
		ZE_GFX_ENABLE_RID(gfx);
		ZE_GFX_SET_RID(constantBuffer.Get());
	}

	template<typename T>
	ConstBufferPixel<T>::ConstBufferPixel(Graphics& gfx, const std::string& tag, U32 slot)
		: ConstBuffer<T>(gfx, tag, slot)
	{
		ZE_GFX_ENABLE_RID(gfx);
		ZE_GFX_SET_RID(constantBuffer.Get());
	}

	template<typename T>
	std::string ConstBufferPixel<T>::GenerateRID(const std::string& tag, U32 slot) noexcept
	{
		return "C" + std::to_string(slot) + "P" + std::to_string(sizeof(T)) + "#" + tag;
	}

	template<typename T>
	GfxResPtr<ConstBufferPixel<T>> ConstBufferPixel<T>::Get(Graphics& gfx, const std::string& tag, const T& values, U32 slot)
	{
		return Codex::Resolve<ConstBufferPixel<T>>(gfx, tag, values, slot);
	}

	template<typename T>
	GfxResPtr<ConstBufferPixel<T>> ConstBufferPixel<T>::Get(Graphics& gfx, const std::string& tag, U32 slot)
	{
		return Codex::Resolve<ConstBufferPixel<T>>(gfx, tag, slot);
	}
#pragma endregion
}