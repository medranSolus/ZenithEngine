// Macros for specifying constant buffers, shader constants adn textures in API agnostic way
// (note that all constants have to be grouped together into single data type for single shader)
#ifndef BUFFERS_HLSLI
#define BUFFERS_HLSLI

// API agnostic macro for CBuffer, Constant and Texture
// All names are given corresponding prefixes:
// - CBuffer: cb_
// - Constant: ct_
// - Texture: tx_
#ifdef _DX11
#	define CBUFFER_EX(name, dataType, slot, rangeSlot, spaceSlot) cbuffer dataType##Buffer : register(b##slot) { dataType cb_##name; }
#	define CONSTANT_EX(name, dataType, slot, rangeSlot, spaceSlot) cbuffer dataType##Constant : register(b##slot) { dataType ct_##name; }
#	define TEXTURE_EX(name, texType, slot, rangeSlot) texType tx_##name : register(t##slot)
#elif defined(_DX12)
#	define CBUFFER_EX(name, dataType, slot, rangeSlot, spaceSlot) cbuffer dataType##Buffer : register(b##slot, space##spaceSlot) { dataType cb_##name; }
#	define CONSTANT_EX(name, dataType, slot, rangeSlot, spaceSlot) cbuffer dataType##Constant : register(b##slot, space##spaceSlot) { dataType ct_##name; }
#	define TEXTURE_EX(name, texType, slot, rangeSlot) texType tx_##name : register(t##slot)
#elif defined(_VK)
#	define CBUFFER_EX(name, dataType, slot, rangeSlot, spaceSlot) cbuffer dataType##Buffer : register(b##slot, space##rangeSlot) { dataType cb_##name; }
#	define CONSTANT_EX(name, dataType, slot, rangeSlot, spaceSlot) cbuffer dataType##Constant: register(b##slot, space##rangeSlot)  { dataType ct_##name; }
#	define TEXTURE_EX(name, texType, slot, rangeSlot) texType tx_##name : register(t##slot, space##rangeSlot)
#else
#	error Wrong type of graphics API used!
#endif

// Global CBuffer accessible from multiple shader types
#define CBUFFER_GLOBAL(name, dataType, slot) CBUFFER_EX(name, dataType, slot, 0, 0)
// Global Constant accessible from multiple shader types
#define CONSTANT_GLOBAL(name, dataType, slot) CONSTANT_EX(name, dataType, slot, 0, 0)

// Typical 2D texture
#define TEX2D(name, slot) TEXTURE_EX(name, Texture2D, slot, 0)

// Shader agnostic macro for CBuffer and Constant
#if defined(_PS) || defined(_CS)
#	define CBUFFER(name, dataType, slot) CBUFFER_EX(name, dataType, slot, 0, 0)
#	define CONSTANT(name, dataType, slot) CONSTANT_EX(name, dataType, slot, 0, 0)
#elif defined _VS
#	define CBUFFER(name, dataType, slot) CBUFFER_EX(name, dataType, slot, 0, 1)
#	define CONSTANT(name, dataType, slot) CONSTANT_EX(name, dataType, slot, 0, 1)
#elif defined _GS
#	define CBUFFER(name, dataType, slot) CBUFFER_EX(name, dataType, slot, 0, 2)
#	define CONSTANT(name, dataType, slot) CONSTANT_EX(name, dataType, slot, 0, 2)
#elif defined _DS
#	define CBUFFER(name, dataType, slot) CBUFFER_EX(name, dataType, slot, 0, 3)
#	define CONSTANT(name, dataType, slot) CONSTANT_EX(name, dataType, slot, 0, 3)
#elif defined _HS
#	define CBUFFER(name, dataType, slot) CBUFFER_EX(name, dataType, slot, 0, 4)
#	define CONSTANT(name, dataType, slot) CONSTANT_EX(name, dataType, slot, 0, 4)
#else
#	error Wrong type of shader used!
#endif

#endif // BUFFERS_HLSLI