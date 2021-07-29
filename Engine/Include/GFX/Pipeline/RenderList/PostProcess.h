#pragma once

namespace ZE::GFX::Pipeline::RenderList
{
	class PostProcess final
	{
	public:
		PostProcess() {}
		PostProcess(PostProcess&&) = delete;
		PostProcess(const PostProcess&) = delete;
		PostProcess& operator=(PostProcess&&) = delete;
		PostProcess& operator=(const PostProcess&) = delete;
		~PostProcess() {}
	};
}