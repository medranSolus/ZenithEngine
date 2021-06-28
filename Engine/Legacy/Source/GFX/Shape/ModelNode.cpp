#include "GFX/Shape/ModelNode.h"

namespace ZE::GFX::Shape
{
	ModelNode::ModelNode(U64 id, std::string&& name, std::vector<std::shared_ptr<Mesh>>&& nodeMeshes,
		const Matrix& nodeTransform) noexcept
		: Object(std::forward<std::string>(name)), id(id), meshes(std::move(nodeMeshes))
	{
		Math::XMStoreFloat4x4(&baseTransform, nodeTransform);
		currentTransform = std::make_shared<Float4x4>();
		for (auto& mesh : meshes)
			mesh->SetTransformMatrix(currentTransform);
	}

	void ModelNode::SetOutline() noexcept
	{
		for (const auto& mesh : meshes)
			mesh->SetOutline();
		for (const auto& child : children)
			child->SetOutline();
	}

	void ModelNode::DisableOutline() noexcept
	{
		for (const auto& mesh : meshes)
			mesh->DisableOutline();
		for (const auto& child : children)
			child->DisableOutline();
	}

	void ModelNode::Submit(U64 channelFilter, const Matrix& higherTransform) const noexcept
	{
		const Matrix transformMatrix = Math::XMLoadFloat4x4(transform.get()) *
			Math::XMLoadFloat4x4(&baseTransform) * higherTransform;
		Math::XMStoreFloat4x4(currentTransform.get(), transformMatrix);
		for (const auto& mesh : meshes)
			mesh->Submit(channelFilter);
		for (const auto& child : children)
			child->Submit(channelFilter, transformMatrix);
	}

	bool ModelNode::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		bool change = false;
		for (const auto& mesh : meshes)
			change |= mesh->Accept(gfx, probe);
		return change;
	}

	bool ModelNode::Accept(Graphics& gfx, Probe::ModelProbe& probe) noexcept
	{
		bool change = false;
		if (probe.PushNode(*this))
		{
			for (auto& child : children)
				change |= child->Accept(gfx, probe);
			probe.PopNode();
		}
		return change;
	}

	void ModelNode::SetMesh(Graphics& gfx, bool meshOnly) noexcept
	{
		if (isMesh != meshOnly)
		{
			isMesh = meshOnly;
			if (isMesh)
				for (auto& nodeMesh : meshes)
					nodeMesh->SetTopologyMesh(gfx);
			else
				for (auto& nodeMesh : meshes)
					nodeMesh->SetTopologyPlain(gfx);
		}
		for (auto& node : children)
			node->SetMesh(gfx, meshOnly);
	}
}