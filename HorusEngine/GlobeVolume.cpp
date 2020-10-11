#include "GlobeVolume.h"
#include "Primitives.h"

namespace GFX::Light::Volume
{
	GlobeVolume::GlobeVolume(Graphics& gfx, unsigned int density)
		: IVolume(gfx)
	{
		std::string typeName = Primitive::Sphere::GetNameIcoSolid(density);
		if (Resource::VertexBuffer::NotStored(typeName) && Resource::IndexBuffer::NotStored(typeName))
		{
			auto list = Primitive::Sphere::MakeIcoSolid(density);
			vertexBuffer = Resource::VertexBuffer::Get(gfx, typeName, list.vertices);
			indexBuffer = Resource::IndexBuffer::Get(gfx, typeName, list.indices);
		}
		else
		{
			Primitive::IndexedTriangleList list;
			vertexBuffer = Resource::VertexBuffer::Get(gfx, typeName, list.vertices);
			indexBuffer = Resource::IndexBuffer::Get(gfx, typeName, list.indices);
		}
		pixelShader = Resource::PixelShader::Get(gfx, "PointLightPS");
	}
}