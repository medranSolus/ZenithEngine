#include "Vertex.h"

namespace GFX::Data
{
	Vertex::Vertex(char* data, const VertexLayout& layout) noexcept(!IS_DEBUG)
		: data(data), layout(layout)
	{
		assert(data != nullptr);
	}
}