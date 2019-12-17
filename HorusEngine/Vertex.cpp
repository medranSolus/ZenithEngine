#include "Vertex.h"
#include <utility>
#include <cstring>

namespace GFX::BasicType
{
	Vertex::Vertex(char * data, const VertexLayout& layout) noexcept(!IS_DEBUG) : data(data), layout(layout)
	{
		assert(data != nullptr);
	}
}
