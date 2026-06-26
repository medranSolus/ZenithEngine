#include "RHI/DX11/Resource/DynamicCBuffer.h"

namespace ZE::RHI::DX11::Resource
{
	Status DynamicCBuffer::AllocBlock(GFX::Device& dev) noexcept
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.ByteWidth = BLOCK_SIZE;
		bufferDesc.StructureByteStride = 0;

		DX::ComPtr<IBuffer> buff;
		ZE_DX_RET_FAILED(dev.Get().dx11.GetDevice()->CreateBuffer(&bufferDesc, nullptr, &buff));
		ZE_DX_SET_ID(buff, "DynamicCBuffer_" + std::to_string(blocks.size()));

		D3D11_MAPPED_SUBRESOURCE subres = {};
		ZE_DX_RET_FAILED(dev.Get().dx11.GetMainContext()->Map(buff.Get(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &subres));

		buffer = reinterpret_cast<U8*>(subres.pData);
		blocks.emplace_back(std::move(buff), Data::Library<U32, U32>{});

		return {};
	}

	DynamicCBuffer::~DynamicCBuffer()
	{
		if (buffer && blocks.size())
		{
			DX::ComPtr<ID3D11Device> dev;
			blocks.at(currentBlock).first->GetDevice(&dev);

			DX::ComPtr<ID3D11DeviceContext> ctx;
			dev->GetImmediateContext(&ctx);
			ctx->Unmap(blocks.at(currentBlock).first.Get(), 0);
		}
	}

	Expected<DynamicCBuffer> DynamicCBuffer::Create(GFX::Device& dev) noexcept
	{
		DynamicCBuffer dcb;
		ZE_CODE_RET_FAILED_EXPECT(dcb.AllocBlock(dev));
		return dcb;
	}

	Expected<GFX::Resource::DynamicBufferAlloc> DynamicCBuffer::Alloc(GFX::Device& dev, const void* values, U32 bytes) noexcept
	{
		ZE_ASSERT(blocks.size(), "Dynamic buffer has been freed already!");
		ZE_ASSERT(bytes <= BLOCK_SIZE, "Structure too large for dynamic buffer!");

		const U32 newBlock = Math::AlignUp(bytes, 256U);
#ifndef _ZE_RENDER_GRAPH_SINGLE_THREAD
		const std::lock_guard<std::mutex> lock(allocLock);
#endif
		if (nextOffset + newBlock > BLOCK_SIZE)
		{
			dev.Get().dx11.GetMainContext()->Unmap(blocks.at(currentBlock).first.Get(), 0);
			nextOffset = 0;
			if (++currentBlock >= blocks.size())
				AllocBlock(dev);
			else
			{
				D3D11_MAPPED_SUBRESOURCE subres = {};
				ZE_DX_RET_FAILED_EXPECT(dev.Get().dx11.GetMainContext()->Map(blocks.at(currentBlock).first.Get(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &subres));
				buffer = reinterpret_cast<U8*>(subres.pData);
			}
		}
		std::memcpy(buffer + nextOffset, values, bytes);

		blocks.at(currentBlock).second.Add(nextOffset, newBlock / 16);
		GFX::Resource::DynamicBufferAlloc info
		{
			nextOffset,
			currentBlock
		};
		nextOffset += newBlock;
		return info;
	}

	void DynamicCBuffer::Bind(GFX::CommandList& cl, GFX::Binding::Context& bindCtx, const GFX::Resource::DynamicBufferAlloc& allocInfo) const noexcept
	{
		ZE_ASSERT(blocks.size(), "Dynamic buffer has been freed already!");
		ZE_ASSERT(allocInfo.Block <= currentBlock, "Block out of range!");
		ZE_ASSERT(allocInfo.Offset < BLOCK_SIZE, "Offset out of range!");

		auto& schema = bindCtx.BindingSchema.Get().dx11;
		auto slotInfo = schema.GetCurrentSlot(bindCtx.Count++);
		ZE_ASSERT(slotInfo.SlotsCount == 1, "Constant buffer slot should only contain 1 entry!");

		auto slotData = schema.GetSlotData(slotInfo.DataStart);
		ZE_ASSERT(slotData.Count == 1, "Constant buffer slot should only be bound as single buffer!");

		auto& block = blocks.at(allocInfo.Block).first;
		const U32 offset = allocInfo.Offset / 16;
		const U32 size = blocks.at(allocInfo.Block).second.Get(allocInfo.Offset);
		auto* ctx = cl.Get().dx11.GetContext();

		if (slotData.Shaders & GFX::Resource::ShaderType::Compute)
			ctx->CSSetConstantBuffers1(slotData.BindStart, 1, block.GetAddressOf(), &offset, &size);
		else
		{
			if (slotData.Shaders & GFX::Resource::ShaderType::Vertex)
				ctx->VSSetConstantBuffers1(slotData.BindStart, 1, block.GetAddressOf(), &offset, &size);
			if (slotData.Shaders & GFX::Resource::ShaderType::Domain)
				ctx->DSSetConstantBuffers1(slotData.BindStart, 1, block.GetAddressOf(), &offset, &size);
			if (slotData.Shaders & GFX::Resource::ShaderType::Hull)
				ctx->HSSetConstantBuffers1(slotData.BindStart, 1, block.GetAddressOf(), &offset, &size);
			if (slotData.Shaders & GFX::Resource::ShaderType::Geometry)
				ctx->GSSetConstantBuffers1(slotData.BindStart, 1, block.GetAddressOf(), &offset, &size);
			if (slotData.Shaders & GFX::Resource::ShaderType::Pixel)
				ctx->PSSetConstantBuffers1(slotData.BindStart, 1, block.GetAddressOf(), &offset, &size);
		}
	}

	Status DynamicCBuffer::StartFrame(GFX::Device& dev) noexcept
	{
		ZE_ASSERT(blocks.size(), "Dynamic buffer has been freed already!");

		const U64 blockCount = blocks.size();
		if (blockCount > 1)
		{
			auto ctx = dev.Get().dx11.GetMainContext();
			ctx->Unmap(blocks.at(currentBlock).first.Get(), 0);
			buffer = nullptr;

			if (currentBlock + BLOCK_SHRINK_STEP < blockCount)
				blocks.resize(currentBlock + 1);

			D3D11_MAPPED_SUBRESOURCE subres = {};
			for (auto& block : blocks)
			{
				ZE_DX_RET_FAILED(ctx->Map(block.first.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subres));
				ctx->Unmap(block.first.Get(), 0);

				block.second.Clear();
			}

			ZE_DX_RET_FAILED(ctx->Map(blocks.front().first.Get(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &subres));
			buffer = reinterpret_cast<U8*>(subres.pData);
			currentBlock = 0;
		}
		nextOffset = 0;

		return {};
	}
}