#pragma once
#include "ConstantBuffer.h"

namespace GFX::Resource
{
	template<typename T>
	class ConstantPixelBuffer : public ConstantBuffer<T>
	{
		using ConstantBuffer<T>::ConstantBuffer;
		using ConstantBuffer<T>::GetContext;
		using ConstantBuffer<T>::constantBuffer;
		using ConstantBuffer<T>::slot;
		using ConstantBuffer<T>::name;

	public:
		ConstantPixelBuffer(const ConstantPixelBuffer&) = delete;
		ConstantPixelBuffer& operator=(const ConstantPixelBuffer&) = delete;
		~ConstantPixelBuffer() = default;

		static inline std::shared_ptr<ConstantPixelBuffer> Get(Graphics& gfx, const std::string& tag, const T& values, UINT slot = 0U);
		static inline std::shared_ptr<ConstantPixelBuffer> Get(Graphics& gfx, const std::string& tag, UINT slot = 0U);

		static inline std::string GenerateRID(const std::string& tag, const T& values, UINT slot = 0U) noexcept { return GenerateRID(tag, slot); }
		static inline std::string GenerateRID(const std::string& tag, UINT slot = 0U) noexcept;

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->PSSetConstantBuffers(slot, 1U, constantBuffer.GetAddressOf()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(name, slot); }
	};

	template<typename T>
	struct is_resolvable_by_codex<ConstantPixelBuffer<T>>
	{
		static constexpr bool value{ true };
	};

	template<typename T>
	inline std::shared_ptr<ConstantPixelBuffer<T>> ConstantPixelBuffer<T>::Get(Graphics& gfx, const std::string& tag, const T& values, UINT slot)
	{
		return Codex::Resolve<ConstantPixelBuffer<T>>(gfx, tag, values, slot);
	}

	template<typename T>
	inline std::shared_ptr<ConstantPixelBuffer<T>> ConstantPixelBuffer<T>::Get(Graphics& gfx, const std::string& tag, UINT slot)
	{
		return Codex::Resolve<ConstantPixelBuffer<T>>(gfx, tag, slot);
	}

	template<typename T>
	inline std::string ConstantPixelBuffer<T>::GenerateRID(const std::string& tag, UINT slot) noexcept
	{
		return "#" + std::string(typeid(ConstantPixelBuffer<T>).name()) + "#" + tag + "#" + std::to_string(slot) + "#";
	}
}
