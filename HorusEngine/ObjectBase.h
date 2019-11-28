#pragma once
#include "IDrawable.h"

namespace GFX::Object
{
	template<typename T>
	class ObjectBase : public IDrawable
	{
		static const Resource::IndexBuffer * indexBuffer;
		static std::vector<std::unique_ptr<Resource::IBindable>> staticBinds;

		inline const std::vector<std::unique_ptr<Resource::IBindable>> & GetStaticBinds() const noexcept override { return staticBinds; }
		inline const Resource::IndexBuffer * GetStaticIndexBuffer() const noexcept override { return indexBuffer; }

	protected:
		inline bool IsStaticInit() const noexcept { return staticBinds.size(); }
		
		constexpr void AddStaticBind(std::unique_ptr<Resource::IBindable> bind) noexcept(!IS_DEBUG)
		{
			assert("Must use AddStaticIndexBuffer to bind index buffer" && typeid(*bind) != typeid(Resource::IndexBuffer));
			staticBinds.push_back(std::move(bind));
		}

		constexpr void AddStaticIndexBuffer(std::unique_ptr<Resource::IndexBuffer> buffer) noexcept(!IS_DEBUG)
		{
			assert("Attempting to add index buffer a second time" && indexBuffer == nullptr);
			indexBuffer = buffer.get();
			staticBinds.push_back(std::move(buffer));
		}
	};

	template<typename T>
	const Resource::IndexBuffer * ObjectBase<T>::indexBuffer = nullptr;

	template<typename T>
	std::vector<std::unique_ptr<Resource::IBindable>> ObjectBase<T>::staticBinds;
}
