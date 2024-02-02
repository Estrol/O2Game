#ifndef __EST_AUDIO_ENCODER_H
#define __EST_AUDIO_ENCODER_H

#include "EstTypes.h"

#if __cplusplus
extern "C" {
#endif

// Load file as encoder channel
EST_API enum EST_RESULT EST_EncoderLoad(const char *path, est_encoder_callback callback, enum EST_DECODER_FLAGS flags, ECHANDLE *handle);

// Load audio file from memory as encoder channel
EST_API enum EST_RESULT EST_EncoderLoadMemory(const void *data, int size, est_encoder_callback callback, enum EST_DECODER_FLAGS flags, ECHANDLE *handle);

// Free the encoder channel
EST_API enum EST_RESULT EST_EncoderFree(ECHANDLE handle);

EST_API enum EST_RESULT EST_EncoderGetInfo(ECHANDLE handle, est_encoder_info *info);

// Render the encoder channel
// Note: You don't need use this channel if you using EST_GetSampleChannel
EST_API enum EST_RESULT EST_EncoderRender(ECHANDLE handle);

// Set the encoder channel attribute
EST_API enum EST_RESULT EST_EncoderSetAttribute(ECHANDLE handle, enum EST_ATTRIBUTE_FLAGS attribute, float value);

// Get the encoder channel attribute
EST_API enum EST_RESULT EST_EncoderGetAttribute(ECHANDLE handle, enum EST_ATTRIBUTE_FLAGS attribute, float *value);

// Get decoded data from encoder channel
// Note: Must use EST_EncoderRender first to get the decoded data
EST_API enum EST_RESULT EST_EncoderGetData(ECHANDLE handle, void *data, int *size);

// Flush the encoder channel
EST_API enum EST_RESULT EST_EncoderFlushData(ECHANDLE handle);

// Get available decoded PCM data length
// Note: Must use EST_EncoderRender first to get the decoded data
EST_API enum EST_RESULT EST_EncoderGetAvailableDataSize(ECHANDLE handle, int *size);

// Convert encoder channel to Sample channel
// Note: No need to use EST_EncoderRender
// Note2: You need free the out sample after you done used it.
EST_API enum EST_RESULT EST_EncoderGetSample(ECHANDLE handle, EHANDLE *outSample);

// Export the encoder channel to file
// Note: Need to call EST_EncoderRender first
EST_API enum EST_RESULT EST_EncoderExportFile(ECHANDLE handle, enum EST_FILE_EXPORT type, char *filePath);

#if __cplusplus
}
#endif

#endif