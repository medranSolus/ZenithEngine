#pragma once
#include "BaseProbe.h"
#include "imgui.h"

namespace ZE::GFX::Shape
{
	class ModelNode;
	class Model;
}

namespace ZE::GFX::Probe
{
	class ModelProbe : public BaseProbe
	{
		Shape::ModelNode* selectedNode = nullptr;

	public:
		ModelProbe() = default;
		ModelProbe(ModelProbe&&) = default;
		ModelProbe(const ModelProbe&) = default;
		ModelProbe& operator=(ModelProbe&&) = default;
		ModelProbe& operator=(const ModelProbe&) = default;
		virtual ~ModelProbe() = default;

		constexpr void Reset() noexcept override { selectedNode = nullptr; BaseProbe::Reset(); }
		void PopNode() const noexcept { ImGui::TreePop(); }

		bool PushNode(Shape::ModelNode& node) noexcept;
		void Visit(Graphics& gfx, Shape::ModelNode& node) const noexcept;
		bool Visit(Graphics& gfx, Shape::Model& model, Shape::ModelNode& root) noexcept;
	};
}