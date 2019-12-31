#pragma once
#include "IDrawable.h"
#include "Vertex.h"

namespace GFX::Object
{
	template<typename T>
	class ObjectBase : public IDrawable
	{
		static const Resource::IndexBuffer * staticIndexBuffer;
		static std::vector<std::unique_ptr<Resource::IBindable>> staticBinds;
		const Resource::IndexBuffer * indexBuffer = nullptr;
		std::vector<std::unique_ptr<Resource::IBindable>> binds;

		inline const std::vector<std::unique_ptr<Resource::IBindable>> & GetBinds() const noexcept override { return binds; }
		inline const std::vector<std::unique_ptr<Resource::IBindable>> & GetStaticBinds() const noexcept override { return staticBinds; }
		inline const Resource::IndexBuffer * GetStaticIndexBuffer() const noexcept override { return staticIndexBuffer; }
		inline const Resource::IndexBuffer * GetIndexBuffer() const noexcept override { return indexBuffer; }

	protected:
		inline bool IsStaticInit() const noexcept { return staticBinds.size(); }

		constexpr void AddBind(std::unique_ptr<Resource::IBindable> bind) noexcept(!IS_DEBUG)
		{
			assert("*Must* use AddIndexBuffer to bind index buffer" && typeid(*bind) != typeid(Resource::IndexBuffer));
			binds.emplace_back(std::move(bind));
		}

		constexpr void AddStaticBind(std::unique_ptr<Resource::IBindable> bind) noexcept(!IS_DEBUG)
		{
			assert("Must use AddStaticIndexBuffer to bind index buffer" && typeid(*bind) != typeid(Resource::IndexBuffer));
			staticBinds.emplace_back(std::move(bind));
		}

		constexpr void AddStaticIndexBuffer(std::unique_ptr<Resource::IndexBuffer> buffer) noexcept(!IS_DEBUG)
		{
			assert("Attempting to add index buffer a second time" && staticIndexBuffer == nullptr);
			staticIndexBuffer = buffer.get();
			staticBinds.emplace_back(std::move(buffer));
		}

		constexpr void AddIndexBuffer(std::unique_ptr<Resource::IndexBuffer> buffer) noexcept(!IS_DEBUG)
		{
			assert("Attempting to add index buffer a second time" && indexBuffer == nullptr);
			indexBuffer = buffer.get();
			binds.emplace_back(std::move(buffer));
		}

		template<typename T>
		T * GetResource() noexcept
		{
			for (auto & bind : binds)
				if (auto res = dynamic_cast<T*>(&bind))
					return res;
			return nullptr;
		}
	};

	template<typename T>
	const Resource::IndexBuffer * ObjectBase<T>::staticIndexBuffer = nullptr;

	template<typename T>
	std::vector<std::unique_ptr<Resource::IBindable>> ObjectBase<T>::staticBinds;
}
