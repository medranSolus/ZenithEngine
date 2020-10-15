#include "SSAOPass.h"
#include "RenderPassesBase.h"
#include "PipelineResources.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Pipeline::RenderPass
{
	inline Data::CBuffer::DCBLayout SSAOPass::MakeLayoutKernel() noexcept
	{
		static Data::CBuffer::DCBLayout layout;
		static bool initNeeded = true;
		if (initNeeded)
		{
			layout.Add(DCBElementType::Array, "kernel");
			layout["kernel"].InitArray(DCBElementType::Float3, SSAO_KERNEL_SIZE);
			layout.Add(DCBElementType::Float2, "tileDimensions");
			initNeeded = false;
		}
		return layout;
	}

	inline Data::CBuffer::DCBLayout SSAOPass::MakeLayoutOptions() noexcept
	{
		static Data::CBuffer::DCBLayout layout;
		static bool initNeeded = true;
		if (initNeeded)
		{
			layout.Add(DCBElementType::UInteger, "kernelSize");
			layout.Add(DCBElementType::Float, "sampleRadius");
			layout.Add(DCBElementType::Float, "bias");
			initNeeded = false;
		}
		return layout;
	}

	SSAOPass::SSAOPass(Graphics& gfx, const std::string& name)
		: FullscreenPass(gfx, name)
	{
		renderTarget = std::make_shared<Resource::RenderTargetShaderInput>(gfx, 11U, DXGI_FORMAT_R32_FLOAT);
		ssaoScratchBuffer = std::make_shared<Resource::RenderTargetShaderInput>(gfx, 11U, DXGI_FORMAT_R32_FLOAT);

		kernelBuffer = GFX::Resource::ConstBufferExPixelCache::Get(gfx, typeid(SSAOPass).name(), MakeLayoutKernel(), 12U);
		kernelBuffer->GetBuffer()["tileDimensions"] = DirectX::XMFLOAT2(gfx.GetWidth() / 4.0f, gfx.GetHeight() / 4.0f);
		std::mt19937_64 engine(std::random_device{}());
		for (size_t i = 0; i < SSAO_KERNEL_SIZE; ++i)
		{
			const DirectX::XMVECTOR sample = DirectX::XMVectorSet(Math::RandNDC(engine),
				Math::RandNDC(engine), Math::Rand01(engine), 0.0f);

			float scale = static_cast<float>(i) / SSAO_KERNEL_SIZE;
			scale = Math::Lerp(0.1f, 1.0f, scale * scale);

			DirectX::XMStoreFloat3(&kernelBuffer->GetBuffer()["kernel"][i],
				DirectX::XMVectorMultiply(DirectX::XMVector3Normalize(sample), DirectX::XMVectorSet(scale, scale, scale, 0.0f)));
		}

		optionsBuffer = GFX::Resource::ConstBufferExPixelCache::Get(gfx, typeid(SSAOPass).name(), MakeLayoutOptions(), 13U);
		optionsBuffer->GetBuffer()["kernelSize"] = static_cast<uint32_t>(SSAO_KERNEL_SIZE);
		optionsBuffer->GetBuffer()["sampleRadius"] = 0.5f;
		optionsBuffer->GetBuffer()["bias"] = 0.000001f;

		AddBindableSink<GFX::Resource::IBindable>("geometryBuffer");
		AddBindableSink<Resource::DepthStencilShaderInput>("depth");

		RegisterSource(Base::SourceDirectBindable<Resource::IRenderTarget>::Make("ssaoBuffer", renderTarget));
		RegisterSource(Base::SourceDirectBuffer<Resource::IRenderTarget>::Make("ssaoScratch", ssaoScratchBuffer));
		RegisterSource(Base::SourceDirectBindable<GFX::Resource::ConstBufferExPixelCache>::Make("ssaoKernel", kernelBuffer));

		AddBind(optionsBuffer);
		AddBind(kernelBuffer);
		AddBind(GFX::Resource::PixelShader::Get(gfx, "AmbientOcclusionPS"));
		AddBind(GFX::Resource::Sampler::Get(gfx, GFX::Resource::Sampler::Type::Point, true, 1U));
		AddBind(GFX::Resource::Blender::Get(gfx, GFX::Resource::Blender::Type::None));

		Surface ssaoNoise(SSAO_NOISE_SIZE / 4, SSAO_NOISE_SIZE / 4, DXGI_FORMAT_R32G32_FLOAT);
		float* buffer = reinterpret_cast<float*>(ssaoNoise.GetBuffer());
		for (size_t i = 0; i < SSAO_NOISE_SIZE * 2; ++i)
			buffer[i] = Math::RandNDC(engine);
		AddBind(GFX::Resource::Texture::Get(gfx, ssaoNoise, "ssaoNoise", 12U));
	}

	void SSAOPass::Execute(Graphics& gfx)
	{
		assert(mainCamera);
		mainCamera->BindPS(gfx);
		FullscreenPass::Execute(gfx);
	}

	void SSAOPass::ShowWindow(Graphics& gfx)
	{
		uint32_t size = optionsBuffer->GetBufferConst()["kernelSize"];
		float radius = optionsBuffer->GetBufferConst()["sampleRadius"];
		float bias = optionsBuffer->GetBufferConst()["bias"];
		ImGui::Text("SSAO:");
		if (ImGui::DragInt("Kernel size", reinterpret_cast<int*>(&size), 1.0f, 1, SSAO_KERNEL_SIZE))
			optionsBuffer->GetBuffer()["kernelSize"] = size;
		if (ImGui::DragFloat("Radius##SSAO", &radius, 0.01f, 0.01f, FLT_MAX, "%.2f"))
			optionsBuffer->GetBuffer()["sampleRadius"] = radius;
		if (ImGui::DragFloat("Bias##SSAO", &bias, 0.0000001f, 0.0f, 1.0f, "%.7f"))
			optionsBuffer->GetBuffer()["bias"] = bias;
	}
}