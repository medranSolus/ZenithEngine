#pragma once
#include <DirectXMath.h>

namespace GFX::Primitive
{
	class Color
	{
	public:
		float r = 0.0f;
		float g = 0.0f;
		float b = 0.0f;
		float a = 1.0f;

		Color() {}
		Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
		Color(const Color & c) : r(c.r), g(c.g), b(c.b), a(c.a) {}
		Color & operator=(const Color & c);

		Color operator+(const Color & c) const;
	};

	class Vertex
	{
	public:
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;

		Vertex(const DirectX::XMFLOAT3 & pos = { 0.0f,0.0f,0.0f }) : pos(pos) {}
		Vertex(const Vertex & vertex) : pos(vertex.pos) {}
		Vertex & operator=(const Vertex & vertex);

		inline Vertex operator+(const Vertex & vertex) const { return { { pos.x + vertex.pos.x, pos.y + vertex.pos.y, pos.z + vertex.pos.z } }; }
		inline float operator()() const { return sqrtf(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z); }

		Vertex & operator*=(float scale);
		Vertex & operator/=(float scale);
	};

	class VertexColor : public Vertex
	{
	public:
		Color col;

		VertexColor() {}
		VertexColor(const DirectX::XMFLOAT3 & pos, const Color & col) : Vertex(pos), col(col) {}
		VertexColor(const Vertex & vertex, const Color & col) : Vertex(vertex), col(col) {}
		VertexColor(const VertexColor & vertex) : Vertex(vertex.pos), col(vertex.col) {}
		VertexColor & operator=(const VertexColor & vertex);

		VertexColor operator+(const VertexColor & vertex) const;
	};

	class VertexTexture : public Vertex
	{
	public:
		float u = 0.0f;
		float v = 0.0f;

		VertexTexture() {}
		VertexTexture(const DirectX::XMFLOAT3 & pos, float u, float v) : Vertex(pos), u(u), v(v) {}
		VertexTexture(const Vertex & vertex, float u, float v) : Vertex(vertex), u(u), v(v) {}
		VertexTexture(const VertexTexture & vertex) : Vertex(vertex.pos), u(vertex.u), v(vertex.v) {}
		VertexTexture & operator=(const VertexTexture & vertex);

		VertexTexture operator+(const VertexTexture & vertex) const;
	};
}