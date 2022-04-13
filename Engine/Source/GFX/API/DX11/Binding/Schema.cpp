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
		U32 dataCount = 0;
		for (const auto& entry : desc.Ranges)
		{
			entry.Validate();

			if (entry.Flags & (GFX::Binding::RangeFlag::Constant | GFX::Binding::RangeFlag::BufferPack))
			{
				++count;
				++dataCount;
			}
			else if (entry.Flags & GFX::Binding::RangeFlag::BufferPackAppend)
			{
				++dataCount;
			}
			else
			{
				count += entry.Count;
				dataCount += entry.Count;
			}

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
		slots = new SlotInfo[count];
		slotsData = new SlotData[dataCount];
		for (U32 i = 0, j = 0; const auto& entry : desc.Ranges)
		{
			if (entry.Flags & GFX::Binding::RangeFlag::Constant)
			{
				ZE_ASSERT(entry.StartSlot < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT, "Too much shader slots!");
				slots[i++] = { j, 1 };
				slotsData[j++] = { entry.Shaders, entry.StartSlot, 1 };
			}
			else
			{
				ZE_ASSERT(entry.StartSlot + entry.Count < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT,
					"Too much shader slots!");

				if (entry.Flags & GFX::Binding::RangeFlag::BufferPack)
				{
					slots[i++] = { j, 1 };
					slotsData[j++] = { entry.Shaders, entry.StartSlot, entry.Count };
				}
				else if (entry.Flags & GFX::Binding::RangeFlag::BufferPackAppend)
				{
					++slots[i - 1].SlotsCount;
					slotsData[j - 1] = { entry.Shaders, entry.StartSlot, entry.Count };
				}
				else
				{
					for (U32 k = 0; k < entry.Count; ++k)
					{
						slots[i++] = { j, 1 };
						slotsData[j++] = { entry.Shaders, entry.StartSlot + k, 1 };
					}
				}
			}
		}
	}

	Schema::~Schema()
	{
		if (slots)
			slots.DeleteArray();
		if (slotsData)
			slotsData.DeleteArray();
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