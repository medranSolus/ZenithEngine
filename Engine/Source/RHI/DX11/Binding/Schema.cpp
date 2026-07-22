#include "RHI/DX11/Binding/Schema.h"

namespace ZE::RHI::DX11::Binding
{
	Expected<Schema> Schema::Create(GFX::Device& dev, const GFX::Binding::SchemaDesc& desc) noexcept
	{
		Schema schema;

		schema.samplersCount = Utils::SafeCast<U32>(desc.Samplers.size());
		if (schema.samplersCount)
		{
			schema.samplers = std::make_unique_for_overwrite<std::pair<U32, DX::ComPtr<ISamplerState>>[]>(desc.Samplers.size());
			for (U32 i = 0; const auto& samplerDesc : desc.Samplers)
			{
				D3D11_SAMPLER_DESC splrDesc = {};
				splrDesc.Filter = GetFilterType(samplerDesc.Type);
				splrDesc.AddressU = GetTextureAddressMode(samplerDesc.Address.U);
				splrDesc.AddressV = GetTextureAddressMode(samplerDesc.Address.V);
				splrDesc.AddressW = GetTextureAddressMode(samplerDesc.Address.W);
				splrDesc.MipLODBias = samplerDesc.MipLevelBias;
				splrDesc.MaxAnisotropy = samplerDesc.MaxAnisotropy;
				splrDesc.ComparisonFunc = GetComparisonFunc(samplerDesc.Comparison);
				*reinterpret_cast<ColorF4*>(splrDesc.BorderColor) = GetStaticBorderColor(samplerDesc.EdgeColor);
				splrDesc.MinLOD = samplerDesc.MinLOD;
				splrDesc.MaxLOD = samplerDesc.MaxLOD;

				schema.samplers[i].first = samplerDesc.Slot;
				ZE_DX_RET_FAILED_EXPECT(dev.Get().dx11.GetDevice()->CreateSamplerState(&splrDesc, &schema.samplers[i++].second));
			}
		}

		// Check input data
		schema.count = 0;
		U32 dataCount = 0;
		GFX::ShaderPresenceMask shaderPresenceConstants;
		for (const auto& entry : desc.Ranges)
		{
			entry.Validate();

			if (entry.Flags & (GFX::Binding::RangeFlag::Constant | GFX::Binding::RangeFlag::BufferPack))
			{
				if (entry.Flags & GFX::Binding::RangeFlag::Constant)
				{
					ZE_ASSERT(!shaderPresenceConstants.SetPresence(entry.Shaders), "Only single Constant per shader type is allowed!");
				}
				++schema.count;
				++dataCount;
			}
			else if (entry.Flags & GFX::Binding::RangeFlag::BufferPackAppend)
			{
				++dataCount;
			}
			else
			{
				schema.count += entry.Count;
				dataCount += entry.Count;
			}
			schema.activeShaders.SetPresence(entry.Shaders);
		}
		ZE_ASSERT(schema.activeShaders.IsCompute() != schema.activeShaders.IsGfx(),
			"Compute Shader binding detected alongside other shaders resulting in disabling all other graphics shader stages. Check creation of the SchemaDesc!");

		// Gather slots
		schema.slots = std::make_unique_for_overwrite<SlotInfo[]>(schema.count);
		schema.slotsData = std::make_unique_for_overwrite<SlotData[]>(dataCount);
		for (U32 i = 0, j = 0; const auto& entry : desc.Ranges)
		{
			if (entry.Flags & GFX::Binding::RangeFlag::Constant)
			{
				ZE_ASSERT(entry.StartSlot < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT, "Too much shader slots!");
				schema.slots[i++] = { j, 1 };
				schema.slotsData[j++] = { entry.Shaders, entry.StartSlot, 1 };
			}
			else
			{
				ZE_ASSERT(entry.StartSlot + entry.Count < D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT,
					"Too much shader slots!");

				if (entry.Flags & GFX::Binding::RangeFlag::BufferPack)
				{
					schema.slots[i++] = { j, 1 };
					schema.slotsData[j++] = { entry.Shaders, entry.StartSlot, entry.Count };
				}
				else if (entry.Flags & GFX::Binding::RangeFlag::BufferPackAppend)
				{
					++schema.slots[i - 1].SlotsCount;
					schema.slotsData[j - 1] = { entry.Shaders, entry.StartSlot, entry.Count };
				}
				else
				{
					for (U32 k = 0; k < entry.Count; ++k)
					{
						schema.slots[i++] = { j, 1 };
						schema.slotsData[j++] = { entry.Shaders, entry.StartSlot + k, 1 };
					}
				}
			}
		}
		return schema;
	}

	void Schema::SetCompute(GFX::CommandList& cl) const noexcept
	{
		ZE_ASSERT(activeShaders.IsCompute(), "Schema is not created for compute pass!");

		auto* ctx = cl.Get().dx11.GetContext();
		for (U32 i = 0; i < samplersCount; ++i)
			ctx->CSSetSamplers(samplers[i].first, 1, samplers[i].second.GetAddressOf());

		ID3D11UnorderedAccessView* nullUAV[D3D11_1_UAV_SLOT_COUNT] = { nullptr };
		ctx->CSSetUnorderedAccessViews(0, D3D11_1_UAV_SLOT_COUNT, nullUAV, nullptr);
	}

	void Schema::SetGraphics(GFX::CommandList& cl) const noexcept
	{
		ZE_ASSERT(!activeShaders.IsCompute(), "Schema is not created for graphics pass!");

		auto* ctx = cl.Get().dx11.GetContext();
		if (activeShaders.IsVertex())
			for (U32 i = 0; i < samplersCount; ++i)
				ctx->VSSetSamplers(samplers[i].first, 1, samplers[i].second.GetAddressOf());
		if (activeShaders.IsDomain())
			for (U32 i = 0; i < samplersCount; ++i)
				ctx->DSSetSamplers(samplers[i].first, 1, samplers[i].second.GetAddressOf());
		if (activeShaders.IsHull())
			for (U32 i = 0; i < samplersCount; ++i)
				ctx->HSSetSamplers(samplers[i].first, 1, samplers[i].second.GetAddressOf());
		if (activeShaders.IsGeometry())
			for (U32 i = 0; i < samplersCount; ++i)
				ctx->GSSetSamplers(samplers[i].first, 1, samplers[i].second.GetAddressOf());
		if (activeShaders.IsPixel())
			for (U32 i = 0; i < samplersCount; ++i)
				ctx->PSSetSamplers(samplers[i].first, 1, samplers[i].second.GetAddressOf());
	}
}