#pragma once
#include "GFX/Pipeline/Resource/IBufferResource.h"
#include "Exception/RenderGraphCompileException.h"

namespace GFX::Pipeline::RenderPass::Base
{
	class Source
	{
		std::string name;

		static bool IsValidName(const std::string& name) noexcept;

	protected:
		Source(std::string&& sourceName);

	public:
		Source(Source&&) = default;
		Source(const Source&) = default;
		Source& operator=(Source&&) = default;
		Source& operator=(const Source&) = default;
		virtual ~Source() = default;

		constexpr const std::string& GetName() const noexcept { return name; }

		virtual GfxResPtr<GFX::Resource::IBindable> LinkBindable() { throw RGC_EXCEPT("Source \"" + GetName() + "\" cannot be used as bindable!"); }
		virtual GfxResPtr<Resource::IBufferResource> LinkBuffer() { throw RGC_EXCEPT("Source \"" + GetName() + "\" cannot be used as pipeline buffer resource!"); }
	};
}