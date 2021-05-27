/* 
 * Copyright (C) 2021 Marek Machliñski - All Rights Reserved.
 * 
 * This file is part of Zenith Engine project which is released under dual-license.
 * For more license details see file README.md or go to https://github.com/medranSolus/ZenithEngine.
 * 
 * Every class used in engine belongs to ZE namespace.
 */


 /*
 * Common types used by engine (note that this are the only types not under ZE namespace)
 *
 * [S|U][8|16|32|64]	- Simple Signed or Unsigned integer of specified size, ex. S16 = signed 16 bit int
 * Float[2|3|4]		- Float vector of specified size, ex. Float3 = 3 consecutive floats
 * Float[3x3|4x4]	- Square matrix of specified dimensions
 * Vector			- 16 byte aligned Float4 used in mathematical equations in DirectXMath library SSE methods
 * Matrix			- 16 byte aligned Float4x4 used in mathematical equations in DirectXMath library SSE methods
 */
#include "Types.h"

 /*
 * Color data
 *
 * Pixel	- 32 bit RGBA color
 * ColorF3	- Float3 RGB color
 * ColorF4	- Float4 RGBA color
 */
#include "Pixel.h"
#include "ColorF3.h"
#include "ColorF4.h"

 // Image data
#include "GFX/Surface.h"

// Basic utility functions
#include "Utils.h"

// Mathematical functions
#include "MathExt.h"

// Simple logger
#include "Logger.h"

// Performance profiler
#include "Perf.h"

/*
* Exceptions that are possible to throw by engine
*
* BasicException				- Base class for all exceptions
* WinApiException				- Exceptions related to WinAPI
* DirectXTexException			- Exceptions raised by calling DirectXTex methods
* ImageException				- Image processing exceptions
* GfxDebugException				- DirectX debug layer exceptions
* GraphicsException				- Generic exceptions caused by inappropriate state of graphics API
* DeviceRemovedException		- Exception raised when OS removed access to GPU, caused by driver or hardware failure
* ModelException				- Exceptions raised when loading 3D models
* RenderGraphCompileException	- Exceptions raised when errors are present in definition of render graph
*/
#include "Exception/Exceptions.h"

// Holds simple timer
#include "Timer.h"

/*
* Window management classes
* 
* Window	- Main window
* Mouse		- Accessing mouse events
* Keyboard	- Accessing keyboard events
*/
#include "WinAPI/Window.h"

// Displaying ImGui dialog windows
#include "GUI/DialogWindow.h"

/*
* Cameras to move across rendered scene
* 
* CameraPool		- Container for managing all cameras
* PersonCamera		- First person camera
* FloatingCamera	- Spaceship-like camera
*/
#include "Camera/Cameras.h"

// Graphics data used in processing
#include "GFX/Data/DataTypes.h"

// Light objects to iluminate the scene
#include "GFX/Light/Lights.h"

// Render passes used in engine deffered rendering
#include "GFX/Pipeline/RenderPass/RenderPasses.h"

// Resources used by render pipeline
#include "GFX/Pipeline/Resource/PipelineResources.h"

// Main engine render graph containing all current features
#include "GFX/Pipeline/MainPipelineGraph.h"

// Factory for creating techniques used in transfering shapes to pipeline
#include "GFX/Pipeline/TechniqueFactory.h"

// Creation of primitive geometry
#include "GFX/Primitive/Primitives.h"

// Graphics API resources
#include "GFX/Resource/GfxResources.h"

// Shapes displayed by the engine
#include "GFX/Shape/Shapes.h"

// Visual data used by pixel shaders
#include "GFX/Visual/Visuals.h"

// Main graphics controlling class
#include "GFX/Graphics.h"