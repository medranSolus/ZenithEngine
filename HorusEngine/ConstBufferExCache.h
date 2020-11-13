#pragma once
#include "ConstBufferExGeometry.h"
#include "ConstBufferExPixel.h"
#include "ConstBufferExVertex.h"

namespace GFX::Resource
{
	template<typename T>
	class ConstBufferExCache : public T
	{
		using ConstBufferEx::GetContext;
		using ConstBufferEx::constantBuffer;
		using ConstBufferEx::rootLayout;
		using ConstBufferEx::name;
		using ConstBufferEx::slot;

		bool dirty = false;
		Data::CBuffer::DynamicCBuffer buffer;

		static inline std::string GenerateRID(const std::string& tag,
			const Data::CBuffer::DCBLayoutElement& root, UINT slot = 0U) noexcept;

	public:
		ConstBufferExCache(Graphics& gfx, const std::string& tag,
			const Data::CBuffer::DCBLayoutFinal& layout, UINT slot = 0U);

		ConstBufferExCache(Graphics& gfx, const std::string& tag,
			const Data::CBuffer::DynamicCBuffer& buffer, UINT slot = 0U);

		virtual ~ConstBufferExCache() = default;

		static inline GfxResPtr<ConstBufferExCache> Get(Graphics& gfx, const std::string& tag,
			const Data::CBuffer::DCBLayoutFinal& layout, UINT slot = 0U);
		static inline GfxResPtr<ConstBufferExCache> Get(Graphics& gfx, const std::string& tag,
			const Data::CBuffer::DynamicCBuffer& buffer, UINT slot = 0U);

		static inline std::string GenerateRID(const std::string& tag,
			const Data::CBuffer::DCBLayoutFinal& layout, UINT slot = 0U) noexcept;
		static inline std::string GenerateRID(const std::string& tag,
			const Data::CBuffer::DynamicCBuffer& buffer, UINT slot = 0U) noexcept;

		constexpr Data::CBuffer::DynamicCBuffer& GetBuffer() noexcept { dirty = true; return buffer; }
		constexpr const Data::CBuffer::DynamicCBuffer& GetBufferConst() const noexcept { return buffer; }

		void Bind(Graphics& gfx) override;
		inline std::string GetRID() const noexcept override { return GenerateRID(name, buffer, slot); }
		inline bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return dirty |= probe.Visit(buffer); }
	};

	template<typename T>
	struct is_resolvable_by_codex<ConstBufferExCache<T>>
	{
		static constexpr bool generate{ true };
	};

	template<typename T>
	inline std::string ConstBufferExCache<T>::GenerateRID(const std::string& tag,
		const Data::CBuffer::DCBLayoutElement& root, UINT slot) noexcept
	{
		return "C" + T::GenerateRID(tag, root, slot);
	}

	template<typename T>
	inline ConstBufferExCache<T>::ConstBufferExCache(Graphics& gfx, const std::string& tag,
		const Data::CBuffer::DCBLayoutFinal& layout, UINT slot)
		: T(gfx, tag, *layout.GetRoot(), slot, nullptr, false), buffer(layout)
	{
#ifdef _DEBUG
		GFX_ENABLE_ALL(gfx);
		SET_DEBUG_NAME_RID(constantBuffer.Get());
#endif
	}

	template<typename T>
	inline ConstBufferExCache<T>::ConstBufferExCache(Graphics& gfx, const std::string& tag,
		const Data::CBuffer::DynamicCBuffer& buffer, UINT slot)
		: T(gfx, tag, buffer.GetRootElement(), slot, &buffer, false), buffer(buffer)
	{
#ifdef _DEBUG
		GFX_ENABLE_ALL(gfx);
		SET_DEBUG_NAME_RID(constantBuffer.Get());
#endif
	}

	template<typename T>
	inline GfxResPtr<ConstBufferExCache<T>> ConstBufferExCache<T>::Get(Graphics& gfx, const std::string& tag,
		const Data::CBuffer::DCBLayoutFinal& layout, UINT slot)
	{
		return Codex::Resolve<ConstBufferExCache<T>>(gfx, tag, layout, slot);
	}

	template<typename T>
	inline GfxResPtr<ConstBufferExCache<T>> ConstBufferExCache<T>::Get(Graphics& gfx, const std::string& tag,
		const Data::CBuffer::DynamicCBuffer& buffer, UINT slot)
	{
		return Codex::Resolve<ConstBufferExCache<T>>(gfx, tag, buffer, slot);
	}

	template<typename T>
	inline std::string ConstBufferExCache<T>::GenerateRID(const std::string& tag,
		const Data::CBuffer::DCBLayoutFinal& layout, UINT slot) noexcept
	{
		return GenerateRID(tag, *layout.GetRoot(), slot);
	}

	template<typename T>
	inline std::string ConstBufferExCache<T>::GenerateRID(const std::string& tag,
		const Data::CBuffer::DynamicCBuffer& buffer, UINT slot) noexcept
	{
		return GenerateRID(tag, buffer.GetRootElement(), slot);
	}

	template<typename T>
	inline void ConstBufferExCache<T>::Bind(Graphics& gfx)
	{
		if (dirty)
		{
			ConstBufferEx::Update(gfx, buffer);
			dirty = false;
		}
		T::Bind(gfx);
	}

	typedef ConstBufferExCache<ConstBufferExGeometry> ConstBufferExGeometryCache;
	typedef ConstBufferExCache<ConstBufferExPixel> ConstBufferExPixelCache;
	typedef ConstBufferExCache<ConstBufferExVertex> ConstBufferExVertexCache;
}