#pragma once
#include "GFX/Data/CBuffer/DynamicCBuffer.h"
#include "Camera/ProjectionData.h"

namespace GFX
{
	class Graphics;

	namespace Pipeline
	{
		class Technique;
	}
	namespace Shape
	{
		class BaseShape;
	}
}

namespace GFX::Probe
{
	class BaseProbe
	{
		Pipeline::Technique* technique = nullptr;
		bool compact = false;

		std::string MakeTag(std::string&& label) const noexcept;

	public:
		BaseProbe() = default;
		BaseProbe(BaseProbe&&) = default;
		BaseProbe(const BaseProbe&) = default;
		BaseProbe& operator=(BaseProbe&&) = default;
		BaseProbe& operator=(const BaseProbe&) = default;
		virtual ~BaseProbe() = default;

		constexpr void ReleaseTechnique() noexcept { technique = nullptr; }
		constexpr void SetCompact() noexcept { compact = true; }
		constexpr void SetNormal() noexcept { compact = false; }

		void SetTechnique(Pipeline::Technique* currentTechnique) noexcept;

		bool Visit(Data::CBuffer::DynamicCBuffer& buffer) const noexcept;
		bool VisitObject(Data::CBuffer::DynamicCBuffer& buffer) const noexcept;
		bool VisitMaterial(Data::CBuffer::DynamicCBuffer& buffer) const noexcept;
		bool VisitLight(Data::CBuffer::DynamicCBuffer& buffer) const noexcept;
		void VisitShape(Graphics& gfx, Shape::BaseShape& shape) const noexcept;
		bool VisitCamera(Camera::ProjectionData& projection) const noexcept;
	};
}