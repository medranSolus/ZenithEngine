#include "ColorF4.h"

ColorF4 ColorF4::operator-(const ColorF4& c) const
{
	ColorF4 nc;
	Math::XMStoreFloat4(&nc.RGBA,
		Math::XMVectorAdd(Math::XMLoadFloat4(&RGBA), Math::XMLoadFloat4(&c.RGBA)));
	return nc;
}

ColorF4 ColorF4::operator+(const ColorF4& c) const
{
	ColorF4 nc;
	Math::XMStoreFloat4(&nc.RGBA,
		Math::XMVectorAdd(Math::XMLoadFloat4(&RGBA), Math::XMLoadFloat4(&c.RGBA)));
	return nc;
}

ColorF4 ColorF4::operator*(float x) const
{
	ColorF4 nc(x, x, x, x);
	Math::XMStoreFloat4(&nc.RGBA,
		Math::XMVectorMultiply(Math::XMLoadFloat4(&RGBA), Math::XMLoadFloat4(&nc.RGBA)));
	return nc;
}