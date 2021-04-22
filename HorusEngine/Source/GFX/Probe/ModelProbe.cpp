#include "GFX/Probe/ModelProbe.h"
#include "GFX/Shape/Model.h"

namespace GFX::Probe
{
	bool ModelProbe::PushNode(Shape::ModelNode& node) noexcept
	{
		const U64 selectedID = selectedNode == nullptr ? 0 : selectedNode->GetID();
		const bool expanded = ImGui::TreeNodeEx((void*)node.GetID(),
			ImGuiTreeNodeFlags_OpenOnArrow |
			(node.HasChildren() ? 0 : ImGuiTreeNodeFlags_Leaf) |
			(node.GetID() == selectedID ? ImGuiTreeNodeFlags_Selected : 0), node.GetName().c_str());
		if (ImGui::IsItemClicked())
			selectedNode = &node;
		return expanded;
	}

	void ModelProbe::Visit(Graphics& gfx, Shape::ModelNode& node) const noexcept
	{
		bool meshOnly = node.IsMesh();
		if (ImGui::Checkbox("Node mesh-only", &meshOnly))
			node.SetMesh(gfx, meshOnly);
	}

	bool ModelProbe::Visit(Graphics& gfx, Shape::Model& model, Shape::ModelNode& root) noexcept
	{
		if (selectedNode == nullptr)
			selectedNode = &root;
		bool change = model.IsOutline();
		ImGui::Columns(2, "#model_node_options", false);
		if (ImGui::Checkbox("Model outline", &change))
		{
			if (change)
				model.SetOutline();
			else
				model.DisableOutline();
		}
		ImGui::NextColumn();
		Visit(gfx, *selectedNode);
		ImGui::Columns(1);
		change = selectedNode->Object::Accept(gfx, static_cast<BaseProbe&>(*this));
		ImGui::Separator();
		ImGui::Columns(2);
		ImGui::BeginChild("##NodeTree", { 0.0f, 231.5f }, false, ImGuiWindowFlags_HorizontalScrollbar);
		change |= root.Accept(gfx, *this);
		ImGui::EndChild();
		ImGui::NextColumn();
		SetCompact();
		ImGui::NewLine();
		change |= selectedNode->Accept(gfx, static_cast<BaseProbe&>(*this));
		SetNormal();
		ImGui::Columns(1);
		return change;
	}
}