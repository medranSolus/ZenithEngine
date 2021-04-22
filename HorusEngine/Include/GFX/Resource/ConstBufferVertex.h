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
		ConstBufferVertex(Graphics& gfx, const std::string& tag, const T& values, U32 slot = 0);
		ConstBufferVertex(Graphics& gfx, const std::string& tag, U32 slot = 0);
		virtual ~ConstBufferVertex() = default;

		static std::string GenerateRID(const std::string & tag, const T & values, U32 slot = 0) noexcept { return GenerateRID(tag, slot); }
		static std::string GenerateRID(const std::string & tag, U32 slot = 0) noexcept;

		static GfxResPtr<ConstBufferVertex> Get(Graphics& gfx, const std::string& tag, const T& values, U32 slot = 0);
		static GfxResPtr<ConstBufferVertex> Get(Graphics& gfx, const std::string& tag, U32 slot = 0);

		void Bind(Graphics& gfx) const override { GetContext(gfx)->VSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf()); }
		std::string GetRID() const noexcept override { return GenerateRID(name, slot); }
	};

	template<typename T>
	struct is_resolvable_by_codex<ConstBufferVertex<T>>
	{
		static constexpr bool GENERATE_ID{ true };
	};

#pragma region Functions
	template<typename T>
	ConstBufferVertex<T>::ConstBufferVertex(Graphics& gfx, const std::string& tag, const T& values, U32 slot)
		: ConstBuffer(gfx, tag, values, slot)
	{
		GFX_ENABLE_RID(gfx);
		GFX_SET_RID(constantBuffer.Get());
	}

	template<typename T>
	ConstBufferVertex<T>::ConstBufferVertex(Graphics& gfx, const std::string& tag, U32 slot)
		: ConstBuffer<T>(gfx, tag, slot)
	{
		GFX_ENABLE_RID(gfx);
		GFX_SET_RID(constantBuffer.Get());
	}

	template<typename T>
	std::string ConstBufferVertex<T>::GenerateRID(const std::string& tag, U32 slot) noexcept
	{
		return "C" + std::to_string(slot) + "V" + std::to_string(sizeof(T)) + "#" + tag;
	}

	template<typename T>
	GfxResPtr<ConstBufferVertex<T>> ConstBufferVertex<T>::Get(Graphics& gfx, const std::string& tag, const T& values, U32 slot)
	{
		return Codex::Resolve<ConstBufferVertex<T>>(gfx, tag, values, slot);
	}

	template<typename T>
	GfxResPtr<ConstBufferVertex<T>> ConstBufferVertex<T>::Get(Graphics& gfx, const std::string& tag, U32 slot)
	{
		return Codex::Resolve<ConstBufferVertex<T>>(gfx, tag, slot);
	}
#pragma endregion
}