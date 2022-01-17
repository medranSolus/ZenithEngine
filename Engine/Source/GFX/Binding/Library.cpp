#include "GFX/Binding/Library.h"

namespace ZE::GFX::Binding
{
	Library::~Library()
	{
		if (schemas)
			schemas.DeleteArray();
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