// Macros for specifying various buffers in API agnostic way
// (note that all constants have to be grouped together into single data type for single shader)
#ifndef BUFFERS_HLSLI
#define BUFFERS_HLSLI

// API agnostic macro for accessed buffers
// All names are given corresponding prefixes:
// - Constant: ct_
// - CBuffer: cb_
// - Texture: tx_
// - UAV: ua_
#ifdef _DX11
#	define CONSTANT_EX(name, dataType, slot, spaceSlot) cbuffer dataType##Constant : register(b##slot) { dataType ct_##name; }
#	define CBUFFER_EX(name, dataType, slot, rangeSlot, spaceSlot) cbuffer dataType##Buffer : register(b##slot) { dataType cb_##name; }
#	define TEXTURE_EX(name, texType, slot, rangeSlot) texType tx_##name : register(t##slot)
#	define UAV_EX(name, uavType, slot, rangeSlot) uavType ua_##name : register(u##slot)
#elif defined(_DX12)
#	define CONSTANT_EX(name, dataType, slot, spaceSlot) cbuffer dataType##Constant : register(b##slot, space##spaceSlot) { dataType ct_##name; }
#	define CBUFFER_EX(name, dataType, slot, rangeSlot, spaceSlot) cbuffer dataType##Buffer : register(b##slot, space##spaceSlot) { dataType cb_##name; }
#	define TEXTURE_EX(name, texType, slot, rangeSlot) texType tx_##name : register(t##slot)
#	define UAV_EX(name, uavType, slot, rangeSlot) uavType ua_##name : register(u##slot)
#elif defined(_VK)
#	define CONSTANT_EX(name, dataType, slot, spaceSlot) [[vk::push_constant]] dataType ct_##name
#	define CBUFFER_EX(name, dataType, slot, rangeSlot, spaceSlot) cbuffer dataType##Buffer : register(b##slot, space##rangeSlot) { dataType cb_##name; }
#	define TEXTURE_EX(name, texType, slot, rangeSlot) texType tx_##name : register(t##slot, space##rangeSlot)
#	define UAV_EX(name, uavType, slot, rangeSlot) uavType ua_##name : register(u##slot, space##rangeSlot)
#else
#	error Wrong type of graphics API used!
#endif

// Global CBuffer accessible from multiple shader types
#define CBUFFER_GLOBAL(name, dataType, slot, rangeSlot) CBUFFER_EX(name, dataType, slot, rangeSlot, 0)
// Global Constant accessible from multiple shader types
#define CONSTANT_GLOBAL(name, dataType, slot) CONSTANT_EX(name, dataType, slot, 0)

// Typical 2D texture
#define TEX2D(name, slot, rangeSlot) TEXTURE_EX(name, Texture2D, slot, rangeSlot)
// Typical 2D UAV
#define UAV2D(name, dataType, slot, rangeSlot) UAV_EX(name, RWTexture2D<dataType>, slot, rangeSlot)

// Shader agnostic macro for Constant and CBuffer
#if defined(_PS) || defined(_CS)
#	define CONSTANT(name, dataType, slot) CONSTANT_EX(name, dataType, slot, 0)
#	define CBUFFER(name, dataType, slot, rangeSlot) CBUFFER_EX(name, dataType, slot, rangeSlot, 0)
#elif defined _VS
#	define CONSTANT(name, dataType, slot) CONSTANT_EX(name, dataType, slot, 1)
#	define CBUFFER(name, dataType, slot, rangeSlot) CBUFFER_EX(name, dataType, slot, rangeSlot, 1)
#elif defined _GS
#	define CONSTANT(name, dataType, slot) CONSTANT_EX(name, dataType, slot, 2)
#	define CBUFFER(name, dataType, slot, rangeSlot) CBUFFER_EX(name, dataType, slot, rangeSlot, 2)
#elif defined _DS
#	define CONSTANT(name, dataType, slot) CONSTANT_EX(name, dataType, slot, 3)
#	define CBUFFER(name, dataType, slot, rangeSlot) CBUFFER_EX(name, dataType, slot, rangeSlot, 3)
#elif defined _HS
#	define CONSTANT(name, dataType, slot) CONSTANT_EX(name, dataType, slot, 4)
#	define CBUFFER(name, dataType, slot, rangeSlot) CBUFFER_EX(name, dataType, slot, rangeSlot, 4)
#else
#	error Wrong type of shader used!
#endif

#endif // BUFFERS_HLSLI