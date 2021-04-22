#pragma once
#include "IObject.h"

namespace GFX
{
	class BasicObject : public IObject
	{
		static Data::CBuffer::DCBLayout MakeLayout() noexcept;

	protected:
		Data::CBuffer::DynamicCBuffer buffer;
		std::string name = "";

	public:
		BasicObject() noexcept : BasicObject({ 0.0f,0.0f,0.0f }, "") {}
		BasicObject(const Float3& position) noexcept : BasicObject(position, "") {}
		BasicObject(std::string&& name) noexcept : BasicObject({ 0.0f,0.0f,0.0f }, std::forward<std::string>(name)) {}
		BasicObject(const Float3& position, std::string&& name, float scale = 1.0f) noexcept;
		BasicObject(BasicObject&&) = default;
		BasicObject(const BasicObject&) = default;
		BasicObject& operator=(BasicObject&&) = default;
		BasicObject& operator=(const BasicObject&) = default;
		virtual ~BasicObject() = default;

		constexpr const std::string& GetName() const noexcept override { return name; }
		void SetName(const std::string& newName) noexcept override { name = newName; }

		const Float3& GetAngle() const noexcept override { return buffer["angle"]; }
		void SetAngle(const Float3& meshAngle) noexcept override { buffer["angle"] = meshAngle; }

		float GetScale() const noexcept override { return buffer["scale"]; }
		void SetScale(float newScale) noexcept override { buffer["scale"] = newScale; }

		const Float3& GetPos() const noexcept override { return buffer["position"]; }
		void SetPos(const Float3& position) noexcept override { buffer["position"] = position; }

		bool Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept override { return probe.VisitObject(buffer); }
		void UpdatePos(const Float3& delta) noexcept override;
		void UpdateAngle(const Float3& deltaAngle) noexcept override;
	};
}