#pragma once

namespace ZE::GFX::Pipeline::RenderList
{
	class Lambertian final
	{
	public:
		Lambertian() {}
		Lambertian(Lambertian&&) = delete;
		Lambertian(const Lambertian&) = delete;
		Lambertian& operator=(Lambertian&&) = delete;
		Lambertian& operator=(const Lambertian&) = delete;
		~Lambertian() {}
	};
}