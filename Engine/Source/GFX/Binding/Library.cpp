#include "GFX/Binding/Library.h"

namespace ZE::GFX::Binding
{
	Library::~Library()
	{
		if (schemas)
			schemas.DeleteArray();
	}

	bool Library::FetchBinding(const std::string& name, U32& index) const noexcept
	{
		if (locations.contains(name))
		{
			index = locations.at(name);
			return false;
		}
		return true;
	}

	U32 Library::RegisterCommonBinding(Device& dev, const SchemaDesc& desc, const std::string& name)
	{
		ZE_ASSERT(!locations.contains(name), "Common data binding already registered!");

		U32 index = AddDataBinding(dev, desc);
		locations.emplace(name, index);
		return index;
	}

	U32 Library::AddDataBinding(Device& dev, const SchemaDesc& desc)
	{
		Schema* newSchemas = new Schema[schemaCount + 1];
		if (schemaCount > 0)
		{
			for (U32 i = 0; i < schemaCount; ++i)
				newSchemas[i] = std::move(schemas[i]);
			schemas.DeleteArray(newSchemas);
		}
		else
			schemas = newSchemas;
		schemas[schemaCount].Init(dev, desc);
		return schemaCount++;
	}
}