#pragma once
#include "IShape.h"
#include "Vertex.h"

namespace GFX::Shape
{
	template<typename T>
	class BaseShape : public IShape
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

		template<typename R>
		R * GetResource() noexcept;

		constexpr void AddBind(std::unique_ptr<Resource::IBindable> bind) noexcept(!IS_DEBUG);
		constexpr void AddStaticBind(std::unique_ptr<Resource::IBindable> bind) noexcept(!IS_DEBUG);
		constexpr void AddStaticIndexBuffer(std::unique_ptr<Resource::IndexBuffer> buffer) noexcept(!IS_DEBUG);
		constexpr void AddIndexBuffer(std::unique_ptr<Resource::IndexBuffer> buffer) noexcept(!IS_DEBUG);
	};

	template<typename T>
	const Resource::IndexBuffer * BaseShape<T>::staticIndexBuffer = nullptr;

	template<typename T>
	std::vector<std::unique_ptr<Resource::IBindable>> BaseShape<T>::staticBinds;

	template<typename T>
	constexpr void BaseShape<T>::AddBind(std::unique_ptr<Resource::IBindable> bind) noexcept(!IS_DEBUG)
	{
		assert("*Must* use AddIndexBuffer to bind index buffer" && typeid(*bind) != typeid(Resource::IndexBuffer));
		binds.emplace_back(std::move(bind));
	}

	template<typename T>
	constexpr void BaseShape<T>::AddStaticBind(std::unique_ptr<Resource::IBindable> bind) noexcept(!IS_DEBUG)
	{
		assert("Must use AddStaticIndexBuffer to bind index buffer" && typeid(*bind) != typeid(Resource::IndexBuffer));
		staticBinds.emplace_back(std::move(bind));
	}

	template<typename T>
	constexpr void BaseShape<T>::AddStaticIndexBuffer(std::unique_ptr<Resource::IndexBuffer> buffer) noexcept(!IS_DEBUG)
	{
		assert("Attempting to add index buffer a second time" && staticIndexBuffer == nullptr);
		staticIndexBuffer = buffer.get();
		staticBinds.emplace_back(std::move(buffer));
	}

	template<typename T>
	constexpr void BaseShape<T>::AddIndexBuffer(std::unique_ptr<Resource::IndexBuffer> buffer) noexcept(!IS_DEBUG)
	{
		assert("Attempting to add index buffer a second time" && indexBuffer == nullptr);
		indexBuffer = buffer.get();
		binds.emplace_back(std::move(buffer));
	}

	template<typename T>
	template<typename R>
	R * BaseShape<T>::GetResource() noexcept
	{
		for (auto & bind : binds)
			if (auto res = dynamic_cast<R*>(&bind))
				return res;
		return nullptr;
	}
}
