#include "GFX/Pipeline/RenderPass/SSAO.h"

namespace ZE::GFX::Pipeline::RenderPass::SSAO
{
	ExecuteData* Setup(Device& dev, RendererBuildData& buildData)
	{
		ExecuteData* passData = new ExecuteData;
		passData->CL.Init(dev, CommandType::Compute);

		Binding::SchemaDesc desc;
		desc.AddRange({ 1, 0, Resource::ShaderType::Compute, Binding::RangeFlag::UAV | Binding::RangeFlag::BufferPack });
		desc.AddRange({ 1, 0, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
		desc.AddRange({ 1, 1, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack });
		desc.AddRange({ 1, 2, Resource::ShaderType::Compute, Binding::RangeFlag::SRV | Binding::RangeFlag::BufferPack | Binding::RangeFlag::StaticData });
		desc.AddRange({ 1, 12, Resource::ShaderType::Compute, Binding::RangeFlag::CBV });
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Compute);
		passData->BindingIndexSSAO = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::Shader ssao(L"AmbientOcclusionCS");
		passData->StateSSAO.Init(dev, ssao, buildData.BindingLib.GetSchema(passData->BindingIndexSSAO));

		desc.Clear();
		desc.AddRange({ 1, 0, Resource::ShaderType::Compute, Binding::RangeFlag::UAV | Binding::RangeFlag::BufferPack });
		desc.Append(buildData.RendererSlots, Resource::ShaderType::Compute);
		passData->BindingIndexBlur = buildData.BindingLib.AddDataBinding(dev, desc);

		Resource::Shader blur(L"SSAOBlurCS");
		passData->StateBlur.Init(dev, blur, buildData.BindingLib.GetSchema(passData->BindingIndexBlur));

		Resource::Texture::PackDesc noiseDesc;
		std::vector<Surface> surfaces;
		surfaces.emplace_back(NOISE_WIDTH, NOISE_HEIGHT, PixelFormat::R32G32_Float);

		std::mt19937_64 engine(std::random_device{}());
		float* buffer = reinterpret_cast<float*>(surfaces.front().GetBuffer());
		for (U32 i = 0; i < NOISE_SIZE * 2; ++i)
			buffer[i] = Math::RandNDC(engine);

		noiseDesc.Options = Resource::Texture::PackOption::StaticCreation;
		noiseDesc.AddTexture(Resource::Texture::Type::Tex2D, Resource::Texture::Usage::NonPixelShader, std::move(surfaces));
		passData->Noise.Init(dev, noiseDesc);
		dev.StartUpload();

		return passData;
	}

	void Execute(Device& dev, CommandList& cl, RendererExecuteData& renderData, PassData& passData)
	{
		Resources ids = *passData.Buffers.CastConst<Resources>();
		ExecuteData& data = *reinterpret_cast<ExecuteData*>(passData.OptData);

		data.CL.Reset(dev);
		data.CL.Open(dev, data.StateSSAO);

		ZE_DRAW_TAG_BEGIN(data.CL, L"SSAO", Pixel(0x89, 0xCF, 0xF0));
		Binding::Context ctxSSAO{ renderData.Bindings.GetSchema(data.BindingIndexSSAO) };
		ctxSSAO.BindingSchema.SetCompute(data.CL);
		renderData.Buffers.SetUAV(data.CL, ctxSSAO, ids.SSAO);
		renderData.Buffers.SetSRV(data.CL, ctxSSAO, ids.Normal);
		renderData.Buffers.SetSRV(data.CL, ctxSSAO, ids.Depth);
		data.Noise.Bind(data.CL, ctxSSAO);
		renderData.DynamicBuffer.Bind(data.CL, ctxSSAO);
		renderData.SettingsBuffer.Bind(data.CL, ctxSSAO);
		data.CL.Compute(dev, 64, 32, 1); // Need to decouple from screen dimmensions
		renderData.Buffers.BarrierUAV(data.CL, ids.SSAO);

		data.StateBlur.Bind(data.CL);
		Binding::Context ctxBlur{ renderData.Bindings.GetSchema(data.BindingIndexBlur) };
		ctxBlur.BindingSchema.SetCompute(data.CL);
		renderData.Buffers.SetUAV(data.CL, ctxBlur, ids.SSAO);
		renderData.SettingsBuffer.Bind(data.CL, ctxBlur);
		data.CL.Compute(dev, 8, 8, 1);
		ZE_DRAW_TAG_END(data.CL);

		data.CL.Close(dev);
		dev.ExecuteCompute(data.CL);
	}
}