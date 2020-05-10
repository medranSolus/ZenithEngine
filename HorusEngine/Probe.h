#pragma once
#include "DynamicCBuffer.h"

namespace GFX::Pipeline
{
	class Technique;
}
namespace GFX::Shape
{
	class BaseShape;
}

namespace GFX
{
	class BasicObject;

	class Probe
	{
		Pipeline::Technique* technique = nullptr;

	public:
		void SetTechnique(Pipeline::Technique* currentTechnique) noexcept;
		void Release() noexcept;

		bool VisitBuffer(Data::CBuffer::DynamicCBuffer& buffer) noexcept(!IS_DEBUG);
		bool VisitObject(BasicObject& object) noexcept;
		bool VisitShape(Shape::BaseShape& shape) noexcept;
	};
}