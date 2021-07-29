#pragma once

namespace ZE::GFX::Pipeline::RenderList
{
	class Outline final
	{
	public:
		Outline() {}
		Outline(Outline&&) = delete;
		Outline(const Outline&) = delete;
		Outline& operator=(Outline&&) = delete;
		Outline& operator=(const Outline&) = delete;
		~Outline() {}
	};
}