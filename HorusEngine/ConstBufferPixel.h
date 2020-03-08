#pragma once
#include "ConstBuffer.h"

namespace GFX::Resource
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
		ConstBufferPixel(const ConstBufferPixel&) = delete;
		ConstBufferPixel& operator=(const ConstBufferPixel&) = delete;
		~ConstBufferPixel() = default;

		static inline std::shared_ptr<ConstBufferPixel> Get(Graphics& gfx, const std::string& tag, const T& values, UINT slot = 0U);
		static inline std::shared_ptr<ConstBufferPixel> Get(Graphics& gfx, const std::string& tag, UINT slot = 0U);

		static inline std::string GenerateRID(const std::string& tag, const T& values, UINT slot = 0U) noexcept { return GenerateRID(tag, slot); }
		static inline std::string GenerateRID(const std::string& tag, UINT slot = 0U) noexcept;

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->PSSetConstantBuffers(slot, 1U, constantBuffer.GetAddressOf()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(name, slot); }
	};

	template<typename T>
	struct is_resolvable_by_codex<ConstBufferPixel<T>>
	{
		static constexpr bool value{ true };
	};

	template<typename T>
	inline std::shared_ptr<ConstBufferPixel<T>> ConstBufferPixel<T>::Get(Graphics& gfx, const std::string& tag, const T& values, UINT slot)
	{
		return Codex::Resolve<ConstBufferPixel<T>>(gfx, tag, values, slot);
	}

	template<typename T>
	inline std::shared_ptr<ConstBufferPixel<T>> ConstBufferPixel<T>::Get(Graphics& gfx, const std::string& tag, UINT slot)
	{
		return Codex::Resolve<ConstBufferPixel<T>>(gfx, tag, slot);
	}

	template<typename T>
	inline std::string ConstBufferPixel<T>::GenerateRID(const std::string& tag, UINT slot) noexcept
	{
		return "#" + std::string(typeid(ConstBufferPixel<T>).name()) + "#" + tag + "#" + std::to_string(slot) + "#";
	}
}