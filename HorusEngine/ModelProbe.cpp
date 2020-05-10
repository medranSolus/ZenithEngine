#include "ModelProbe.h"
#include "Model.h"

namespace GFX::Probe
{
	bool ModelProbe::PushNode(Shape::ModelNode& node) noexcept
	{
		const unsigned long long selectedID = selectedNode == nullptr ? 0 : selectedNode->GetID();
		const bool expanded = ImGui::TreeNodeEx((void*)node.GetID(),
			ImGuiTreeNodeFlags_OpenOnArrow |
			(node.HasChildren() ? 0 : ImGuiTreeNodeFlags_Leaf) |
			(node.GetID() == selectedID ? ImGuiTreeNodeFlags_Selected : 0), node.GetName().c_str());
		if (ImGui::IsItemClicked())
			selectedNode = &node;
		return expanded;
	}

	void ModelProbe::Visit(Graphics& gfx, Shape::ModelNode& node) noexcept
	{
		bool meshOnly = node.IsMesh();
		ImGui::Checkbox("Mesh-only", &meshOnly);
		if (node.IsMesh() != meshOnly)
			node.SetMesh(gfx, meshOnly);
	}

	void ModelProbe::Visit(Graphics& gfx, Shape::Model& model, Shape::ModelNode& root) noexcept
	{
		ImGui::Columns(2);
		ImGui::BeginChild("##NodeTree", ImVec2(0.0f, 231.5f), false, ImGuiWindowFlags_HorizontalScrollbar);
		root.Accept(gfx, *this);
		ImGui::EndChild();
		ImGui::NextColumn();
		ImGui::NewLine();
		if (selectedNode != nullptr)
		{
			selectedNode->Object::Accept(gfx, *this);
			Visit(gfx, *selectedNode);
		}
		ImGui::Columns(1);
	}
}