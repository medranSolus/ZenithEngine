#pragma once
#include "OAL.h"

/* Definitions of extensions (up to OpenAL SOFT version 1.25.2)
* 
* Every listed extension here have tags attached to it, which will help
* in searching for possible use case for that extension.
* Possible extension tags:
*
* EXT_DEBUG  - debugging and other tools
* EXT_FORMAT - new format support
* EXT_3D     - spatial audio support
* 
* For list of current extensions go to: https://openal-soft.org/openal-extensions/
*/
#pragma region AL extensions
// List of required AL extensions, intended for use in X() macro
#define ZE_AL_EXT_LIST_REQUIRED \
	X(AL_EXT_float32)		/* Support for 32-bit floating-point audio data [EXT_FORMAT] */ \
	X(AL_EXT_STATIC_BUFFER)	/* Allow for referencing audio data outside the scope of OpenAL-managed buffers */ \
	X(AL_SOFT_events)		/* Allow for event-based source audio control */

// List of AL extensions, used in debug targets, intended for use in X() macro
#define ZE_AL_EXT_LIST_DEBUG \
	X(AL_EXT_debug) /* Debugging and error reporting [EXT_DEBUG] */

// List of optional AL extensions, intended for use in X() macro
#define ZE_AL_EXT_LIST_OPTIONAL \
	X(AL_EXT_double)					/* Support for 64-bit floating-point audio data [EXT_FORMAT] */ \
	X(AL_EXT_MCFORMATS)					/* Support for multi-channel audio data [EXT_FORMAT] */ \
	X(AL_SOFT_direct_channels)			/* Bypass 3D sound and route audio directly to the speakers (creating standard 2D audio) [EXT_3D] */ \
	X(AL_SOFT_direct_channels_remix)	/* Extend functionality of AL_SOFT_direct_channels and allow for remixing into actual set of speakers [EXT_3D] */ \
	X(AL_SOFT_source_spatialize)		/* Manual specification whether given source should be spatialized [EXT_3D] */

// List of currently not used AL extensions, intended for use in X() macro
#define ZE_AL_EXT_LIST_NOT_USED \
	X(AL_EXT_ALAW)					/* Support for a-law encoded audio data (mostly telcom) [EXT_FORMAT] */ \
	X(AL_EXT_BFORMAT)				/* Support for B-Format ambisonics audio data [EXT_FORMAT] [EXT_3D] */ \
	X(AL_EXT_direct_context)		/* Instead of using implicit current context for all functions, use direct context pointer in each call */ \
	X(AL_EXT_IMA4)					/* Support IMA4 ADPCM audio data [EXT_FORMAT] */ \
	X(AL_EXT_MULAW)					/* Support for u-law encoded audio data (mostly telcom) [EXT_FORMAT] */ \
	X(AL_EXT_MULAW_BFORMAT)			/* Support for u-law B-Format ambisonics audio data [EXT_FORMAT] [EXT_3D] */ \
	X(AL_EXT_MULAW_MCFORMATS)		/* Support for u-law encoded multi channel-audio data (mostly telcom) [EXT_FORMAT] */ \
	X(AL_EXT_source_distance_model)	/* Allows for custom distance curves for different sources [EXT_3D] */ \
	X(AL_EXT_SOURCE_RADIUS)			/* Turn sound sources into objects instead of space points [EXT_3D] */ \
	X(AL_EXT_STEREO_ANGLES)			/* Manually specify angles of the stereo speakers [EXT_3D] */ \
	X(AL_SOFT_bformat_hoa)			/* Support for High-Order Ambisonics audio data [EXT_FORMAT] [EXT_3D] */ \
	X(AL_SOFT_block_alignment)		/* Specify alignment requirements for compressed audio buffers */ \
	X(AL_SOFT_buffer_length_query)	/* Same as AL_SOFT_source_length */ \
	X(AL_SOFT_callback_buffer)		/* Register callbacks for streaming audio data */ \
	X(AL_SOFT_deferred_updates)		/* Perform updates to the multiple sound params in single step */ \
	X(AL_SOFT_effect_target)		/* Allows chaining effects with each other */ \
	X(AL_SOFT_gain_clamp_ex)		/* Custom gain limit */ \
	X(AL_SOFT_loop_points)			/* Allows for looping mid-buffer rather than whole submitted audio data */ \
	X(AL_SOFT_MSADPCM)				/* Support for Microsoft ADPCM format [EXT_FORMAT] */ \
	X(AL_SOFT_source_latency)		/* Get exact latency of audio data being played, useful for sync with animation */ \
	X(AL_SOFT_source_length)		/* Query the time duration of the sound buffer */ \
	X(AL_SOFT_source_resampler)		/* Change resamplers for given sources when converting between bitrates */ \
	X(AL_SOFT_source_start_delay)	/* Play audio with a delay into the future */ \
	X(AL_SOFT_UHJ)					/* New UHJ audio formats from B-Formats [EXT_FORMAT] [EXT_3D] */ \
	X(AL_SOFT_UHJ_ex)				/* UHJ formats extended for u-law/a-law/IMA4/MS ADPCM formats [EXT_FORMAT] [EXT_3D] */

