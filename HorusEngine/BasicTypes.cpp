#include "BasicTypes.h"
#include <utility>
#include <cstring>

namespace GFX::Primitive
{
	Color & Color::operator=(const Color & c)
	{
		r = c.r; 
		g = c.g; 
		b = c.b; 
		a = c.a;
		return *this;
	}

	Color Color::operator+(const Color & c) const
	{
		return { (r + c.r) / 2.0f, (g + c.g) / 2.0f, (b + c.b) / 2.0f, (a + c.a) / 2.0f };
	}

	Vertex & Vertex::operator=(const Vertex & vertex)
	{
		pos = vertex.pos;
		return *this;
	}

	Vertex & Vertex::operator*=(float scale)
	{
		pos.x *= scale;
		pos.y *= scale;
		pos.z *= scale;
		return *this;
	}

	Vertex & Vertex::operator/=(float scale)
	{
		pos.x /= scale;
		pos.y /= scale;
		pos.z /= scale;
		return *this;
	}

	VertexColor & VertexColor::operator=(const VertexColor & vertex)
	{
		pos = vertex.pos;
		col = vertex.col;
		return *this;
	}

	VertexColor VertexColor::operator+(const VertexColor & vertex) const
	{
		return { this->Vertex::operator+(vertex), col };
	}

	VertexTexture & VertexTexture::operator=(const VertexTexture & vertex)
	{
		pos = vertex.pos;
		u = vertex.u;
		v = vertex.v;
		return *this;
	}

	VertexTexture VertexTexture::operator+(const VertexTexture & vertex) const
	{
		return { this->Vertex::operator+(vertex), u, v };
	}
}
