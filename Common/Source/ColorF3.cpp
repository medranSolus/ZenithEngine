#include "ColorF3.h"

ColorF3 ColorF3::operator-(const ColorF3& c) const noexcept
{
	ColorF3 nc;
	Math::XMStoreFloat3(&nc.RGB,
		Math::XMVectorSubtract(Math::XMLoadFloat3(&RGB), Math::XMLoadFloat3(&c.RGB)));
	return nc;
}

ColorF3 ColorF3::operator+(const ColorF3& c) const noexcept
{
	ColorF3 nc;
	Math::XMStoreFloat3(&nc.RGB,
		Math::XMVectorAdd(Math::XMLoadFloat3(&RGB), Math::XMLoadFloat3(&c.RGB)));
	return nc;
}

ColorF3 ColorF3::operator*(float x) const noexcept
{
	ColorF3 nc(x, x, x);
	Math::XMStoreFloat3(&nc.RGB,
		Math::XMVectorMultiply(Math::XMLoadFloat3(&RGB), Math::XMLoadFloat3(&nc.RGB)));
	return nc;
}