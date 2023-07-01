#include "GFX/API/DX11/Binding/Schema.h"

namespace ZE::GFX::API::DX11::Binding
{
	Schema::Schema(GFX::Device& dev, const GFX::Binding::SchemaDesc& desc)
	{
		ZE_DX_ENABLE(dev.Get().dx11);

		samplersCount = desc.Samplers.size();
		if (samplersCount)
		{
			samplers = new std::pair<U32, DX::ComPtr<ISamplerState>>[desc.Samplers.size()];
			for (U32 i = 0; const auto& samplerDesc : desc.Samplers)
			{
				D3D11_SAMPLER_DESC desc = {};
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
				ZE_DX_THROW_FAILED(dev.Get().dx11.GetDevice()->CreateSamplerState(&desc, &samplers[i++].second));
			}
		}

		// Check input data
		count = 0;
		U32 dataCount = 0;
		ShaderPresenceMask shaderPresenceConstants;
		for (const auto& entry : desc.Ranges)
		{
			entry.Validate();

			if (entry.Flags & (GFX::Binding::RangeFlag::Constant | GFX::Binding::RangeFlag::BufferPack))
			{
				if (entry.Flags & GFX::Binding::RangeFlag::Constant)
				{
					ZE_ASSERT(!shaderPresenceConstants.SetPresence(entry.Shaders), "Only single Constant per shader type is allowed!");
				}
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
			activeShaders.SetPresence(entry.Shaders);
		}
		ZE_ASSERT(activeShaders.IsCompute() != activeShaders.IsGfx(),
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

	void Schema::Free(GFX::Device& dev) noexcept
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
		ZE_ASSERT(activeShaders.IsCompute(), "Schema is not created for compute pass!");

		for (U32 i = 0; i < samplersCount; ++i)
			cl.Get().dx11.GetContext()->CSSetSamplers(samplers[i].first, 1, samplers[i].second.GetAddressOf());
	}

	void Schema::SetGraphics(GFX::CommandList& cl) const noexcept
	{
		ZE_ASSERT(!activeShaders.IsCompute(), "Schema is not created for graphics pass!");

		if (activeShaders.IsVertex())
			for (U32 i = 0; i < samplersCount; ++i)
				cl.Get().dx11.GetContext()->VSSetSamplers(samplers[i].first, 1, samplers[i].second.GetAddressOf());
		if (activeShaders.IsDomain())
			for (U32 i = 0; i < samplersCount; ++i)
				cl.Get().dx11.GetContext()->DSSetSamplers(samplers[i].first, 1, samplers[i].second.GetAddressOf());
		if (activeShaders.IsHull())
			for (U32 i = 0; i < samplersCount; ++i)
				cl.Get().dx11.GetContext()->HSSetSamplers(samplers[i].first, 1, samplers[i].second.GetAddressOf());
		if (activeShaders.IsGeometry())
			for (U32 i = 0; i < samplersCount; ++i)
				cl.Get().dx11.GetContext()->GSSetSamplers(samplers[i].first, 1, samplers[i].second.GetAddressOf());
		if (activeShaders.IsPixel())
			for (U32 i = 0; i < samplersCount; ++i)
				cl.Get().dx11.GetContext()->PSSetSamplers(samplers[i].first, 1, samplers[i].second.GetAddressOf());
	}
}