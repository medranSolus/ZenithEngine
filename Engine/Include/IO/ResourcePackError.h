#pragma once

namespace ZE::IO
{
	// Status returned by resource pack operations
	enum class ResourcePackStatus : U8
	{
		Ok,
		ErrorUnkown,
		ErrorBadSignature,
		ErrorUnknowVersion,
		ErrorNoResources,
		ErrorUnknownResourceEntry,
		ErrorMissingMaterialEntries,
		ErrorUnknownTextureSchema,
		ErrorEmptyTextureCount,
		ErrorIncorrectTextureEntry,
		ErrorIncorrectMaterialBufferSize,
	};

	// Error category for operation on resource packs
	class ResourcePackError : public std::error_category
	{
	protected:
		ResourcePackError() = default;

	public:
		ZE_CLASS_MOVE(ResourcePackError);
		virtual ~ResourcePackError() = default;

		static constexpr const std::error_category& GetCategory() noexcept { static ResourcePackError CATEGORY; return CATEGORY; }
		static Status Make(ResourcePackStatus result) noexcept { return { static_cast<int>(result), GetCategory() }; }

		const char* name() const noexcept override { return "Resource Pack Error"; }
		std::string message(int condition) const override;
	};
}

#define ZE_RESPACK_ERROR(result) ZE::IO::ResourcePackError::Make(result)