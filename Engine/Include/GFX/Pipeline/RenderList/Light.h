#pragma once

namespace ZE::GFX::Pipeline::RenderList
{
	class Light final
	{
	public:
		Light() {}
		Light(Light&&) = delete;
		Light(const Light&) = delete;
		Light& operator=(Light&&) = delete;
		Light& operator=(const Light&) = delete;
		~Light() {}
	};
}