#pragma once
#include "Schema.h"

namespace ZE::GFX::Binding
{
	// Library responsible for bookkeeping all data schemas
	class Library final
	{
		U32 schemaCount = 0;
		Schema* schemas = nullptr;

	public:
		Library() = default;
		ZE_CLASS_DELETE(Library);
		~Library();

		constexpr Schema& GetSchema(U32 index) noexcept { ZE_ASSERT(index < schemaCount, "Trying to get Schema out of range!"); return schemas[index]; }

		U32 AddDataBinding(Device& dev, const SchemaDesc& desc);
	};
}