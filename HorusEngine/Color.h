#pragma once
#include <DirectXMath.h>
#include <utility>

namespace GFX::Data
{
	class ColorByte
	{
	public:
		unsigned char r = 0;
		unsigned char g = 0;
		unsigned char b = 0;
		unsigned char a = 255;

		ColorByte() = default;
		constexpr ColorByte(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) noexcept : r(r), g(g), b(b), a(a) {}
		constexpr ColorByte(const ColorByte& c) noexcept : r(c.r), g(c.g), b(c.b), a(c.a) {}
		constexpr ColorByte& operator=(const ColorByte& c) noexcept;
		~ColorByte() = default;

		constexpr bool operator==(const ColorByte& c) const noexcept { return c.r == r && c.g == g && c.b == b && c.a == a; }
		constexpr bool operator!=(const ColorByte& c) const noexcept { return c.r != r || c.g != g || c.b != b || c.a != a; }
		constexpr ColorByte operator-() const { return *this * -1.0f; }

		constexpr ColorByte operator+(const ColorByte& c) const noexcept;
		constexpr ColorByte operator*(float x) const noexcept;
	};

	class ColorFloat3
	{
	public:
		DirectX::XMFLOAT3 col = { 0.0f,0.0f,0.0f };

		ColorFloat3() = default;
		constexpr ColorFloat3(float r, float g, float b) noexcept : col(r, g, b) {}

		constexpr ColorFloat3(const DirectX::XMFLOAT3& col) noexcept : col(col) {}
		constexpr ColorFloat3(DirectX::XMFLOAT3&& col) noexcept : col(std::move(col)) {}

		constexpr ColorFloat3(const ColorFloat3& c) noexcept : col(c.col) {}
		constexpr ColorFloat3(ColorFloat3&& c) noexcept : col(std::move(c.col)) {}

		constexpr ColorFloat3(const ColorByte& c) noexcept : col(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f) {}

		constexpr ColorFloat3& operator=(const ColorFloat3& c) noexcept;
		constexpr ColorFloat3& operator=(ColorFloat3&& c) noexcept;

		inline operator DirectX::XMFLOAT3() const noexcept { return col; }

		ColorFloat3& operator=(const ColorByte& c) noexcept;
		~ColorFloat3() = default;

		inline bool operator==(const ColorFloat3& c) const { return DirectX::XMVector3Equal(DirectX::XMLoadFloat3(&c.col), DirectX::XMLoadFloat3(&col)); }
		inline bool operator!=(const ColorFloat3& c) const { return DirectX::XMVector3NotEqual(DirectX::XMLoadFloat3(&c.col), DirectX::XMLoadFloat3(&col)); }
		inline ColorFloat3 operator-() const { return *this * -1.0f; }

		ColorFloat3 operator+(const ColorFloat3& c) const;
		ColorFloat3 operator*(float x) const;
	};

	class ColorFloat4
	{
	public:
		DirectX::XMFLOAT4 col = { 0.0f,0.0f,0.0f,1.0f };

		ColorFloat4() = default;
		constexpr ColorFloat4(float r, float g, float b, float a = 1.0f) noexcept : col(r, g, b, a) {}

		constexpr ColorFloat4(const DirectX::XMFLOAT4& col) noexcept : col(col) {}
		constexpr ColorFloat4(DirectX::XMFLOAT4&& col) noexcept : col(std::move(col)) {}
		constexpr ColorFloat4(const DirectX::XMFLOAT3& col) noexcept : col(col.x, col.y, col.z, 1.0f) {}

		constexpr ColorFloat4(const ColorFloat4& c) noexcept : col(c.col) {}
		constexpr ColorFloat4(ColorFloat4&& c) noexcept : col(std::move(c.col)) {}

		constexpr ColorFloat4(const ColorByte& c) noexcept : col(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f) {}
		constexpr ColorFloat4(const ColorFloat3& col) noexcept : col(col.col.x, col.col.y, col.col.z, 1.0f) {}

		constexpr ColorFloat4& operator=(const ColorFloat4& c) noexcept;
		constexpr ColorFloat4& operator=(ColorFloat4&& c) noexcept;

		inline operator DirectX::XMFLOAT4() const noexcept { return col; }

		ColorFloat4& operator=(const ColorByte& c) noexcept;
		ColorFloat4& operator=(const ColorFloat3& c) noexcept;
		~ColorFloat4() = default;

		inline bool operator==(const ColorFloat4& c) const { return DirectX::XMVector4Equal(DirectX::XMLoadFloat4(&c.col), DirectX::XMLoadFloat4(&col)); }
		inline bool operator!=(const ColorFloat4& c) const { return DirectX::XMVector4NotEqual(DirectX::XMLoadFloat4(&c.col), DirectX::XMLoadFloat4(&col)); }
		inline ColorFloat4 operator-() const { return *this * -1.0f; }

		ColorFloat4 operator+(const ColorFloat4& c) const;
		ColorFloat4 operator*(float x) const;
	};

	constexpr ColorByte& ColorByte::operator=(const ColorByte& c) noexcept
	{
		r = c.r;
		g = c.g;
		b = c.b;
		a = c.a;
		return *this;
	}

	constexpr ColorByte ColorByte::operator+(const ColorByte& c) const noexcept
	{
		return { static_cast<unsigned char>((r + c.r) >> 1),
			static_cast<unsigned char>((g + c.g) >> 1),
			static_cast<unsigned char>((b + c.b) >> 1),
			static_cast<unsigned char>((a + c.a) >> 1) };
	}

	constexpr ColorByte ColorByte::operator*(float x) const noexcept
	{
		return { static_cast<unsigned char>(r * x),
			static_cast<unsigned char>(g * x),
			static_cast<unsigned char>(b * x), a };
	}

	constexpr ColorFloat3& ColorFloat3::operator=(const ColorFloat3& c) noexcept
	{
		col = c.col;
		return *this;
	}

	constexpr ColorFloat3& ColorFloat3::operator=(ColorFloat3&& c) noexcept
	{
		col = std::move(c.col);
		return *this;
	}

	constexpr ColorFloat4& ColorFloat4::operator=(const ColorFloat4& c) noexcept
	{
		col = c.col;
		return *this;
	}

	constexpr ColorFloat4& ColorFloat4::operator=(ColorFloat4&& c) noexcept
	{
		col = std::move(c.col);
		return *this;
	}
}