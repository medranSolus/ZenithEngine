#pragma once

namespace ZE::Data
{
	// Component describing present meshes inside single entity
	struct Model
	{
		U64 MeshCount;
		Ptr<U64> MeshIDs;
	};
}