#include "GFX/Binding/Library.h"

namespace ZE::GFX::Binding
{
	bool Library::FetchBinding(const std::string& name, U32& index) const noexcept
	{
		if (locations.contains(name))
		{
			index = locations.at(name);
			return false;
		}
		return true;
	}

	Expected<U32> Library::RegisterCommonBinding(Device& dev, const SchemaDesc& desc, const std::string& name) noexcept
	{
		ZE_ASSERT(!locations.contains(name), "Common data binding already registered!");

		U32 index = 0;
		ZE_EXPECT_RET_FAILED(index, AddDataBinding(dev, desc));
		locations.emplace(name, index);
		return index;
	}

	Expected<U32> Library::AddDataBinding(Device& dev, const SchemaDesc& desc) noexcept
	{
		Schema schema = {};
		ZE_EXPECT_RET_FAILED(schema, Schema::Create(dev, desc));
		schemas.emplace_back(std::move(schema));
		return schemas.size() - 1;
	}
}