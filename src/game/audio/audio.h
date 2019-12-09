#pragma once

#ifndef AUDIO_H
#define AUDIO_H

#include "always.h"
#include "adpcm.h"
#include "gamefile.h"
#include "hooker.h"

#define PAN_LEFT 32767
#define PAN_CENTER 0
#define PAN_RIGHT -32767

#pragma pack(push, 1)
struct AUDHeaderStruct
{
    unsigned short m_Rate; // Frequency rate of the sound data
    unsigned int m_Size; // Size of the packed data in the file, filesize - header
    unsigned int m_UncompSize; // Size of the uncompressed stream
    unsigned char m_Flags; // see SChannelFlag.
    unsigned char m_Compression; // see SCompressType.
};

struct AUDChunkHeaderStruct
{
    unsigned short m_Size; // Size of the chunk
    unsigned short m_OutSize; // Decompressed size of the chunk data
    unsigned int m_MagicID; // In little endian format.
};
#pragma pack(pop)

enum SoundCompressType
{
    COMP_NONE = 0,
    COMP_ZAP = 1,
    COMP_ADPCM = 99
};

enum
{
    AUD_CHUNK_MAGIC_ID = 0x0000DEAF,
    INVALID_AUDIO_HANDLE = -1,
    VOLUME_MIN = 0,
    VOLUME_MAX = 255,
    PRIORITY_MIN = 0,
    PRIORITY_MAX = 255,
#ifdef CHRONOSHIFT_STANDALONE
    MAX_SAMPLE_TRACKERS = 30, // RA has a issue where lots of sounds get cut
                              // off because of only a small number of trackers...
#else
    MAX_SAMPLE_TRACKERS = 5,
#endif
    STREAM_BUFFER_COUNT = 16,
    BUFFER_SIZE = 8192, // 256 * 32,
    UNCOMP_BUFFER_SIZE = (BUFFER_SIZE / 2) + 50,
    BUFFER_BYTES = 32768, // BUFFER_SIZE * 4, // 32 kb
    TIMER_DELAY = 25,
    TIMER_RESOLUTION = 1,
    TIMER_TARGET_RESOLUTION = 10 // 10-millisecond target resolution
};

typedef void (*AudioFocusCallback)();
typedef BOOL (*AudioStreamCallback)(short, short *, void **, int *);

// to avoid "definition of macro 'DSBFREQUENCY_MIN' not identical" spam
typedef struct IDirectSoundBuffer *LPDIRECTSOUNDBUFFER;

#pragma pack(push, 1)
struct SampleTrackerType
{
    BOOL m_Active; // Is this sample currently active?
    BOOL m_Loading; // Is this to be preloaded?
    BOOL m_DontTouch; // You heard the man. Seems to be used to skip callbacks.
    BOOL m_IsScore; // Is this sample score(music)?
    void *m_Original; // Original sample data pointer.
    int m_OriginalSize; // Original sample size, including header.
    LPDIRECTSOUNDBUFFER m_PlayBuffer;
    int m_PlaybackRate; // Samples per second.
    int m_BitSize; // Bit size represented as 0 on 8 bits, 2 on 16 bits.
    BOOL m_Stereo; // Is this sample stereo?
    int m_DataLength; // Unused.
    unsigned int m_DestPtr; // Used as both a pointer and cursor location.
    BOOL m_MoreSource; // Should load more data?
    BOOL m_OneShot; // Not sure, should play only once maybe?
    void *m_Source; // Sample data pointer, not including header.
    int m_Remainder; // Sample size not including header.
    CRITICAL_SECTION m_CriticalSection;
    int m_Priority; // Priority of this sample tracker.
    short m_Handle; // Sample ID.
    int m_Volume; // Volume of this sample tracker.
    int m_Reducer; // By how much to reduce the volume.
    SoundCompressType m_Compression; // Sample compression type.
    short m_TrailerLen; // Unused.
    char m_Trailer[32]; // Unused.
    int m_Pitch; // Unused.
    short m_Flags; // Unused.
    short m_Service; // Requires servicing, checked in callbacks.
    int m_Restart; // Unused.
    AudioStreamCallback m_Callback; // Function to call in callback loop when handling this tracker.
    void *m_QueueBuffer; // Queued file buffer.
    int m_QueueSize; // Queued file size.
    short m_Odd;
    int m_FilePending; // Number of pending files.
    int m_FilePendingSize; // Total pending size of all files.
    int m_FileHandle; // File handle of the current sample.
    void *m_FileBuffer; // Current file buffer.
    ADPCMStreamType m_StreamInfo; // ADPCM stream info, only used on COMP_ADPCM.

