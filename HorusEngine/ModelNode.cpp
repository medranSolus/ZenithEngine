#include "ModelNode.h"

namespace GFX::Shape
{
	ModelNode::ModelNode(unsigned long long id, const std::string& name, std::vector<std::shared_ptr<Mesh>>&& nodeMeshes,
		const DirectX::FXMMATRIX& nodeTransform) noexcept
		: Object(name), id(id), meshes(std::move(nodeMeshes))
	{
		DirectX::XMStoreFloat4x4(&baseTransform, nodeTransform);
		currentTransform = std::make_shared<DirectX::XMFLOAT4X4>();
		for (auto& mesh : meshes)
			mesh->SetTransformMatrix(currentTransform);
	}

	void ModelNode::Submit(Pipeline::RenderCommander& renderer, const DirectX::FXMMATRIX& higherTransform) noexcept(!IS_DEBUG)
	{
		const DirectX::XMMATRIX transformMatrix = DirectX::XMLoadFloat4x4(transform.get()) *
			DirectX::XMLoadFloat4x4(&baseTransform) * higherTransform;
		DirectX::XMStoreFloat4x4(currentTransform.get(), transformMatrix);
		for (const auto& mesh : meshes)
			mesh->Submit(renderer);
		for (const auto& child : children)
			child->Submit(renderer, transformMatrix);
	}

	void ModelNode::Accept(Graphics& gfx, Probe::ModelProbe& probe) noexcept
	{
		if (probe.PushNode(*this))
		{
			for (auto& child : children)
				child->Accept(gfx, probe);
			probe.PopNode();
		}
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