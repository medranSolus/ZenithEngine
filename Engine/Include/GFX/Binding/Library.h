#pragma once
#include "Schema.h"

namespace ZE::GFX::Binding
{
	// Library responsible for bookkeeping all data schemas
	class Library final
	{
		std::vector<Schema> schemas;
		std::unordered_map<std::string, U32> locations;

	public:
		Library() = default;
		ZE_CLASS_DELETE(Library);
		~Library() = default;

		constexpr Schema& GetSchema(U32 index) noexcept { ZE_ASSERT(index < schemas.size(), "Trying to get Schema out of range!"); return schemas.at(index); }

		// If common binding is registered returns its index and returns false (no more work to be done)
		bool FetchBinding(const std::string& name, U32& index) const noexcept;
		Expected<U32> RegisterCommonBinding(Device& dev, const SchemaDesc& desc, const std::string& name) noexcept;
		Expected<U32> AddDataBinding(Device& dev, const SchemaDesc& desc) noexcept;
	};
}