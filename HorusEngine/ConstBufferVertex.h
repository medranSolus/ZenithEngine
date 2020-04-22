#pragma once
#include "ConstBuffer.h"

namespace GFX::Resource
{
	template<typename T>
	class ConstBufferVertex : public ConstBuffer<T>
	{
		using ConstBuffer<T>::ConstBuffer;
		using ConstBuffer<T>::GetContext;
		using ConstBuffer<T>::constantBuffer;
		using ConstBuffer<T>::name;
		using ConstBuffer<T>::slot;

	public:
		ConstBufferVertex(const ConstBufferVertex&) = delete;
		ConstBufferVertex& operator=(const ConstBufferVertex&) = delete;
		virtual ~ConstBufferVertex() = default;

		static inline std::shared_ptr<ConstBufferVertex> Get(Graphics& gfx, const std::string& tag, const T& values, UINT slot = 0U);
		static inline std::shared_ptr<ConstBufferVertex> Get(Graphics& gfx, const std::string& tag, UINT slot = 0U);

		static inline std::string GenerateRID(const std::string& tag, const T& values, UINT slot = 0U) noexcept { return GenerateRID(tag, slot); }
		static inline std::string GenerateRID(const std::string& tag, UINT slot = 0U) noexcept;

		inline void Bind(Graphics& gfx) noexcept override { GetContext(gfx)->VSSetConstantBuffers(slot, 1U, constantBuffer.GetAddressOf()); }
		inline std::string GetRID() const noexcept override { return GenerateRID(name, slot); }
	};

	template<typename T>
	struct is_resolvable_by_codex<ConstBufferVertex<T>>
	{
		static constexpr bool generate{ true };
	};

	template<typename T>
	inline std::shared_ptr<ConstBufferVertex<T>> ConstBufferVertex<T>::Get(Graphics& gfx, const std::string& tag, const T& values, UINT slot)
	{
		return Codex::Resolve<ConstBufferVertex<T>>(gfx, tag, values, slot);
	}

	template<typename T>
	inline std::shared_ptr<ConstBufferVertex<T>> ConstBufferVertex<T>::Get(Graphics& gfx, const std::string& tag, UINT slot)
	{
		return Codex::Resolve<ConstBufferVertex<T>>(gfx, tag, slot);
	}

	template<typename T>
	inline std::string ConstBufferVertex<T>::GenerateRID(const std::string& tag, UINT slot) noexcept
	{
		return "#" + std::string(typeid(ConstBufferVertex<T>).name()) + "#" + tag + "#" + std::to_string(slot) + "#";
	}
}