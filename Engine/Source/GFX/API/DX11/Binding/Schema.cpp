#include "GFX/API/DX11/Binding/Schema.h"
#include "GFX/API/DX/GraphicsException.h"

namespace ZE::GFX::API::DX11::Binding
{
	Schema::Schema(GFX::Device& dev, const GFX::Binding::SchemaDesc& desc)
	{
		ZE_ASSERT(desc.Ranges.size() > 0, "Empty SchemaDesc!");
		ZE_GFX_ENABLE(dev.Get().dx11);

		samplersCount = desc.Samplers.size();
		if (samplersCount)
		{
			samplers = new std::pair<U32, DX::ComPtr<ID3D11SamplerState>>[desc.Samplers.size()];
			for (U32 i = 0; const auto& samplerDesc : desc.Samplers)
			{
				D3D11_SAMPLER_DESC desc;
				desc.Filter = GetFilterType(samplerDesc.Type);
				desc.AddressU = GetTextureAddressMode(samplerDesc.Address.U);
				desc.AddressV = GetTextureAddressMode(samplerDesc.Address.V);
				desc.AddressW = GetTextureAddressMode(samplerDesc.Address.W);
				desc.MipLODBias = samplerDesc.MipLevelBias;
				desc.MaxAnisotropy = samplerDesc.MaxAnisotropy;
				desc.ComparisonFunc = GetComparisonFunc(samplerDesc.Comparison);
				*reinterpret_cast<ColorF4*>(desc.BorderColor) = GetStaticBorderColor(samplerDesc.EdgeColor);
				desc.MinLOD = samplerDesc.MinLOD;
				desc.MaxLOD = samplerDesc.MaxLOD;

				samplers[i].first = samplerDesc.Slot;
				ZE_GFX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateSamplerState(&desc, &samplers[i++].second));
			}
		}

		// Check input data
		count = 0;
		for (const auto& entry : desc.Ranges)
		{
			entry.Validate();

			if (entry.Flags & GFX::Binding::RangeFlag::Constant)
				++count;
			else
				count += entry.Count;

			if (entry.Shaders & GFX::Resource::ShaderType::Compute)
				activeShaders[5] = true;
			else
			{
				if (entry.Shaders & GFX::Resource::ShaderType::Vertex)
					activeShaders[0] = true;
				if (entry.Shaders & GFX::Resource::ShaderType::Domain)
					activeShaders[1] = true;
				if (entry.Shaders & GFX::Resource::ShaderType::Hull)
					activeShaders[2] = true;
				if (entry.Shaders & GFX::Resource::ShaderType::Geometry)
					activeShaders[3] = true;
				if (entry.Shaders & GFX::Resource::ShaderType::Pixel)
					activeShaders[4] = true;
			}
		}
		ZE_ASSERT(activeShaders[5] && !activeShaders[0] && !activeShaders[1] && !activeShaders[2] && !activeShaders[3] && !activeShaders[4] || !activeShaders[5],
			"Compute Shader binding detected alongside other shaders resulting in disabling all other graphics shader stages. Check creation of the SchemaDesc!");

		// Gather slots
		slots = new std::pair<GFX::Resource::ShaderTypes, U32>[count];
		for (U32 i = 0; const auto& entry : desc.Ranges)
		{
			if (entry.Flags & GFX::Binding::RangeFlag::Constant)
			{
				ZE_ASSERT(entry.StartSlot < D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT, "Too much shader slots!");
				slots[i++] = { entry.Shaders, entry.StartSlot };
			}
			else
			{
				ZE_ASSERT(entry.StartSlot + entry.Count < D3D12_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT,
					"Too much shader slots!");

				for (U32 j = 0; j < entry.Count; ++j)
					slots[i++] = { entry.Shaders, entry.StartSlot + j };
			}
		}
	}

	Schema::~Schema()
	{
		if (slots)
			slots.DeleteArray();
		if (samplers)
			samplers.DeleteArray();
	}

	void Schema::SetCompute(GFX::CommandList& cl) const noexcept
	{
		ZE_ASSERT(activeShaders[5], "Schema is not created for compute pass!");

		for (U32 i = 0; i < samplersCount; ++i)
			cl.Get().dx11.GetContext()->CSSetSamplers(samplers[i].first, 1, samplers[i].second.GetAddressOf());
	}

	void Schema::SetGraphics(GFX::CommandList& cl) const noexcept
	{
		ZE_ASSERT(!activeShaders[5], "Schema is not created for graphics pass!");

		if (activeShaders[0])
			for (U32 i = 0; i < samplersCount; ++i)
				cl.Get().dx11.GetContext()->VSSetSamplers(samplers[i].first, 1, samplers[i].second.GetAddressOf());
		if (activeShaders[1])
			for (U32 i = 0; i < samplersCount; ++i)
				cl.Get().dx11.GetContext()->DSSetSamplers(samplers[i].first, 1, samplers[i].second.GetAddressOf());
		if (activeShaders[2])
			for (U32 i = 0; i < samplersCount; ++i)
				cl.Get().dx11.GetContext()->HSSetSamplers(samplers[i].first, 1, samplers[i].second.GetAddressOf());
		if (activeShaders[3])
			for (U32 i = 0; i < samplersCount; ++i)
				cl.Get().dx11.GetContext()->GSSetSamplers(samplers[i].first, 1, samplers[i].second.GetAddressOf());
		if (activeShaders[4])
			for (U32 i = 0; i < samplersCount; ++i)
				cl.Get().dx11.GetContext()->PSSetSamplers(samplers[i].first, 1, samplers[i].second.GetAddressOf());
	}
}