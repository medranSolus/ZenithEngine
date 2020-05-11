#pragma once
#include "Graphics.h"
#include "DynamicCBuffer.h"

namespace GFX::Pipeline
{
	class Technique;
}
namespace GFX::Shape
{
	class BaseShape;
}

namespace GFX::Probe
{
	class BaseProbe
	{
		Pipeline::Technique* technique = nullptr;

	public:
		BaseProbe() = default;
		BaseProbe(const BaseProbe&) = default;
		BaseProbe& operator=(const BaseProbe&) = default;
		virtual ~BaseProbe() = default;

		void SetTechnique(Pipeline::Technique* currentTechnique) noexcept;
		void ReleaseTechnique() noexcept;

		bool Visit(Data::CBuffer::DynamicCBuffer& buffer) noexcept(!IS_DEBUG);
		bool VisitObject(Data::CBuffer::DynamicCBuffer& buffer) noexcept(!IS_DEBUG);
		bool VisitMaterial(Data::CBuffer::DynamicCBuffer& buffer) noexcept(!IS_DEBUG);
		bool VisitLight(Data::CBuffer::DynamicCBuffer& buffer) noexcept(!IS_DEBUG);
		void VisitShape(Graphics& gfx, Shape::BaseShape& shape) noexcept;
	};
}