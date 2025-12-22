#pragma once
#include "GFX/Pipeline/RenderGraph.h"
#include "GUI/ImGuiManager.h"
#include "StartupConfig.h"

namespace ZE
{
	// Main Zenith Engine component containing all the rendering logic
	class Engine final : public StartupConfig
	{
		enum Flags : U8 { Initialized, ExecuteUploadSync, PixCapture, PixCaptureInProgress, Count };

		double prevTime = 0.0;
		GFX::Graphics graphics;
		GUI::ImGuiManager imgui;
		Window::MainWindow window;
		GFX::Pipeline::RenderGraphBuilder graphBuilder;
		GFX::Pipeline::RenderGraph renderGraph;
		Data::AssetsStreamer assets;
		std::bitset<Flags::Count> flags;

		bool UploadSync();

	public:
		Engine(const SettingsInitParams& params) noexcept : StartupConfig(params) {}
		ZE_CLASS_DELETE(Engine);
		virtual ~Engine();

		constexpr GFX::Graphics& Gfx() noexcept { return graphics; }
		constexpr GUI::ImGuiManager& ImGui() noexcept { return imgui; }
		constexpr Window::MainWindow& Window() noexcept { return window; }
		constexpr GFX::Pipeline::RenderGraph& RenderGraph() noexcept { return renderGraph; }
		constexpr Data::AssetsStreamer& Assets() noexcept { return assets; }

		// Initialization method that must be called before using engine
		bool Init(const EngineParams& params);
		// Need to be called before starting first frame
		void Start(EID camera) noexcept;
		void ShowRenderGraphDebugUI() noexcept;
		// Returns number of update steps that have to be taken by simulations multiplied by delta time
		double BeginFrame(double deltaTime, U64 maxUpdateSteps);
		void EndFrame();
	};
}