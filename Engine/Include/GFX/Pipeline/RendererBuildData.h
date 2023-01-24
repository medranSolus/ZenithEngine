#pragma once
#include "GFX/Resource/Texture/Schema.h"
#include "GFX/Binding/Library.h"
#include "GFX/Resource/PipelineStateDesc.h"
#include "GFX/Resource/Shader.h"

namespace ZE::GFX::Pipeline
{
	// Structures used by RenderGraph implementation to create RenderPasses
	struct RendererBuildData
	{
		// Allows creation of materials and data bindings. Always save only index as Schema address may change during setup
		Binding::Library& BindingLib;
		// Holds descriptions for TexturePack's internal structure to be used later during their creation and accesses
		Resource::Texture::Library& TextureLib;
		// If pass require global renderer data in shaders just simply append this SchemaDesc
		Binding::SchemaDesc RendererSlots;
		std::unordered_map<std::string, Resource::Shader> ShaderCache;
		// When several passes are gonna share same binding slots simply save and retrieve it's index inside the map
		std::unordered_map<std::string, U32> SchemaLocations;
		// If graphic's PSO can be used in multiple passes it should be placed here to avoid state duplication.
		// Just insert 'PipelineStates.size()' along the desc and use it later on as index into PSO array.
		// Index for relevant data binding is also required.
		// Schema index | PSO desc | PSO location = PipelineStates.size()
		std::unordered_map<std::string, std::pair<U32, std::pair<Resource::PipelineStateDesc, U32>>> PipelineStates;

		void FreeShaderCache(Device& dev) noexcept { for (auto& shader : ShaderCache) shader.second.Free(dev); }
	};
}