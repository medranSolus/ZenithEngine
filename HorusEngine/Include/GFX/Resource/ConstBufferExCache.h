#pragma once
#include "ConstBufferExGeometry.h"
#include "ConstBufferExPixel.h"
#include "ConstBufferExVertex.h"
#include "GfxDebugName.h"
#include "GFX/Graphics.h"

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

		mutable bool dirty = false;
		Data::CBuffer::DynamicCBuffer buffer;

		static std::string GenerateRID(const std::string& tag, const Data::CBuffer::DCBLayoutElement& root, U32 slot) noexcept;

	public:
		ConstBufferExCache(Graphics& gfx, const std::string& tag,
			const Data::CBuffer::DCBLayoutFinal& layout, U32 slot = 0);

		ConstBufferExCache(Graphics& gfx, const std::string& tag,
			const Data::CBuffer::DynamicCBuffer& buffer, U32 slot = 0);

		virtual ~ConstBufferExCache() = default;

		static GfxResPtr<ConstBufferExCache> Get(Graphics& gfx, const std::string& tag,
			const Data::CBuffer::DCBLayoutFinal& layout, U32 slot = 0);
		static GfxResPtr<ConstBufferExCache> Get(Graphics& gfx, const std::string& tag,
			const Data::CBuffer::DynamicCBuffer& buffer, U32 slot = 0);

		static std::string GenerateRID(const std::string& tag,
			const Data::CBuffer::DCBLayoutFinal& layout, U32 slot = 0) noexcept;
		static std::string GenerateRID(const std::string& tag,
			const Data::CBuffer::DynamicCBuffer& buffer, U32 slot = 0) noexcept;

		constexpr Data::CBuffer::DynamicCBuffer& GetBuffer() noexcept { dirty = true; return buffer; }
		constexpr const Data::CBuffer::DynamicCBuffer& GetBufferConst() const noexcept { return buffer; }

		std::string GetRID() const noexcept override { return GenerateRID(name, buffer, slot); }
		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return dirty |= probe.Visit(buffer); }

		void Bind(Graphics& gfx) const override;
	};

	template<typename T>
	struct is_resolvable_by_codex<ConstBufferExCache<T>>
	{
		static constexpr bool GENERATE_ID{ true };
	};

#pragma region Functions
	template<typename T>
	std::string ConstBufferExCache<T>::GenerateRID(const std::string& tag,
		const Data::CBuffer::DCBLayoutElement& root, U32 slot) noexcept
	{
		return "C" + T::GenerateRID(tag, root, slot);
	}

	template<typename T>
	ConstBufferExCache<T>::ConstBufferExCache(Graphics& gfx, const std::string& tag,
		const Data::CBuffer::DCBLayoutFinal& layout, U32 slot)
		: T(gfx, tag, *layout.GetRoot(), slot, nullptr, false), buffer(layout)
	{
#ifdef _MODE_DEBUG
		GFX_ENABLE_ALL(gfx);
		GFX_SET_RID(constantBuffer.Get());
#endif
	}

	template<typename T>
	ConstBufferExCache<T>::ConstBufferExCache(Graphics& gfx, const std::string& tag,
		const Data::CBuffer::DynamicCBuffer& buffer, U32 slot)
		: T(gfx, tag, buffer.GetRootElement(), slot, &buffer, false), buffer(buffer)
	{
#ifdef _MODE_DEBUG
		GFX_ENABLE_ALL(gfx);
		GFX_SET_RID(constantBuffer.Get());
#endif
	}

	template<typename T>
	std::string ConstBufferExCache<T>::GenerateRID(const std::string& tag,
		const Data::CBuffer::DCBLayoutFinal& layout, U32 slot) noexcept
	{
		return GenerateRID(tag, *layout.GetRoot(), slot);
	}

	template<typename T>
	std::string ConstBufferExCache<T>::GenerateRID(const std::string& tag,
		const Data::CBuffer::DynamicCBuffer& buffer, U32 slot) noexcept
	{
		return GenerateRID(tag, buffer.GetRootElement(), slot);
	}

	template<typename T>
	GfxResPtr<ConstBufferExCache<T>> ConstBufferExCache<T>::Get(Graphics& gfx,
		const std::string& tag, const Data::CBuffer::DCBLayoutFinal& layout, U32 slot)
	{
		return Codex::Resolve<ConstBufferExCache<T>>(gfx, tag, layout, slot);
	}

	template<typename T>
	GfxResPtr<ConstBufferExCache<T>> ConstBufferExCache<T>::Get(Graphics& gfx,
		const std::string& tag, const Data::CBuffer::DynamicCBuffer& buffer, U32 slot)
	{
		return Codex::Resolve<ConstBufferExCache<T>>(gfx, tag, buffer, slot);
	}

	template<typename T>
	void ConstBufferExCache<T>::Bind(Graphics& gfx) const
	{
		if (dirty)
		{
			ConstBufferEx::Update(gfx, buffer);
			dirty = false;
		}
		T::Bind(gfx);
	}
#pragma endregion

	typedef ConstBufferExCache<ConstBufferExGeometry> ConstBufferExGeometryCache;
	typedef ConstBufferExCache<ConstBufferExPixel> ConstBufferExPixelCache;
	typedef ConstBufferExCache<ConstBufferExVertex> ConstBufferExVertexCache;
}