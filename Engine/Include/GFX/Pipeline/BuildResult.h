#pragma once
#include "Types.h"

namespace ZE::GFX::Pipeline
{
	// Status result of building render graph
	enum class BuildResult : U8
	{
		Success = 0,
		ErrorTooManyPasses,
		ErrorTooManyResources,
		ErrorPassWrongOutputSet,
		ErrorOutputAsInputSamePass,
		ErrorPassCircularDependency,
		ErrorPassEmptyName,
		ErrorPassNameClash,
		ErrorMultiplePresentPassesWithSameConnectorName,
		ErrorProcessorAllInputsOptional,
		ErrorConfigNotLoaded,
		ErrorPassInputIncorrectFormat,
		ErrorPassNameNotFound,
		ErrorNotAllInputsFound,
		ErrorMissingNonOptionalInput,
		ErrorWrongResourceDimensionsFlags,
		ErrorIncorrectResourceUsage,
		ErrorIncorrectResourceFormat,
		ErrorWrongResourceConfiguration,
		ErrorPassExecutionCallbackNotProvided,
		ErrorPassFreeInitDataCallbackNotProvided,
		ErrorPassInitCallbackNotProvided,
		ErrorResourceInputLayoutMismatch,
		ErrorResourceOutputLayoutMismatch,
		ErrorResourceLayoutChangesInIncorrectOrder,
	};

	constexpr const char* DecodeBuildResult(BuildResult result) noexcept;

#pragma region Functions
	constexpr const char* DecodeBuildResult(BuildResult result) noexcept
	{
#define DECODE(res, msg) case BuildResult::##res: return (msg)
		switch (result)
		{
		default: return "UNKNOWN";
			DECODE(Success, "Success");
			DECODE(ErrorTooManyPasses, "Specified too many passes in RenderGraphDesc");
			DECODE(ErrorTooManyResources, "Specified too many resources in RenderGraphDesc");
			DECODE(ErrorPassWrongOutputSet, "One of the passes in group doesn't have matching outputs with rest of the group");
			DECODE(ErrorOutputAsInputSamePass, "Output of pass used as input of the same pass");
			DECODE(ErrorPassCircularDependency, "Provided render graph is not acyclic");
			DECODE(ErrorPassEmptyName, "Passes belonging to group need to have additional pass names");
			DECODE(ErrorPassNameClash, "Passes in group with same names");
			DECODE(ErrorMultiplePresentPassesWithSameConnectorName, "Multiple passes in same group are present at the same time in computed configuration");
			DECODE(ErrorProcessorAllInputsOptional, "No present inputs to the processing node");
			DECODE(ErrorConfigNotLoaded, "Computing graph without loading it's config");
			DECODE(ErrorPassInputIncorrectFormat, "All passes inputs should be in format: <PASS_CONNECTOR_NAME>.<PASS_OUTPUT_RESOURCE>");
			DECODE(ErrorPassNameNotFound, "Missing dependency pass in graph");
			DECODE(ErrorNotAllInputsFound, "Pass without all required input is present");
			DECODE(ErrorMissingNonOptionalInput, "Pass input is missing resource linkage");
			DECODE(ErrorWrongResourceDimensionsFlags, "Incorrect dimension flags of resource");
			DECODE(ErrorIncorrectResourceUsage, "Resource usage in graph is not compatible with it's description");
			DECODE(ErrorIncorrectResourceFormat, "Resource format is not supported for current resource usage in graph");
			DECODE(ErrorWrongResourceConfiguration, "Wrong configuration flags for resource");
			DECODE(ErrorPassExecutionCallbackNotProvided, "No execution callback for given render pass");
			DECODE(ErrorPassInitCallbackNotProvided, "No initialization callback for given render pass while init data has been provided");
		}
#undef DECODE
	}
#pragma endregion
}