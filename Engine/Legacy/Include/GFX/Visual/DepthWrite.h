#pragma once
#include "IVisual.h"

namespace ZE::GFX::Visual
{
	class DepthWrite : public IVisual
	{
	public:
		DepthWrite(Graphics& gfx, std::shared_ptr<Data::VertexLayout> vertexLayout);
		DepthWrite(DepthWrite&&) = default;
		DepthWrite(const DepthWrite&) = default;
		DepthWrite& operator=(DepthWrite&&) = default;
		DepthWrite& operator=(const DepthWrite&) = default;
		virtual ~DepthWrite() = default;
	};
}