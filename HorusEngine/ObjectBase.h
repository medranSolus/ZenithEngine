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
		DirectX::XMFLOAT3 angle = { 0.0f,0.0f,0.0f };
		DirectX::XMFLOAT3 pos;

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

	public:
		constexpr ObjectBase(float x0, float y0, float z0) noexcept : pos(x0, y0, z0) {}

		constexpr DirectX::XMFLOAT3 GetPos() const noexcept { return pos; }
		constexpr void SetPos(DirectX::XMFLOAT3 position) noexcept { pos = position; }
		constexpr DirectX::XMFLOAT3 GetAngle() const noexcept { return angle; }
		constexpr void SetAngle(DirectX::XMFLOAT3 meshAngle) noexcept { angle = meshAngle; }
	};

	template<typename T>
	const Resource::IndexBuffer * ObjectBase<T>::indexBuffer = nullptr;

	template<typename T>
	std::vector<std::unique_ptr<Resource::IBindable>> ObjectBase<T>::staticBinds;
}
