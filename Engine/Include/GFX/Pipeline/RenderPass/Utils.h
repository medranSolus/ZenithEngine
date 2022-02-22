#pragma once
#include "GFX/Resource/CBuffer.h"
#include "GFX/TransformBuffer.h"
#include "Data/Transform.h"

namespace ZE::GFX::Pipeline::RenderPass::Utils
{
	// Resizes vector of temporary transform buffers
	template<U64 ShrinkStepOffset = 0>
	void ResizeTransformBuffers(Device& dev, std::vector<Resource::CBuffer>& transformBuffers, U64 count);

	// Computes single model transform
	void SetupTransformData(const Data::Transform& transform, ModelTransform& model, const Matrix& viewProjectionTransposed);

#pragma region Functions
	template<U64 ShrinkStepOffset>
	void ResizeTransformBuffers(Device& dev, std::vector<Resource::CBuffer>& transformBuffers, U64 count)
	{
		U64 buffCount = Math::DivideRoundUp(count * sizeof(ModelTransform), sizeof(TransformBuffer)) / sizeof(TransformBuffer);
		if (buffCount + ShrinkStepOffset < transformBuffers.size())
		{
			for (U64 i = buffCount; i < transformBuffers.size(); ++i)
				transformBuffers.at(i).Free(dev);
			transformBuffers.resize(buffCount);
		}
		else if (buffCount > transformBuffers.size())
		{
			U64 i = transformBuffers.size();
			transformBuffers.resize(buffCount);
			for (; i < buffCount; ++i)
				transformBuffers.at(i).Init(dev, nullptr, sizeof(TransformBuffer), true);
		}
	}
#pragma endregion
}