#ifndef __EST_AUDIO_H
#define __EST_AUDIO_H

#include "EstTypes.h"

#if __cplusplus
extern "C" {
#endif

// Initialize the audio system
// Params:
// sampleRate - The sample rate of the audio system
// flags - The device flags
// Returns:
// EST_OK - The audio system was initialized successfully
// EST_OUT_OF_MEMORY - The audio system failed to initialize due to lack of memory
// EST_INVALID_ARGUMENT - The audio system failed to initialize due to invalid arguments
// EST_INVALID_STATE - The audio system failed to initialize due to invalid state (Already initialized)
// EST_INVALID_OPERATION - The audio system failed to initialize due to backend issues
EST_API enum EST_RESULT EST_DeviceInit(int sampleRate, enum EST_DEVICE_FLAGS flags);

EST_API enum EST_RESULT EST_GetInfo(est_device_info *info);

// Shutdown the audio system
// Returns:
// EST_OK - The audio system was shutdown successfully
// EST_INVALID_STATE - The audio system failed to shutdown due to invalid state (Not initialized)
EST_API enum EST_RESULT EST_DeviceFree();

// Get the sample rate of the audio system
// Params:
// path - The path to the audio file
// handle - The handle to the audio sample
// Returns:
// EST_OK - The sample was loaded successfully
// EST_OUT_OF_MEMORY - The sample failed to load due to lack of memory
// EST_INVALID_ARGUMENT - The sample failed to load due to invalid arguments
// EST_INVALID_STATE - The sample failed to load due to invalid state (Not initialized)
EST_API enum EST_RESULT EST_SampleLoad(const char *path, EHANDLE *handle);

// Get the sample rate of the audio system
// Params:
// data - The data of the audio file
// size - The size of the audio file
// handle - The handle to the audio sample
// Returns:
// EST_OK - The sample was loaded successfully
// EST_OUT_OF_MEMORY - The sample failed to load due to lack of memory
// EST_INVALID_ARGUMENT - The sample failed to load due to invalid arguments
// EST_INVALID_STATE - The sample failed to load due to invalid state (Not initialized)
EST_API enum EST_RESULT EST_SampleLoadMemory(const void *data, int size, EHANDLE *handle);

// Load a raw PCM audio sample
// Note: Must be in format 32-bit float, 2 channel interleaved
// Params:
// data - The data of the audio file
// pcmSize - The size of raw audio in pcm size
// channels - The number of channels of the audio file
// sampleRate - The sample rate of the audio file
// handle - The handle to the audio sample
// Returns:
// EST_OK - The sample was loaded successfully
// EST_OUT_OF_MEMORY - The sample failed to load due to lack of memory
// EST_INVALID_ARGUMENT - The sample failed to load due to invalid arguments
// EST_INVALID_STATE - The sample failed to load due to invalid state (Not initialized)
EST_API enum EST_RESULT EST_SampleLoadRawPCM(const void *data, int pcmSize, int channels, int sampleRate, EHANDLE *handle);

// Unload the audio sample
// Params:
// handle - The handle to the audio sample
// Returns:
// EST_OK - The sample was unloaded successfully
// EST_INVALID_ARGUMENT - The sample failed to unload due to invalid arguments
// EST_INVALID_STATE - The sample failed to unload due to invalid state (Not initialized)
EST_API enum EST_RESULT EST_SampleFree(EHANDLE handle);

// Play the audio sample
// This also resets the sample to the beginning
// Params:
// handle - The handle to the audio sample
// Returns:
// EST_OK - The sample was played successfully
// EST_INVALID_ARGUMENT - The sample failed to play due to invalid arguments
// EST_INVALID_STATE - The sample failed to play due to invalid state (Not initialized)
EST_API enum EST_RESULT EST_SamplePlay(EHANDLE handle);

// Stop the audio sample
// Params:
// handle - The handle to the audio sample
// Returns:
// EST_OK - The sample was played successfully
// EST_INVALID_ARGUMENT - The sample failed to play due to invalid arguments
// EST_INVALID_STATE - The sample failed to play due to invalid state (Not initialized)
EST_API enum EST_RESULT EST_SampleStop(EHANDLE handle);

// Get the status of the audio sample
// Params:
// handle - The handle to the audio sample
// value - The status of the audio sample [see EST_STATUS]
// Returns:
// EST_OK - The sample status was retrieved successfully
// EST_INVALID_ARGUMENT - The sample failed to play due to invalid arguments
// EST_INVALID_STATE - The sample failed to play due to invalid state (Not initialized)
EST_API enum EST_RESULT EST_SampleGetStatus(EHANDLE handle, enum EST_STATUS *value);

// Set the attribute of the audio sample
// Params:
// handle - The handle to the audio sample
// attribute - The attribute to set [see EST_ATTRIBUTE]
// value - The value to set the attribute to
// Returns:
// EST_OK - The sample attribute was set successfully
// EST_INVALID_ARGUMENT - The sample failed to set the attribute due to invalid arguments
// EST_INVALID_STATE - The sample failed to play due to invalid state (Not initialized)
EST_API enum EST_RESULT EST_SampleSetAttribute(EHANDLE handle, enum EST_ATTRIBUTE_FLAGS attribute, float value);

// Get the attribute of the audio sample
// Params:
// handle - The handle to the audio sample
// attribute - The attribute to set [see EST_ATTRIBUTE]
// value - The value to set the attribute to
// Returns:
// EST_OK - The sample attribute was set successfully
// EST_INVALID_ARGUMENT - The sample failed to set the attribute due to invalid arguments
// EST_INVALID_STATE - The sample failed to play due to invalid state (Not initialized)
EST_API enum EST_RESULT EST_SampleGetAttribute(EHANDLE handle, enum EST_ATTRIBUTE_FLAGS attribute, float *value);

// Slide the attribute over the time
// Note: This is thread blocking function
// Params:
// handle - The handle to the audio sample
// attribute - The attribute to set [see EST_ATTRIBUTE]
// value - The value to set the attribute to
// time - The time to how long the attribute slide it, 0 meaning instant
// Returns:
// EST_OK - The sample attribute was set successfully
// EST_INVALID_ARGUMENT - The sample failed to set the attribute due to invalid arguments
// EST_INVALID_STATE - The sample failed to play due to invalid state (Not initialized)
EST_API enum EST_RESULT EST_SampleSlideAttribute(EHANDLE handle, enum EST_ATTRIBUTE_FLAGS attribute, float value, float time);

// Slide the attribute over the time
// Note: This is async version (aka non-blocking) function
// Params:
// handle - The handle to the audio sample
// attribute - The attribute to set [see EST_ATTRIBUTE]
// value - The value to set the attribute to
// time - The time to how long the attribute slide it, 0 meaning instant
// Returns:
// EST_OK - The sample attribute was set successfully
// EST_INVALID_ARGUMENT - The sample failed to set the attribute due to invalid arguments
// EST_INVALID_STATE - The sample failed to play due to invalid state (Not initialized)
EST_API enum EST_RESULT EST_SampleSlideAttributeAsync(EHANDLE handle, enum EST_ATTRIBUTE_FLAGS attribute, float value, float time);

EST_API enum EST_RESULT EST_SampleSetCallback(EHANDLE handle, est_audio_callback callback, void *userdata);
EST_API enum EST_RESULT EST_SampleSetGlobalCallback(est_audio_callback callback, void *userdata);

// Get error message whatever the API function return != ES_OK
// Returns:
// The error message
// or
// null if no error
EST_API const char *EST_GetError();

// INTERNAL, set error message
// Params:
// error - The error message
EST_API void EST_SetError(const char *error);

#if __cplusplus
}
#endif

#endif