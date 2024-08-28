#pragma once
#include "GFX/Graphics.h"
#include "RenderGraphBuilder.h"

namespace ZE::GFX::Pipeline
{
	/*
		- better access to interfaces from the passes
		- rebuilding graph when new type of content is on the scene (keep only the passes that produce something)
		- allow for change of resolution at the start of the frame
		- introduce callback for setting pass data from outside (maybe register as data blob inside RenderGraph with enum?)
		- callback for dynamic updating of pass (can result in rebuilding of graph)
		- common interface for GPU resource aliasing
		- pass to be able to specify after which other pass it should be started, more like a hint if it's not possible due to data flow
		- option for post-creation initialization so basically splitting setup step into 2
		- distincion for render graph updates:
			- soft: some effects are not enabled and some buffers won't be used, so just different resources in flight, good for frame-to-frame changes when effects don't have input data
			- hard: resolution changed, some effect is disabled by the user so it's required to recreate whole frame buffer with different resource aliasing scheme
	*/

	/* IDEA FOR UI AND OPTIONS REGARDING RENDER PASSES

	Have function that returns what type of options given render pass can hold (enum similar to CorePassType with extensions possible)
	If some known value would be encountered then create UI element for it and ask for value with accessor function (will pass in a value and enum that defines type of value too).
	Same way for setting value inside the ExecuteData. If custom value encountered then use callback for anything related to it. Basically:

	Pass::GetSettings() { return { Setting::SharpnessFloat, CustomSetting::Wololo }; }

	for (auto sett : Pass::GetSettings())
	{
		if (sett == Setting::SharpnessFloat)
		{
			float sharpness = 0.0f;
			Pass::GetSetting(Setting::SharpnessFloat, &sharpness); <- just switch with reinterpret_cast<float*> for data access
			// do some UI magic (TM)
			Pass::SetSetting(Setting::SharpnessFloat, &sharpness);
		}
		else
			callback(sett, passInfo); <- All Set/GetSetting(s) calls will be probably stored as callback so it can be accessed easily
	}

	Maybe don't needed for ImGui but for proper UI would be really nice to have as some GFX settings tab. Or maybe let's try to set it up for ImGui as a proxy so later on only UI part would need to change?
	But what if we have enum indicating what type of pass it is, this way we wouldn't have to make any other checks, etc. since we can just cast to proper type anyway.

	So it looks like there are 2 options to handle this, more direct approach would be to have enum only for given pass and then use some casting to access data directly or to have enums for the settings
	so each pass would be handling them transparently. Second option seems more pass agnostic so order of options could be more easily controlled
	(vector of options we are interested in displaying in some manner and pass assigned to it that have them inside, so single iteration over passes to gather options, then decide which one should be first, the go over them and display UI...).
	But first options seems move versalite in terms of possibilities of displaying. We want to show options from pass X so we find it in a list and create UI. We can order passes first in a manner we want to consume them and then display UI.
	First option should be faster since it requires only one custom sort per options screen (let's assume there will be 2 screens, generic graphics options and extended while most of the stuff would be on extended page).
	But then again this custom sort can be a bit heavy. But then again, what if we want to order things in certain way, with enum for options it would be easy to set up with just number for each setting, so after gathering all possible settings
	we would be left with just normal sort and then single iteration.
	Third problem araises when there would be options that would change what passes are used. If we change ex. NIS to FSR2 it would be good to have quality setting pop-up for FSR2
	and then after that it would be good to have access for FSR2 related settings, not only to NIS ones. There is need for component that could be able to register what values we would like to display, something like UI builder with input values behind it...

	Options:
	1. Just cast based on enum value from the pass
	2. Create some settings enum and let every pass handle it's getter and setter for that value
	3. Expand a bit on option 2 and create some UI builder with such enums so we won't be editing exact values inside the pass, but instead we will just edit this opaque values before applying them to passes.
	   Every pass would be able to register it's options inside such builder as UI elements (with type of variables, list of them, maybe even custom ones with render pass own getter/setter?) and then commit it all with single button, etc.
	   That would be nice foundation for UI in general, logic will be handled by this builder, ordering of elements, what type of elements to appear with their general position too,
	   and then it would be displayed by some proper GUI rendering framework (could be ImGui or proper custom UI).

	First option seems okay for now at least with "Debug UI" mode. Second looks okay for debug UI but could be just too much of a hassle so for normal UI option 3 looks good. And option 3 can also display ImGui before proper UI is rolled.
	Question remains for option 3 what type UI rendering it would support or if it's agnostic to it. Retained or immediate mode.
	If immediate mode then it's pretty straightforward, just build UI based on builded info. If retained, then how to ensure proper caching of UI? https://news.ycombinator.com/item?id=19768156

	Or maybe have some seperate UI systems for in-game UI (as UI elements rendered inside the game world for matching position when ex. there need to be some text over a character) but with ability to be screen-only UI (HUD)?...
	Maybe mixed UI mode, for settings and game menus it would be okay to render in immediate mode and for in-game stuff retained mode? And in "retained" mode jsut use UI elemetns as objects inside the world but with special rendering to them?
	But again, what for animated UI elements...

	For now just go with option 1 for simplicity and later on decide on proper UI. Immediate mode can be used for tools too maybe? https://gist.github.com/bkaradzic/853fd21a15542e0ec96f7268150f1b62#why-i-think-immediate-mode-gui-is-way-to-go-for-gamedev-tools
	*/

