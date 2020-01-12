#pragma once
#include "ConstantVertexBuffer.h"
#include "ShaderConstantBuffers.h"
#include "GfxObject.h"

namespace GFX::Resource
{
	class ConstantTransformBuffer : public IBindable
	{
		static std::unique_ptr<ConstantVertexBuffer<TransformConstatBuffer>> vertexBuffer;
		const GfxObject & parent;

	public:
		ConstantTransformBuffer(Graphics & gfx, const GfxObject & parent, UINT slot = 0U);
		ConstantTransformBuffer(const ConstantTransformBuffer&) = delete;
		ConstantTransformBuffer & operator=(const ConstantTransformBuffer&) = delete;
		~ConstantTransformBuffer() = default;

		static inline std::shared_ptr<ConstantTransformBuffer> Get(Graphics& gfx, const GfxObject & parent, UINT slot = 0U);
		static inline std::string GenerateRID(const GfxObject & parent, UINT slot = 0U) noexcept;

		void Bind(Graphics & gfx) noexcept override;
		inline std::string GetRID() const noexcept override { return GenerateRID(parent, vertexBuffer->GetSlot()); }
	};

	template<>
	struct is_resolvable_by_codex<ConstantTransformBuffer>
	{
		static constexpr bool value{ true };
	};

	inline std::shared_ptr<ConstantTransformBuffer> ConstantTransformBuffer::Get(Graphics& gfx, const GfxObject & parent, UINT slot)
	{
		return Codex::Resolve<ConstantTransformBuffer>(gfx, parent, slot);
	}

	inline std::string ConstantTransformBuffer::GenerateRID(const GfxObject & parent, UINT slot) noexcept
	{
		return "#" + std::string(typeid(ConstantTransformBuffer).name()) + "#" + std::to_string(slot) + "#";
	}
}
