#include "GFX/Material/Factory.h"

namespace ZE::GFX::Material
{
	Factory::~Factory()
	{
		if (schemas)
			delete[] schemas;
	}

	U32 Factory::AddDataBinding(Device& dev, const SchemaDesc& desc)
	{
		Schema* newSchemas = new Schema[schemaCount + 1];
		for (U32 i = 0; i < schemaCount; ++i)
			newSchemas[i] = std::move(schemas[i]);
		newSchemas[schemaCount].Init(dev, desc);
		delete[] schemas;
		schemas = newSchemas;
		return schemaCount++;
	}

	U32 Factory::RegisterMaterial(Device& dev, const SchemaDesc& desc, ID name)
	{
		ZE_ASSERT(!materials.contains(name), "Given material is already registered!");
		U32 location = AddDataBinding(dev, desc);
		materials.emplace(name, location);
		return location;
	}
}