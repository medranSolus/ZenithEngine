#include "RHI/DX/DXGI.h"
#include "Settings.h"

namespace ZE::RHI::DX
{
	Expected<ComPtr<IFactory>> CreateFactory() noexcept
	{
		// Create proper DXGI factory
		ComPtr<IDXGIFactory2> oldFactory = nullptr;
		ZE_DX_RET_FAILED_EXPECT(CreateDXGIFactory2(_ZE_DEBUG_GFX_API ? DXGI_CREATE_FACTORY_DEBUG : 0, IID_PPV_ARGS(&oldFactory)));
		ComPtr<IFactory> factory = nullptr;
		ZE_DX_RET_FAILED_EXPECT(oldFactory.As(&factory));

		return factory;
	}

	ComPtr<IAdapter> CreateAdapter(ComPtr<IFactory> factory) noexcept
	{
		ComPtr<IAdapter> adapter = nullptr;
		for (U32 i = 0; true; ++i)
		{
			// Get highest possible performant GPU
			if (FAILED(factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter))))
				break;
			DXGI_ADAPTER_DESC3 desc;
			if (SUCCEEDED(adapter->GetDesc3(&desc)))
			{
				// Ignore Basic Render Driver adapter
				if ((desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE) == 0)
				{
					switch (desc.VendorId)
					{
					case 0x10DE:
					{
						Settings::GpuVendor = GFX::VendorGPU::Nvidia;
						break;
					}
					case 0x1002:
					{
						Settings::GpuVendor = GFX::VendorGPU::AMD;
						break;
					}
					case 0x8086:
					{
						Settings::GpuVendor = GFX::VendorGPU::Intel;
						break;
					}
					}

					// Check if any monitor is attached to the GPU
					// or if Nvidia Optimus is enabled (no output but still returned as first GPU in the list)
					ComPtr<IDXGIOutput> output = nullptr;
					if (SUCCEEDED(adapter->EnumOutputs(0, &output)) || Settings::GpuVendor == GFX::VendorGPU::Nvidia)
						break;
					else
						Settings::GpuVendor = GFX::VendorGPU::Unknown;
					break;
				}
			}
		}
		return adapter;
	}

	Expected<ComPtr<ISwapChain>> CreateSwapChain(ComPtr<IFactory> factory,
		IUnknown* device, HWND window, bool shaderInput, U32& presentFlags) noexcept
	{
		DXGI_SWAP_CHAIN_DESC1 swapDesc = {};
		swapDesc.Width = 0; // Use window sizes
		swapDesc.Height = 0;
		swapDesc.Format = GetDXFormat(Utils::RemoveSRGB(Settings::BackbufferFormat));
		swapDesc.Stereo = FALSE;
		swapDesc.SampleDesc.Count = 1; // Used only in bitblt swap model
		swapDesc.SampleDesc.Quality = 0;
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | (shaderInput ? DXGI_USAGE_SHADER_INPUT : 0); // Pipeline draws to this buffer
		swapDesc.BufferCount = Settings::GetBackbufferCount(); // [2;16]
		swapDesc.Scaling = DXGI_SCALING_STRETCH;
		swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //http://aka.ms/dxgiflipmodel
		swapDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapDesc.Flags = 0;

		// Check support for tearing (vsync-off), required in variable rate displays
		BOOL allowTearing = FALSE;
		presentFlags = 0;
		if (SUCCEEDED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
		{
			if (allowTearing == TRUE)
			{
				swapDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
				presentFlags = DXGI_PRESENT_ALLOW_TEARING;
			}
		}
		else
			Logger::Warning("Cannot determine support for tearing in swapchain!");

		ComPtr<IDXGISwapChain1> tempChain = nullptr;
		ZE_DX_RET_FAILED_EXPECT(factory->CreateSwapChainForHwnd(device,
			window, &swapDesc, nullptr, nullptr, &tempChain));
		ComPtr<ISwapChain> swapChain = nullptr;
		ZE_DX_RET_FAILED_EXPECT(tempChain.As(&swapChain));

		// Don't use Alt+Enter Windows handling, only borderless fulsscreen window
		if (FAILED(factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER)))
			Logger::Warning("Cannot set window association with newly created swapchain!");
		return swapChain;
	}
}