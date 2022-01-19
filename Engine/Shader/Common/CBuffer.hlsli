// Macros for specifying constant buffers

// API agnostic macro for CBuffer
#ifdef _DX11
#define CBUFFER_EX(name, dataType, slot, space_slot) cbuffer dataType##Buffer : register(b##slot) { dataType cb_##name; }
#elif defined _DX12
#define CBUFFER_EX(name, dataType, slot, space_slot) ConstantBuffer<dataType> cb_##name : register(b##slot, space##space_slot)
#else
#error Wrong type of graphics API used!
#endif

// Global CBuffer accessible from multiple shaders
#define CBUFFER_GLOBAL(name, dataType, slot) CBUFFER_EX(name, dataType, slot, 0)

// Shader agnostic macro for CBuffer
#if defined(_PS) || defined(_CS)
#define CBUFFER(name, dataType, slot) CBUFFER_EX(name, dataType, slot, 0)
#elif defined _VS
#define CBUFFER(name, dataType, slot) CBUFFER_EX(name, dataType, slot, 1)
#elif defined _GS
#define CBUFFER(name, dataType, slot) CBUFFER_EX(name, dataType, slot, 2)
#elif defined _DS
#define CBUFFER(name, dataType, slot) CBUFFER_EX(name, dataType, slot, 3)
#elif defined _HS
#define CBUFFER(name, dataType, slot) CBUFFER_EX(name, dataType, slot, 4)
#else
#error Wrong type of shader used!
#endif