// List of AL extensions, intended for use in X() macro
#define ZE_AL_EXT_LIST ZE_AL_EXT_LIST_REQUIRED ZE_AL_EXT_LIST_DEBUG ZE_AL_EXT_LIST_OPTIONAL
#pragma endregion

#pragma region ALC extensions
// List of required ALC extensions, intended for use in X() macro
#define ZE_ALC_EXT_LIST_REQUIRED \
	X(ALC_EXT_EFX) /* Various effects for 3D sound */

// List of ALC extensions, used in debug targets, intended for use in X() macro
#define ZE_ALC_EXT_LIST_DEBUG \
	X(ALC_EXT_debug) /* Debugging and error reporting [EXT_DEBUG] */

// List of optional ALC extensions, intended for use in X() macro
#define ZE_ALC_EXT_LIST_OPTIONAL \
	X(ALC_SOFT_output_limiter) /* Limit peaks of amplitude to not create distortion in 3D sound */

// List of currently not used ALC extensions, intended for use in X() macro
#define ZE_ALC_EXT_LIST_NOT_USED \
	X(ALC_EXT_DEDICATED)			/* Allows for routing spatial sound directly to dedicated speakers [EXT_3D] */ \
	X(ALC_EXT_disconnect)			/* To check if the device got disconnected */ \
	X(ALC_EXT_thread_local_context) /* Manage current context on per-thread level */ \
	X(ALC_SOFT_device_clock)		/* Query the timing clock from the audio device */ \
	X(ALC_SOFT_HRTF)				/* Manual management over HRTF files [EXT_3D] */ \
	X(ALC_SOFT_loopback)			/* Re-route sound to the buffer instead of sound card */ \
	X(ALC_SOFT_loopback_bformat)	/* Allows for B-Formats during loopback capturing */ \
	X(ALC_SOFT_output_mode)			/* Use specific output mode with the given device */ \
	X(ALC_SOFT_pause_device)		/* Allows for pausing playback of entire device */ \
	X(ALC_SOFT_reopen_device)		/* Re-route output to the another device after it's being disconnected */ \
	X(ALC_SOFT_system_events)		/* Detect changes to the device */

// List of ALC extensions, intended for use in X() macro
#define ZE_ALC_EXT_LIST ZE_ALC_EXT_LIST_REQUIRED ZE_ALC_EXT_LIST_DEBUG ZE_ALC_EXT_LIST_OPTIONAL
#pragma endregion

// List of OpenAL extensions, intended for use in X() macro
#define ZE_OAL_EXT_LIST ZE_AL_EXT_LIST ZE_ALC_EXT_LIST

#pragma region Deprecated and not supported extensions
// List of deprecated OpenAL extensions, purely informational to keep track of known extensions, intended for use in X() macro
#define ZE_OAL_EXT_LIST_DEPRECATED \
	X(AL_EXT_FOLDBACK)			/* Replaced by ALC_SOFT_loopback */ \
	X(AL_EXT_vorbis)			/* Support for passing Vorbis encoded stream instead of PCM data [EXT_FORMAT] */ \
	X(AL_LOKI_IMA_ADPCM_format)	/* Support for legacy IMA ADPCM Loki Software format [EXT_FORMAT] */ \
	X(AL_LOKI_quadriphonic)		/* Superseded by AL_EXT_MCFORMATS [EXT_FORMAT] */ \
	X(AL_LOKI_WAVE_format)		/* Support for passing directly WAVE format instead of PCM data [EXT_FORMAT] */ \
	X(AL_SOFT_buffer_samples)	/* Rewrites design for buffer data format and allocation [EXT_FORMAT] */ \
	X(AL_SOFT_buffer_sub_data)	/* Legacy extension for updating only parts of audio buffer data */ \
	X(ALC_LOKI_audio_channel)	/* Manual selection of hardware channel attributes */

// List of not supported OpenAL extensions, that won't be added without clear reason, intended for use in X() macro
#define ZE_OAL_EXT_LIST_NOT_SUPPORTED \
	X(ALC_EXT_DEFAULT_FILTER_ORDER)	/* Specify order of filters to be applied by default (OpenAL Soft does not implement this extension at all) */ \
	ZE_OAL_EXT_LIST_DEPRECATED
#pragma endregion