#pragma once

namespace ZE::GFX::Pipeline
{
	// Status result of building render graph
	enum class BuildResultCode : U8
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
		ErrorPassCopyInitDataCallbackNotProvided,
		ErrorPassInitCallbackNotProvided,
		ErrorResourceInputLayoutMismatch,
		ErrorResourceOutputLayoutMismatch,
		ErrorResourceLayoutChangesInIncorrectOrder,
		ErrorPassGroupEvalutaionFunctionMissing,
		ErrorPassGroupMultipleDynamicProcessors,
		ErrorUnknown,
	};

	// Error handling for building render graph
	class BuildResult : public std::error_category
	{
	protected:
		BuildResult() = default;

	public:
		ZE_CLASS_MOVE(BuildResult);
		virtual ~BuildResult() = default;

		static constexpr const std::error_category& GetCategory() noexcept { static BuildResult CATEGORY; return CATEGORY; }
		static Status Make(BuildResultCode code) noexcept { return { static_cast<int>(code), GetCategory() }; }

		const char* name() const noexcept override { return "Rneder Graph Build Result"; }
		std::string message(int condition) const override;
	};
}

#define ZE_RG_BUILD_ERROR(code) ZE::GFX::Pipeline::BuildResult::Make(code)