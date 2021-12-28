#pragma once
#include "Data.h"

namespace ZE::GFX::Material
{
	// Unique name for single material
	typedef std::string_view ID;

	// Material factory responsible for bookkeeping all data schemas and managing materials created with single schema
	class Factory final
	{
		U32 schemaCount = 0;
		Schema* schemas = nullptr;
		std::unordered_map<ID, U32> materials;

	public:
		Factory() = default;
		ZE_CLASS_DELETE(Factory);
		~Factory();

		constexpr Schema& GetSchema(U32 index) noexcept { ZE_ASSERT(index < schemaCount, "Trying to get Schema out of range!"); return schemas[index]; }
		Schema& GetMaterial(ID name) noexcept { ZE_ASSERT(Contains(name), "Cannot find material!"); return GetSchema(materials.at(name)); }
		Data CreateMaterial(Device& dev, U32 index) { return { dev, GetSchema(index) }; }
		Data CreateMaterial(Device& dev, ID name) { return { dev, GetMaterial(name) }; }
		bool Contains(ID name) const noexcept { return materials.contains(name); }

		U32 AddDataBinding(Device& dev, const SchemaDesc& desc);
		U32 RegisterMaterial(Device& dev, const SchemaDesc& desc, ID name);
	};
}