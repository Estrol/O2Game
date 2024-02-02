#ifndef __EST_TYPES_H
#define __EST_TYPES_H

// clang-format off
#if defined(EST_EXPORT)
    #if defined(_WIN32)
        #define EST_API __declspec(dllexport)
    #elif defined(__GNUC__)
        #define EST_API __attribute__((visibility("default")))
    #endif
#else
    #if defined(_WIN32)
        #define EST_API __declspec(dllimport)
    #elif defined(__GNUC__)
        #define EST_API
    #endif
#endif
// clang-format on

enum EST_BOOL {
    EST_FALSE = 0,
    EST_TRUE = 1
};

enum EST_RESULT {
    EST_OK = 0,
    EST_ERROR = 1,

    EST_ERROR_OUT_OF_MEMORY = 2,
    EST_ERROR_INVALID_ARGUMENT = 3,
    EST_ERROR_INVALID_STATE = 4,
    EST_ERROR_INVALID_OPERATION = 5,
    EST_ERROR_INVALID_FORMAT = 6,
    EST_ERROR_INVALID_DATA = 7,
    EST_ERROR_TIMEDOUT = 8,
    EST_ERROR_ENCODER_EMPTY = 9,
    EST_ERROR_ENCODER_UNSUPPORTED = 10,
    EST_ERROR_ENCODER_INVALID_WRITE = 11
};

enum EST_DEVICE_FLAGS {
    EST_DEVICE_UNKNOWN,

    EST_DEVICE_MONO,   // Single channel audio
    EST_DEVICE_STEREO, // Two channel audio

    EST_DEVICE_FORMAT_S16, // Device signed 16 bit format (NOT IMPLEMENTED)
    EST_DEVICE_FORMAT_F32, // Device 32 bit floating point format (NOT IMPLEMENTED)

    EST_DEVICE_NOSTOP // Prevent the device from stopping when all samples are finished (NOT IMPLEMENTED)
};

enum EST_DECODER_FLAGS {
    EST_DECODER_UNKNOWN,

    EST_DECODER_MONO,
    EST_DECODER_STEREO,

    EST_DECODER_FORMAT_S16, // (NOT IMPLEMENTED)
    EST_DECODER_FORMAT_F32, // (NOT IMPLEMENTED)
};

enum EST_ATTRIBUTE_FLAGS {
    EST_ATTRIB_UNKNOWN,

    EST_ATTRIB_VOLUME = 0,  // Volume of the sample
    EST_ATTRIB_RATE = 1,    // Playback rate of the sample
    EST_ATTRIB_PITCH = 2,   // Pitch toggle
    EST_ATTRIB_PAN = 3,     // Pan of the sample
    EST_ATTRIB_LOOPING = 4, // Sample loop

    EST_ATTRIB_ENCODER_TEMPO = 5,      // Encoder tempo control which change audio rate without pitch change (different from sampleRate)
    EST_ATTRIB_ENCODER_PITCH = 6,      // Encoder pitch control without change the audio rate
    EST_ATTRIB_ENCODER_SAMPLERATE = 7, // Encoder both tempo and pitch control
};

enum EST_STATUS {
    EST_STATUS_UNKNOWN,

    EST_STATUS_IDLE,
    EST_STATUS_PLAYING,
    EST_STATUS_AT_END
};

// Export file format, currently only support WAV
// More format coming soon
enum EST_FILE_EXPORT {
    EST_EXPORT_UNKNOWN,

    EST_EXPORT_WAV // Export sample as wav 16bit format
};

typedef unsigned int EHANDLE;  // EstAudio handle, used for playback channel, thread safety: safe
typedef void        *ECHANDLE; // EstEncoder handle, used for encoder channel, thread safety: safe
typedef unsigned int EUINT32;
#define INVALID_HANDLE -1
#define INVALID_ECHANDLE (void *)0

typedef void (*est_audio_callback)(EHANDLE pHandle, void *pUserData, void *pData, int frameCount);
typedef void (*est_encoder_callback)(ECHANDLE pHandle, void *pUserData, void *pData, int frameCount);

typedef struct
{
    int                   sampleRate;
    int                   channels;
    int                   deviceIndex;
    enum EST_DEVICE_FLAGS flags;
} est_device_info;

typedef struct
{
    int sampleRate;
    int channels;
    int pcmSize;
} est_encoder_info;

#endif