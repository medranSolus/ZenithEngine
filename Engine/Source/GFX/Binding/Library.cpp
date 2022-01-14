#include "GFX/Binding/Library.h"

namespace ZE::GFX::Binding
{
	Library::~Library()
	{
		if (schemas)
			delete[] schemas;
	}

	U32 Library::AddDataBinding(Device& dev, const SchemaDesc& desc)
	{
		Schema* newSchemas = new Schema[schemaCount + 1];
		for (U32 i = 0; i < schemaCount; ++i)
			newSchemas[i] = std::move(schemas[i]);
		newSchemas[schemaCount].Init(dev, desc);
		delete[] schemas;
		schemas = newSchemas;
		return schemaCount++;
	}
}