    void Close_Handle()
    {
        Close_File(m_FileHandle);
        m_FileHandle = INVALID_FILE_HANDLE;
    }
};
#pragma pack(pop)


struct LockedDataType
{
    int m_DigiHandle;
    int m_ServiceSomething;
    unsigned int m_MagicNumber;
    void *m_UncompBuffer;
    int m_StreamBufferSize;
    short m_StreamBufferCount;
    SampleTrackerType m_SampleTracker[MAX_SAMPLE_TRACKERS];
    int m_SoundVolume;
    int m_ScoreVolume;
    int m_LockCount;

    inline SampleTrackerType &operator[](int index) { return m_SampleTracker[index]; }
};

void Init_Locked_Data();
BOOL File_Callback(short id, short *odd, void **buffer, int *size);
int __cdecl Stream_Sample_Vol(void *buffer, int size, AudioStreamCallback callback, int volume, int handle);
int File_Stream_Sample(char const *filename, BOOL real_time_start);
void File_Stream_Preload(int handle);
int File_Stream_Sample_Vol(char const *filename, int volume, BOOL real_time_start);
void Sound_Callback();
void *Load_Sample(char *filename);
int Load_Sample_Into_Buffer(char *filename, void *buffer, int size);
int Sample_Read(int handle, void *buffer, int size);
void Free_Sample(void *sample);
void CALLBACK Sound_Timer_Callback(UINT uID = 0, UINT uMsg = 0, DWORD_PTR dwUser = 0, DWORD_PTR dw1 = 0, DWORD_PTR dw2 = 0);
void Sound_Thread(void *a1);
bool Set_Primary_Buffer_Format();
int Print_Sound_Error(char *sound_error, void *window);
bool Audio_Init(HWND window, int bits_per_sample, bool stereo, int rate, bool reverse_channels);
void Sound_End();
void Stop_Sample(int index);
bool Sample_Status(int index);
int Is_Sample_Playing(void *sample);
void Stop_Sample_Playing(void *sample);
int Get_Free_Sample_Handle(int priority);
int Play_Sample(void *sample, int priority, int volume, short panloc);
bool Attempt_Audio_Restore(LPDIRECTSOUNDBUFFER sound_buffer);
int Convert_HMI_To_Direct_Sound_Volume(int vol);
int Play_Sample_Handle(void *a1, int a2, int a3, short a4, int a5);
void Restore_Sound_Buffers();
int Set_Sound_Vol(int vol);
int Set_Score_Vol(int vol);
void Fade_Sample(int index, int ticks);
void Unfade_Sample(int index, int ticks);
int Get_Digi_Handle();
unsigned int Sample_Length(void *sample);
int Start_Primary_Sound_Buffer(BOOL forced);
void Stop_Primary_Sound_Buffer();
void Suspend_Audio_Thread();
void Resume_Audio_Thread();

int Simple_Copy(void **source, int *ssize, void **alternate, int *altsize, void **dest, int size);
int Sample_Copy(SampleTrackerType *st, void **source, int *ssize, void **alternate, int *altsize, void *dest, int size,
    SoundCompressType sound_comp, void *trailer, short *trailersize);
void maintenance_callback();
inline void *Audio_Add_Long_To_Pointer(void *ptr, int size)
{
    return (char *)ptr + size;
}

#endif // AUDIO_H
