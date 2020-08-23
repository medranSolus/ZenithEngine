#pragma once
#include "IBindable.h"
#include "IBufferResource.h"
#include "RenderGraphCompileException.h"

namespace GFX::Pipeline::RenderPass::Base
{
	class Source
	{
		std::string name;

		static bool IsValidName(const std::string& name);

	public:
		Source(const std::string& name);
		virtual ~Source() = default;

		inline const std::string& GetName() const noexcept { return name; }
		virtual inline std::shared_ptr<GFX::Resource::IBindable> LinkBindable() { throw RGC_EXCEPT("Source \"" + GetName() + "\" cannot be used as bindable!"); }
		virtual inline std::shared_ptr<Resource::IBufferResource> LinkBuffer() { throw RGC_EXCEPT("Source \"" + GetName() + "\" cannot be used as pipeline buffer resource!"); }
		virtual inline void ValidateLink() const {}
	};
}