	// Accessor for data of render pass, optional, otherwise generic function will be used for simplicity with Invalid type.
	// When updating data that requires change in render graph need to handle this outside and indicate it properly, data that is dynamic
	// (ex. is just passed to the pass and used, no other setup required) can be changed freely.
	// After such change it may be needed to call Setup function again for required passes (when render target buffers are recreated so it's hard update)
	//
	// Maybe not needed at all since ExecuteData is held outside render pass facilities since it's only set of data and functions so it would be better to hold some
	// master RenderPass list with below enum and all other function pointers along with it
	//std::pair<void*, Type> GetData() noexcept { return { nullptr, static_cast<Type>(CoreType::UpscaleNIS) }; }

	// TO THINK: What about FfxBackendInterface and create resource function, we need a way to request additional resources to be added to our

	/* Types of passes:
	 - Static - creates data to be consumed later on in pipeline
	 - Dynamic - also creates data but can stop doing it on per frame basis (triggers soft update)
	 - Processing - depends only on input data, when no data provided this pass can be removed from execution (later passes have to account for missing input)
	*/

	// RenderGraph Execution: last exec group with some last transitions for backbuffer and others, have to be main group

	// For FFX it would be okay to create reflection system to identify which pass belongs to which effect, so basically: searchPass(getPassType(FFX_EFFECT_CACAO)).AddInnerBuffer()

	// Class for running render graphs previously created by some builder. Should be generic so it will accept any input provided for data, passes, etc.
	class RenderGraph final
	{
		friend class RenderGraphBuilder;

		// If update of settings caused any need for graph update
		enum class PendingUpdate : U8
		{
			None, // No update needed
			Soft, // Only some passes are disabled so re-route flow of the graph, leaving all else intact
			Hard, // Critical changes to the graph structure that require recreation of whole framebuffer
		};

		// Group of render passes that can be run in parallel with no resource dependency
		struct ParallelPassGroup
		{
			struct PassInfo
			{
				U32 PassID = UINT32_MAX;
				PassExecuteCallback Exec = nullptr;
				PassData Data;
				std::unique_ptr<RID[]> Resources;
			};

			std::vector<BarrierTransition> StartBarriers;
			U32 PassCount = 0;
			std::unique_ptr<PassInfo[]> Passes;
		};

		// Group of passes to be executed in single submission
		struct ExecutionGroup
		{
#if _ZE_RENDER_GRAPH_SINGLE_THREAD
			typedef U64 FenceValue;
#else
			typedef UA64 FenceValue;
#endif
			U32 PassGroupCount = 0;
			std::unique_ptr<ParallelPassGroup[]> PassGroups;
			bool QueueWait = false;
			FenceValue WaitFence = 0;
			FenceValue* SignalFence = nullptr;
			// Only related to cross-queue resources and for last group preparing the backbuffer
			std::vector<BarrierTransition> EndBarriers;
		};

		PendingUpdate update = PendingUpdate::None;

		std::unique_ptr<std::array<ExecutionGroup, 2>[]> passExecGroups;
		U32 execGroupCount = 0;
		RendererPassExecuteData execData;
		ChainPool<CommandList> asyncListChain;

	public:
		RenderGraph() = default;
		ZE_CLASS_MOVE(RenderGraph);
		~RenderGraph() = default;

		void Execute(Graphics& gfx);

		// Here handle the data from passes to create ImGUI element for them
		// (not window, only master app above will do the windows and use here only stuff that is not dependant on pixels,
		// more proportions but with some minimal one that will allow for vertical scroll, etc.)
		//
		// When Invalid type for RenderPass is encountered then just do nothing, for renderer data use provided callback
		// and same for custom passes so user can handle them (AND with CustomStart to get them all, for default always issue warning)
		//
		// Need to indicate change to settings that require rebuild
		void ShowDebugUI() noexcept;
	};
}