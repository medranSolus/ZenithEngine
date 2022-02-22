#include "GFX/Pipeline/RenderPass/Utils.h"

namespace ZE::GFX::Pipeline::RenderPass::Utils
{
	void SetupTransformData(const Data::Transform& transform, ModelTransform& model, const Matrix& viewProjectionTransposed)
	{
		model.Model = Math::XMMatrixTranspose(Math::XMMatrixScalingFromVector(Math::XMLoadFloat3(&transform.Scale)) *
			Math::XMMatrixRotationQuaternion(Math::XMLoadFloat4(&transform.Rotation)) *
			Math::XMMatrixTranslationFromVector(Math::XMLoadFloat3(&transform.Position)));
		model.ModelViewProjection = viewProjectionTransposed * model.Model;
	}
}