#include "audio.h"
#include "alloc.h"
#include "eventhandler.h"
#include "globals.h"
#include "ostimer.h"
#include <algorithm>
#include <math.h>

#define DIRECTSOUND_VERSION 0x0600

// prevent sorting this cause it won't link
// clang-format off
#include <mmsystem.h>
#include <dsound.h>
// clang-format on
BOOL &g_ReverseChannels = Make_Global<BOOL>(0x006ABFDC);
LockedDataType &g_LockedData = Make_Global<LockedDataType>(0x006EC4D8);
LPDIRECTSOUND &g_SoundObject = Make_Global<LPDIRECTSOUND>(0x006ABFE0);
LPDIRECTSOUNDBUFFER &g_DumpBuffer = Make_Global<LPDIRECTSOUNDBUFFER>(0x006ABFBC);
LPDIRECTSOUNDBUFFER &g_PrimaryBufferPtr = Make_Global<LPDIRECTSOUNDBUFFER>(0x006ABFE4);
WAVEFORMATEX &g_DsBuffFormat = Make_Global<WAVEFORMATEX>(0x006ABFEC);
WAVEFORMATEX &g_PrimaryBuffFormat = Make_Global<WAVEFORMATEX>(0x006AC014);
DSBUFFERDESC &g_BufferDesc = Make_Global<DSBUFFERDESC>(0x006AC000);
DSBUFFERDESC &g_PrimaryBufferDesc = Make_Global<DSBUFFERDESC>(0x006AC028);
CRITICAL_SECTION &g_GlobalAudioCriticalSection = Make_Global<CRITICAL_SECTION>(0x006AC03C);
void *g_SoundThreadHandle = Make_Global<void *>(0x006ABFC0);
BOOL &g_SoundThreadActive = Make_Global<BOOL>(0x006ABFC4);
int &g_StartingFileStream = Make_Global<int>(0x006ABFC8);
MemoryFlagType &g_StreamBufferFlag = Make_Global<MemoryFlagType>(0x006ABFD0);
int &g_Misc = Make_Global<int>(0x006ABFD4);
UINT &g_SoundTimerHandle = Make_Global<UINT>(0x006ABFE8);
void *g_FileStreamBuffer = Make_Global<void *>(0x006AC054);
BOOL volatile &g_AudioDone = Make_Global<BOOL>(0x006AC05C);
short &g_SoundType = Make_Global<short>(0x006ABFD8);
short &g_SampleType = Make_Global<short>(0x006ABFDA);

#include "gbuffer.h"
// not sure where to put this
bool Any_Locked()
{
    // captainslog_debug(__FUNCTION__);
    if (g_SeenBuff.Get_LockCount() || g_HidPage.Get_LockCount()) {
        return true;
    }
    return false;
}

/**
 *
 * DEBUGGED
 */
void Init_Locked_Data()
{
    captainslog_debug(__FUNCTION__);
    g_LockedData.m_DigiHandle = INVALID_AUDIO_HANDLE;
    g_LockedData.m_ServiceSomething = 0;
    g_LockedData.m_MagicNumber = AUD_CHUNK_MAGIC_ID;
    g_LockedData.m_UncompBuffer = 0;
    g_LockedData.m_StreamBufferSize = BUFFER_SIZE + 128;
    g_LockedData.m_StreamBufferCount = STREAM_BUFFER_COUNT;
    g_LockedData.m_SoundVolume = VOLUME_MAX;
    g_LockedData.m_ScoreVolume = VOLUME_MAX;
    g_LockedData.m_LockCount = 0;
}

/**
 * Callback executed in sound callback on score(music) samples.
 * DEBUGGED
 */
BOOL File_Callback(short id, short *odd, void **buffer, int *size)
{
    // DEFINE_CALL(func, 0x005BDE70, BOOL, short, short *, void **, int *);
    // return func(id, odd, buffer, size);

    captainslog_debug(__FUNCTION__);

    if (id == INVALID_AUDIO_HANDLE) {
        return false;
    }

    SampleTrackerType *st = &g_LockedData[id];

    if (st->m_FileBuffer == nullptr) {
        return false;
    }

    if (!g_AudioDone) {
        EnterCriticalSection(&g_GlobalAudioCriticalSection);
    }

    st->m_DontTouch = true;

    if (!g_AudioDone) {
        LeaveCriticalSection(&g_GlobalAudioCriticalSection);
    }

    if (*buffer == nullptr && st->m_FilePending) {
        *buffer = Audio_Add_Long_To_Pointer(
            st->m_FileBuffer, g_LockedData.m_StreamBufferSize * (*odd % g_LockedData.m_StreamBufferCount));
        --st->m_FilePending;
        ++*odd;
        *size = st->m_FilePending == 0 ? st->m_FilePendingSize : g_LockedData.m_StreamBufferSize;
    }

    if (!g_AudioDone) {
        EnterCriticalSection(&g_GlobalAudioCriticalSection);
    }

    st->m_DontTouch = false;

    if (!g_AudioDone) {
        LeaveCriticalSection(&g_GlobalAudioCriticalSection);
    }

    Sound_Timer_Callback();

    int count = g_StreamLowImpact ? g_LockedData.m_StreamBufferCount / 2 : g_LockedData.m_StreamBufferCount - 3;

    if (count > st->m_FilePending && st->m_FileHandle != INVALID_FILE_HANDLE) {
        if (g_LockedData.m_StreamBufferCount - 2 != st->m_FilePending) {
            // Fill empty buffers.
            for (int num_empty_buffers = g_LockedData.m_StreamBufferCount - 2 - st->m_FilePending;
                 num_empty_buffers && st->m_FileHandle != INVALID_FILE_HANDLE;
                 --num_empty_buffers) {
                // Buffer to fill with data.
                void *tofill = Audio_Add_Long_To_Pointer(st->m_FileBuffer,
                    g_LockedData.m_StreamBufferSize * ((st->m_FilePending + *odd) % g_LockedData.m_StreamBufferCount));

                int psize = Read_File(st->m_FileHandle, tofill, g_LockedData.m_StreamBufferSize);

                if (psize != g_LockedData.m_StreamBufferSize) {
                    st->Close_Handle();
                }

                if (psize > 0) {
                    if (!g_AudioDone) {
                        EnterCriticalSection(&g_GlobalAudioCriticalSection);
                    }
                    st->m_DontTouch = true;
                    st->m_FilePendingSize = psize;
                    ++st->m_FilePending;
                    st->m_DontTouch = false;

                    if (!g_AudioDone) {
                        LeaveCriticalSection(&g_GlobalAudioCriticalSection);
                    }

                    Sound_Timer_Callback();
                }
            }
        }

        if (!g_AudioDone) {
            EnterCriticalSection(&g_GlobalAudioCriticalSection);
        }

        st->m_DontTouch = true;

        if (st->m_QueueBuffer == nullptr && st->m_FilePending) {
            st->m_QueueBuffer = Audio_Add_Long_To_Pointer(
                st->m_FileBuffer, g_LockedData.m_StreamBufferSize * (st->m_Odd % g_LockedData.m_StreamBufferCount));
            --st->m_FilePending;
            ++st->m_Odd;
            st->m_QueueSize = st->m_FilePending > 0 ? g_LockedData.m_StreamBufferSize : st->m_FilePendingSize;
        }

        st->m_DontTouch = false;

        if (!g_AudioDone) {
            LeaveCriticalSection(&g_GlobalAudioCriticalSection);
        }

        Sound_Timer_Callback();
    }

    if (st->m_FilePending) {
        return true;
    }

    return false;
}

/**
 * Starts playback of a score(music) sample in a specific tracker, at a specific volume.
 * DEBUGGED
 */
int __cdecl Stream_Sample_Vol(void *buffer, int size, AudioStreamCallback callback, int volume, int handle)
{
    // DEFINE_CALL(func, 0x005BE170, int, void *, int, AudioStreamCallback, int, int);
    // return func(buffer, size, callback, volume, handle);

    captainslog_debug(__FUNCTION__);

    if (g_AudioDone || buffer == nullptr || size == 0 || g_LockedData.m_DigiHandle == INVALID_AUDIO_HANDLE) {
        return INVALID_AUDIO_HANDLE;
    }

    AUDHeaderStruct header;

    memcpy(&header, buffer, sizeof(header));
    int oldsize = header.m_Size;
    header.m_Size = size - sizeof(header);
    memcpy(buffer, &header, sizeof(header));

    int playid = Play_Sample_Handle(buffer, PRIORITY_MAX, volume, 0, handle);

    header.m_Size = oldsize;
    memcpy(buffer, &header, sizeof(header));

    if (playid == INVALID_AUDIO_HANDLE) {
        return INVALID_AUDIO_HANDLE;
    }

    SampleTrackerType *st = &g_LockedData[playid];
    st->m_Callback = callback;
    st->m_Odd = 0;

    return playid;
}

/**
 * Loads a sample and puts it into the sample tracker as score(music).
 * DEBUGGED
 */
int File_Stream_Sample(char const *filename, BOOL real_time_start)
{
    captainslog_debug(__FUNCTION__);
    return File_Stream_Sample_Vol(filename, VOLUME_MAX, real_time_start);
}

/**
 * Preloads a sample for later playback.
 * DEBUGGED
 */
void File_Stream_Preload(int index)
{
    // DEFINE_CALL(func, 0x005BE250, void, int);
    // func(index);
    // return;

    captainslog_debug(__FUNCTION__);

    SampleTrackerType *st = &g_LockedData[index];
    int maxnum = (g_LockedData.m_StreamBufferCount / 2) + 4;
    int num = st->m_Loading ? std::min(st->m_FilePending + 2, maxnum) : maxnum;

    int i = 0;

    for (i = st->m_FilePending; i < num; ++i) {
        int size = Read_File(st->m_FileHandle,
            Audio_Add_Long_To_Pointer(st->m_FileBuffer, i * g_LockedData.m_StreamBufferSize),
            g_LockedData.m_StreamBufferSize);

        if (size > 0) {
            st->m_FilePendingSize = size;
            ++st->m_FilePending;
        }

        if (size < g_LockedData.m_StreamBufferSize) {
            break;
        }
    }

    Sound_Timer_Callback();

    if (g_LockedData.m_StreamBufferSize > st->m_FilePendingSize || i == maxnum) {
        int old_vol = g_LockedData.m_SoundVolume;

        int stream_size = st->m_FilePending == 1 ? st->m_FilePendingSize : g_LockedData.m_StreamBufferSize;

        g_LockedData.m_SoundVolume = g_LockedData.m_ScoreVolume;
        g_StartingFileStream = true;
        Stream_Sample_Vol(st->m_FileBuffer, stream_size, File_Callback, st->m_Volume, index);
        g_StartingFileStream = false;

        g_LockedData.m_SoundVolume = old_vol;

        st->m_Loading = false;
        --st->m_FilePending;
        if (st->m_FilePending == 0) {
            st->m_Odd = 0;
            st->m_QueueBuffer = 0;
            st->m_QueueSize = 0;
            st->m_FilePendingSize = 0;
            st->m_Callback = nullptr;
            Close_File(st->m_FileHandle);
        } else {
            st->m_Odd = 2;
            --st->m_FilePending;
            if (st->m_FilePendingSize != g_LockedData.m_StreamBufferSize) {
                st->Close_Handle();
            }
            st->m_QueueBuffer = Audio_Add_Long_To_Pointer(st->m_FileBuffer, g_LockedData.m_StreamBufferSize);
            st->m_QueueSize = st->m_FilePending == 0 ? st->m_FilePendingSize : g_LockedData.m_StreamBufferSize;
        }
    }
}

/**
 * Loads a sample and puts it into the sample tracker as score(music), sets a specific volume.
 * DEBUGGED
 */
int File_Stream_Sample_Vol(char const *filename, int volume, BOOL real_time_start)
{
    // DEFINE_CALL(func, 0x005BE430, int, char const *, int, BOOL);
    // return func(filename, volume, real_time_start);

    captainslog_debug(__FUNCTION__);

    if (g_LockedData.m_DigiHandle == INVALID_AUDIO_HANDLE || filename == nullptr || !Find_File(filename)) {
        return INVALID_AUDIO_HANDLE;
    }

    if (g_FileStreamBuffer == nullptr) {
        g_FileStreamBuffer = Alloc(
            g_LockedData.m_StreamBufferSize * g_LockedData.m_StreamBufferCount, g_StreamBufferFlag | MEM_LOCK | MEM_TEMP);
        for (int i = 0; i < MAX_SAMPLE_TRACKERS; ++i) {
            g_LockedData[i].m_FileBuffer = g_FileStreamBuffer;
        }
    }

    if (g_FileStreamBuffer == nullptr) {
        return INVALID_AUDIO_HANDLE;
    }

    int fh = Open_File(filename, 1);

    if (fh == INVALID_FILE_HANDLE) {
        return INVALID_AUDIO_HANDLE;
    }

    int handle = Get_Free_Sample_Handle(PRIORITY_MAX);

    if (handle < MAX_SAMPLE_TRACKERS) {
        SampleTrackerType *st = &g_LockedData[handle];
        st->m_IsScore = true;
        st->m_FilePending = 0;
        st->m_FilePendingSize = 0;
        st->m_Loading = real_time_start;
        st->m_Volume = volume;
        st->m_FileHandle = fh;
        File_Stream_Preload(handle);
        return handle;
    }

    return INVALID_AUDIO_HANDLE;
}

/**
 * Callback executed each loop.
 *
 */
void Sound_Callback()
{
    // DEFINE_CALL(func, 0x005BE560, void);
    // func();
    // return;

    // captainslog_debug(__FUNCTION__);

    if (!g_AudioDone && g_LockedData.m_DigiHandle != INVALID_AUDIO_HANDLE) {
        Sound_Timer_Callback();

        for (int i = 0; i < MAX_SAMPLE_TRACKERS; ++i) {
            SampleTrackerType *st = &g_LockedData[i];

            // Is a load pending?
            if (st->m_Loading) {
                File_Stream_Preload(i);
                // We are done with this sample.
                continue;
            }

            // Is this sample inactive?
            if (!st->m_Active) {
                // If so, we close the handle.
                if (st->m_FileHandle != INVALID_FILE_HANDLE) {
                    st->Close_Handle();
                }
                // We are done with this sample.
                continue;
            }

            // Has it been faded Is the volume 0?
            if (st->m_Reducer && !st->m_Volume) {
                // If so stop it.
                Stop_Sample(i);

                // We are done with this sample.
                continue;
            }

            // Process pending files.
            if (st->m_QueueBuffer == nullptr
                || st->m_FileHandle != INVALID_FILE_HANDLE && g_LockedData.m_StreamBufferCount - 3 > st->m_FilePending) {
                if (st->m_Callback != nullptr) {
                    if (!st->m_Callback(i, &st->m_Odd, &st->m_QueueBuffer, &st->m_QueueSize)) {
                        // No files are pending so pending file callback not needed anymore.
                        st->m_Callback = nullptr;
                    }
                }

                // We are done with this sample.
                continue;
            }
        }
    }
}

/**
 * Loads a sample from a file into memory.
 * DONE
 */
void *Load_Sample(char *filename)
{
    captainslog_debug(__FUNCTION__);

    if (g_LockedData.m_DigiHandle == INVALID_AUDIO_HANDLE || filename == nullptr || !Find_File(filename)) {
        return false;
    }

    void *data = nullptr;
    int handle = Open_File(filename, 1);

    if (handle != INVALID_FILE_HANDLE) {
        int data_size = File_Size(handle) + sizeof(AUDHeaderStruct);
        data = Alloc(data_size, MEM_NORMAL);
        if (data != nullptr) {
            Sample_Read(handle, data, data_size);
        }
        Close_File(handle);
        g_Misc = data_size;
    }

    return data;
}

/**
 * Opens and reads a sample into a fixed size buffer.
 * DONE
 */
int Load_Sample_Into_Buffer(char *filename, void *buffer, int size)
{
    captainslog_debug(__FUNCTION__);

    if (buffer == nullptr || size == 0 || g_LockedData.m_DigiHandle == INVALID_AUDIO_HANDLE || !filename
        || !Find_File(filename)) {
        return 0;
    }

    int handle = Open_File(filename, 1);
    if (handle == INVALID_FILE_HANDLE) {
        return 0;
    }

    int sample_size = Sample_Read(handle, buffer, size);
    Close_File(handle);
    return sample_size;
}

/**
 * Reads a already open sample into a fixed size buffer.
 * DEBUGGED
 */
int Sample_Read(int handle, void *buffer, int size)
{
    // DEFINE_CALL(func, 0x005BE780, int, int, void *, int);
    // return func(handle, buffer, size);

    captainslog_debug(__FUNCTION__);

    if (buffer == nullptr || handle == INVALID_AUDIO_HANDLE || size <= sizeof(AUDHeaderStruct)) {
        return 0;
    }

    AUDHeaderStruct header;
    int actual_bytes_read = Read_File(handle, &header, sizeof(AUDHeaderStruct));
    int to_read = std::min(size - sizeof(AUDHeaderStruct), header.m_Size);

    actual_bytes_read += Read_File(handle, Audio_Add_Long_To_Pointer(buffer, sizeof(AUDHeaderStruct)), to_read);

    memcpy(buffer, &header, sizeof(AUDHeaderStruct));

    return actual_bytes_read;
}

/**
 * Free's a allocated sample. Allocation must be made with Alloc.
 * DONE
 */
void Free_Sample(void *sample)
{
    captainslog_debug(__FUNCTION__);
    if (sample != nullptr) {
        Free(sample);
    }
}

/**
 * Callback executed on every timer event.
 * DEBUGGED
 */
void CALLBACK Sound_Timer_Callback(UINT uID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    if (!g_AudioDone) {
        EnterCriticalSection(&g_GlobalAudioCriticalSection);
        maintenance_callback();
        LeaveCriticalSection(&g_GlobalAudioCriticalSection);
    }
}

/**
 * Dedicated thread for sound.
 * DEBUGGED
 */
void Sound_Thread(void *a1)
{
    // DEFINE_CALL(func, 0x005BE830, void, void *);
    // func(a1);
    // return;

    // captainslog_debug(__FUNCTION__);

    // TODO : Find a alternative solution, this is the original code, and likely causes lockups on modern systems.
    DuplicateHandle(
        GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &g_SoundThreadHandle, THREAD_ALL_ACCESS, true, 0);
    SetThreadPriority(g_SoundThreadHandle, 15);
    g_SoundThreadActive = true;

    while (!g_AudioDone) {
        EnterCriticalSection(&g_GlobalAudioCriticalSection);
        maintenance_callback();
        LeaveCriticalSection(&g_GlobalAudioCriticalSection);
        PlatformTimerClass::Sleep(TIMER_DELAY);
    }

    g_SoundThreadActive = false;
}

/**
 * Attempts to set primary buffer format.
 * DEBUGGED
 */
bool Set_Primary_Buffer_Format()
{
    captainslog_debug(__FUNCTION__);
    if (g_SoundObject != nullptr && g_PrimaryBufferPtr != nullptr) {
        return g_PrimaryBufferPtr->SetFormat(&g_PrimaryBuffFormat) == DS_OK;
    }

    return false;
}

/**
 * Shows error that occured as a message box.
 * DEBUGGED
 */
int Print_Sound_Error(char *sound_error, void *window)
{
    return MessageBox((HWND)window, sound_error, "DirectSound Audio Error", MB_OK | MB_ICONWARNING);
}

/**
 * Initializes audio engine.
 * DEBUGGED
 */
bool Audio_Init(HWND window, int bits_per_sample, bool stereo, int rate, bool reverse_channels)
{
    // DEFINE_CALL(func, 0x005BE930, bool, HWND, int, bool, int, bool);
    // return func(window, bits_per_sample, stereo, rate, reverse_channels);

    captainslog_debug(__FUNCTION__);

    Init_Locked_Data();
    g_FileStreamBuffer = nullptr;
    memset(g_LockedData.m_SampleTracker, 0, sizeof(g_LockedData.m_SampleTracker));

    // Audio already init'ed.
    if (g_SoundObject != nullptr) {
        return true;
    }
    HRESULT return_code;

    return_code = DirectSoundCreate(NULL, &g_SoundObject, NULL);
    if (return_code != DS_OK) {
        captainslog_debug("Audio_Init - Failed to create Direct Sound Object. Error code %d.", return_code);
        return false;
    }

    return_code = g_SoundObject->SetCooperativeLevel(window, DSSCL_PRIORITY);
    if (return_code != DS_OK) {
        captainslog_debug("Audio_Init - Unable to set Direct Sound cooperative level. Error code %d.", return_code);
        g_SoundObject->Release();
        g_SoundObject = nullptr;

        return false;
    }

    // Set up DSBUFFERDESC structure.
    memset(&g_BufferDesc, 0, sizeof(g_BufferDesc));
    g_BufferDesc.dwSize = sizeof(DSBUFFERDESC);
    g_BufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;

    // Set up wave format structure.
    memset(&g_DsBuffFormat, 0, sizeof(g_DsBuffFormat));
    g_DsBuffFormat.wFormatTag = WAVE_FORMAT_PCM;
    g_DsBuffFormat.nChannels = stereo + 1;
    g_DsBuffFormat.nSamplesPerSec = rate;
    g_DsBuffFormat.wBitsPerSample = bits_per_sample;
    g_DsBuffFormat.nBlockAlign = g_DsBuffFormat.nChannels * g_DsBuffFormat.wBitsPerSample / 8; // todo confirm the math
    g_DsBuffFormat.nAvgBytesPerSec = g_DsBuffFormat.nBlockAlign * g_DsBuffFormat.nSamplesPerSec;
    g_DsBuffFormat.cbSize = 0;

    memcpy(&g_PrimaryBufferDesc, &g_BufferDesc, sizeof(g_PrimaryBufferDesc));
    memcpy(&g_PrimaryBuffFormat, &g_DsBuffFormat, sizeof(g_PrimaryBuffFormat));

    return_code = g_SoundObject->CreateSoundBuffer(&g_PrimaryBufferDesc, &g_PrimaryBufferPtr, NULL);
    if (return_code != DS_OK) {
        captainslog_debug("Audio_Init - Failed to create the primary sound buffer. Error code %d.");
        g_SoundObject->Release();
        g_SoundObject = nullptr;

        return false;
    }

    // Attempt to allocate buffer.
    if (!Set_Primary_Buffer_Format()) {
        captainslog_debug("Audio_Init - Failed to set primary buffer format.");

        int old_bits_per_sample = g_DsBuffFormat.wBitsPerSample;
        int old_block_align = g_DsBuffFormat.nBlockAlign;
        int old_bytes_per_sec = g_DsBuffFormat.nAvgBytesPerSec;

        if (g_DsBuffFormat.wBitsPerSample == 16) {
            captainslog_debug("Audio_Init - Trying a 8-bit primary buffer format.");
            g_DsBuffFormat.wBitsPerSample = 8;
            g_DsBuffFormat.nBlockAlign = g_DsBuffFormat.nChannels;
            g_DsBuffFormat.nAvgBytesPerSec = g_DsBuffFormat.nChannels * g_DsBuffFormat.nSamplesPerSec;

            memcpy(&g_PrimaryBufferDesc, &g_BufferDesc, sizeof(g_PrimaryBufferDesc));
            memcpy(&g_PrimaryBuffFormat, &g_DsBuffFormat, sizeof(g_PrimaryBuffFormat));
        }

        // Attempt to allocate 8 bit buffer.
        if (!Set_Primary_Buffer_Format()) {
            g_PrimaryBufferPtr->Release();
            g_PrimaryBufferPtr = nullptr;

            g_SoundObject->Release();
            g_SoundObject = nullptr;

            captainslog_debug("Audio_Init - Failed to set any primary buffer format. Disabling audio.");

            return false;
        }

        // Restore original format settings.
        g_DsBuffFormat.wBitsPerSample = old_bits_per_sample;
        g_DsBuffFormat.nBlockAlign = old_block_align;
        g_DsBuffFormat.nAvgBytesPerSec = old_bytes_per_sec;
    }

    // Attempt to start playback.
    return_code = g_PrimaryBufferPtr->Play(0, 0, DSBPLAY_LOOPING);
    if (return_code != DS_OK) {
        captainslog_debug("Audio_Init - Failed to start primary sound buffer. Disabling audio. Error code %d.");

        g_PrimaryBufferPtr->Release();
        g_PrimaryBufferPtr = nullptr;

        g_SoundObject->Release();
        g_SoundObject = nullptr;

        return false;
    }

    g_LockedData.m_DigiHandle = 1;

    InitializeCriticalSection(&g_GlobalAudioCriticalSection);

    g_SoundTimerHandle = timeSetEvent(TIMER_DELAY, TIMER_RESOLUTION, Sound_Timer_Callback, 0, 1);

    g_BufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
    g_BufferDesc.dwBufferBytes = BUFFER_BYTES;

    g_BufferDesc.lpwfxFormat = &g_DsBuffFormat;

    g_LockedData.m_UncompBuffer = Alloc(UNCOMP_BUFFER_SIZE, MEM_LOCK | MEM_CLEAR);
    if (g_LockedData.m_UncompBuffer == nullptr) {
        captainslog_debug("Audio_Init - Failed to allocate UncompBuffer.");
        return false;
    }

    // Create placback buffers for all trackers.
    for (int i = 0; i < MAX_SAMPLE_TRACKERS; ++i) {
        SampleTrackerType *st = &g_LockedData[i];

        return_code = g_SoundObject->CreateSoundBuffer(&g_BufferDesc, &st->m_PlayBuffer, NULL);
        if (return_code != DS_OK) {
            captainslog_debug("Audio_Init - Failed to allocate Play Buffer for tracker %d. Error code %d.", i, return_code);
        }

        st->m_PlaybackRate = rate;
        st->m_Stereo = stereo;
        st->m_BitSize = (bits_per_sample == 16 ? 2 : 0);

        st->m_FileHandle = INVALID_FILE_HANDLE;
        st->m_QueueBuffer = nullptr;

        InitializeCriticalSection(&st->m_CriticalSection);
    }

    g_SoundType = 1;
    g_SampleType = 1;

    g_ReverseChannels = reverse_channels;

    g_AudioDone = false;

    return true;
}

/**
 * Cleanup function when exiting the game.
 * DEBUGGED
 */
void Sound_End()
{
    captainslog_debug(__FUNCTION__);
    if (g_SoundObject != nullptr && g_PrimaryBufferPtr != nullptr) {
        for (int i = 0; i < MAX_SAMPLE_TRACKERS; ++i) {
            if (g_LockedData[i].m_PlayBuffer != nullptr) {
                Stop_Sample(i);
                g_LockedData[i].m_PlayBuffer->Stop();
                g_LockedData[i].m_PlayBuffer->Release();
                g_LockedData[i].m_PlayBuffer = nullptr;
                DeleteCriticalSection(&g_LockedData[i].m_CriticalSection);
            }
        }
    }

    if (g_FileStreamBuffer != nullptr) {
        Free(g_FileStreamBuffer);
        g_FileStreamBuffer = nullptr;
    }

    if (g_PrimaryBufferPtr != nullptr) {
        g_PrimaryBufferPtr->Stop();
        g_PrimaryBufferPtr->Release();
        g_PrimaryBufferPtr = nullptr;
    }

    if (g_SoundObject != nullptr) {
        g_SoundObject->Release();
        g_SoundObject = nullptr;
    }

    if (g_LockedData.m_UncompBuffer != nullptr) {
        Free(g_LockedData.m_UncompBuffer);
        g_LockedData.m_UncompBuffer = nullptr;
    }

    if (g_SoundTimerHandle != 0) {
        timeKillEvent(g_SoundTimerHandle);
        g_SoundTimerHandle = 0;
    }

    DeleteCriticalSection(&g_GlobalAudioCriticalSection);

    g_AudioDone = true;
}

/**
 * Stops a specific sample tracker.
 *
 */
void Stop_Sample(int index)
{
    // DEFINE_CALL(func, 0x005BEEA0, void, int);
    // func(index);
    // return;

    // captainslog_debug(__FUNCTION__);

    if (g_LockedData.m_DigiHandle != INVALID_AUDIO_HANDLE && index < MAX_SAMPLE_TRACKERS && !g_AudioDone) {
        EnterCriticalSection(&g_GlobalAudioCriticalSection);
        SampleTrackerType *st = &g_LockedData[index];

        if (st->m_Active || st->m_Loading) {
            st->m_Active = false;

            if (!st->m_IsScore) {
                st->m_Original = nullptr;
            }
            st->m_Priority = 0;

            if (!st->m_Loading) {
                st->m_PlayBuffer->Stop();
            }
            st->m_Loading = false;

            if (st->m_FileHandle != INVALID_FILE_HANDLE) {
                st->Close_Handle();
            }
            st->m_QueueBuffer = nullptr;
        }
        LeaveCriticalSection(&g_GlobalAudioCriticalSection);
    }
}

/**
 * Returns status of a specific sample.
 *
 */
bool Sample_Status(int index)
{
    // DEFINE_CALL(func, 0x005BEFB0, bool, int);
    // return func(index);

    // captainslog_debug(__FUNCTION__);

    if (g_AudioDone) {
        return false;
    }

    if (g_LockedData.m_DigiHandle == INVALID_AUDIO_HANDLE || index >= MAX_SAMPLE_TRACKERS) {
        return false;
    }

    SampleTrackerType *st = &g_LockedData[index];

    if (st->m_Loading) {
        return true;
    }

    if (!st->m_Active) {
        return false;
    }
    g_DumpBuffer = st->m_PlayBuffer;

    DWORD status;
    if (g_DumpBuffer->GetStatus(&status) != DS_OK) {
        captainslog_debug("Sample_Status - GetStatus failed");
        return true;
    }

    // original check, possible typo.
    // return (status & DSBSTATUS_PLAYING) || (status & DSBSTATUS_LOOPING);

    // Buffer has to be set as looping to qualify as playing.
    return (status & DSBSTATUS_PLAYING | DSBSTATUS_LOOPING) != false;
}

/**
 * Is this sample currently playing?
 * DEBUGGED
 */
int Is_Sample_Playing(void *sample)
{
    // DEFINE_CALL(func, 0x005BF050, int, void *);
    // return func(sample);

    // captainslog_debug(__FUNCTION__);

    if (g_AudioDone || sample == nullptr) {
        return false;
    }

    for (int i = 0; i < MAX_SAMPLE_TRACKERS; ++i) {
        if (sample == g_LockedData[i].m_Original && Sample_Status(i)) {
            return true;
        }
    }

    return false;
}

/**
 * Stops playing a specific sample.
 * DEBUGGED
 */
void Stop_Sample_Playing(void *sample)
{
    captainslog_debug(__FUNCTION__);
    if (sample != nullptr) {
        for (int i = 0; i < MAX_SAMPLE_TRACKERS; ++i) {
            if (g_LockedData[i].m_Original == sample) {
                Stop_Sample(i);
                break;
            }
        }
    }
}

/**
 * Gets a usable sample tracker slot based on priority.
 *
 */
int Get_Free_Sample_Handle(int priority)
{
    // DEFINE_CALL(func, 0x005BF0E0, int, int);
    // return func(priority);

    captainslog_debug(__FUNCTION__);

    int index = 0;

    for (index = MAX_SAMPLE_TRACKERS - 1; index >= 0; --index) {
        if (!g_LockedData[index].m_Active && !g_LockedData[index].m_Loading) {
            if (g_StartingFileStream || !g_LockedData[index].m_IsScore) {
                break;
            }
            g_StartingFileStream = 1;
        }
    }

    if (index < 0) {
        for (index = 0; index < MAX_SAMPLE_TRACKERS && g_LockedData[index].m_Priority > priority; ++index) {
            ;
        }

        if (index == MAX_SAMPLE_TRACKERS) {
            return INVALID_AUDIO_HANDLE;
        }
        Stop_Sample(index);
    }

    if (index == INVALID_AUDIO_HANDLE) {
        captainslog_debug("Get_Free_Sample_Handle - NO FREE TRACKERS!.");
        return INVALID_AUDIO_HANDLE;
    }

    if (g_LockedData[index].m_FileHandle != INVALID_FILE_HANDLE) {
        g_LockedData[index].Close_Handle();
    }

    if (g_LockedData[index].m_Original) {
        if (!g_LockedData[index].m_IsScore) {
            g_LockedData[index].m_Original = 0;
        }
    }
    g_LockedData[index].m_IsScore = 0;
    return index;
}

/**
 * Queue's a sample to be played.
 * DEBUGGED
 */
int Play_Sample(void *sample, int priority, int volume, short panloc)
{
    return Play_Sample_Handle(sample, priority, volume, panloc, Get_Free_Sample_Handle(priority));
}

/**
 * Attempts to restore sound buffer that was suspended on a focus loss.
 * DEBUGGED
 */
bool Attempt_Audio_Restore(LPDIRECTSOUNDBUFFER sound_buffer)
{
    // DEFINE_CALL(func, 0x005BF220, bool, LPDIRECTSOUNDBUFFER);
    // return func(sound_buffer);

    captainslog_debug(__FUNCTION__);

    HRESULT return_code = 0;
    DWORD play_status = 0;

    if (g_AudioDone || sound_buffer == nullptr) {
        return false;
    }

    if (g_AudioFocusLoss != nullptr) {
        g_AudioFocusLoss();
    }

    for (int restore_attempts = 0; restore_attempts < 2 && return_code == DSERR_BUFFERLOST; ++restore_attempts) {
        Restore_Sound_Buffers();
        HRESULT return_code = sound_buffer->GetStatus(&play_status);
    }
    return return_code != DSERR_BUFFERLOST;
}

/**
 * Attempts to play the buffer, returns id of the playing sample.
 *
 */
int Attempt_To_Play_Buffer(int id)
{
    // captainslog_debug(__FUNCTION__);
    HRESULT return_code;

    SampleTrackerType *st = &g_LockedData[id];

    // Attempts to play the current sample's buffer.
    do {
        return_code = st->m_PlayBuffer->Play(0, 0, 1);

        // Playback was started so we set some needed sample tracker values.
        if (return_code == DS_OK) {
            if (!g_AudioDone) {
                EnterCriticalSection(&g_GlobalAudioCriticalSection);
            }

            st->m_Active = true;
            st->m_DontTouch = false;
            st->m_Handle = id;

            if (!g_AudioDone) {
                LeaveCriticalSection(&g_GlobalAudioCriticalSection);
            }

            return st->m_Handle;
        }

        // A error we can't handle here occured.
        if (return_code != DSERR_BUFFERLOST) {
            // Flag this sample as not active.
            st->m_Active = false;
            break;
        }

        // We got a DSERR_BUFFERLOST so attempting to restore and if able to trying again.
        if (!Attempt_Audio_Restore(st->m_PlayBuffer)) {
            break;
        }
    } while (return_code == DSERR_BUFFERLOST);

    return INVALID_AUDIO_HANDLE;
}

/**
 * Converts volume this audio engine uses to DirectSound decibels.
 * DEBUGGED
 */
int Convert_HMI_To_Direct_Sound_Volume(int vol)
{
    // DEFINE_CALL(func, 0x005BF280, int, int);
    // return func(vol);

    // captainslog_debug(__FUNCTION__);

    // Complete silence.
    if (vol <= 0) {
        return DSBVOLUME_MIN;
    }

    // Max volume.
    if (vol >= 255) {
        return DSBVOLUME_MAX;
    }

    // Dark magic.
    double v = exp((255.0 - vol) * log(10000 + 1) / 255.0);

    // Simple clamping. 10000.99 would clamp to 9999.99.
    // Flip the value as we need a inverted value for DirectSound.
    return -(v + -1.0);
}

/**
 * Plays a loaded sample using a specific sample tracker.
 * DEBUGGED
 */
int Play_Sample_Handle(void *sample, int priority, int volume, short panloc, int id)
{
    // DEFINE_CALL(func, 0x005BF2E0, int, void *, int, int, short, int);
    // return func(sample, priority, volume, panloc, id);

    // captainslog_debug(__FUNCTION__);

    HRESULT return_code;
    unsigned long status;

    if (Any_Locked()) {
        // BUGFIX: Original returned 0 which im pretty sure could be a valid handle.
        return INVALID_AUDIO_HANDLE;
    }

    if (!g_AudioDone) {
        if (sample == nullptr || g_LockedData.m_DigiHandle == INVALID_AUDIO_HANDLE) {
            return INVALID_AUDIO_HANDLE;
        }

        if (id == INVALID_AUDIO_HANDLE) {
            return INVALID_AUDIO_HANDLE;
        }

        SampleTrackerType *st = &g_LockedData.m_SampleTracker[id];

        // Read in the sample's header.
        AUDHeaderStruct RawHeader;
        memcpy(&RawHeader, sample, sizeof(AUDHeaderStruct));

        // We don't support anything lower than 20000 hz.
        if (RawHeader.m_Rate < 24000 && RawHeader.m_Rate > 20000) {
            RawHeader.m_Rate = 22050;
        }

        if (!g_AudioDone) {
            EnterCriticalSection(&g_GlobalAudioCriticalSection);
        }

        // Set up basic sample tracker info.
        st->m_Compression = SoundCompressType(RawHeader.m_Compression);
        st->m_Original = sample;
        st->m_DontTouch = true;
        st->m_Odd = 0;
        st->m_Reducer = 0;
        st->m_Restart = 0;
        st->m_QueueBuffer = 0;
        st->m_QueueSize = 0;
        st->m_TrailerLen = 0;
        st->m_OriginalSize = RawHeader.m_Size + sizeof(AUDHeaderStruct);
        st->m_Priority = priority;
        st->m_Service = 0;
        st->m_Remainder = RawHeader.m_Size;
        st->m_Source = Audio_Add_Long_To_Pointer(sample, sizeof(AUDHeaderStruct));

        if (!g_AudioDone) {
            LeaveCriticalSection(&g_GlobalAudioCriticalSection);
        }

        // Compression is ADPCM so we need to init it's stream info.
        if (st->m_Compression == COMP_ADPCM) {
            st->m_StreamInfo.m_Channels = (RawHeader.m_Flags & 1) + 1;
            st->m_StreamInfo.m_BitsPerSample = RawHeader.m_Flags & 2 ? 16 : 8;
            st->m_StreamInfo.m_CompSize = RawHeader.m_Size;
            st->m_StreamInfo.m_UnCompSize = RawHeader.m_Size * (st->m_StreamInfo.m_BitsPerSample / 4);
            ADPCM_Stream_Init(&st->m_StreamInfo);
        }

        // If the loaded sample doesn't match the sample tracker we need to adjust the tracker.
        if (RawHeader.m_Rate != st->m_PlaybackRate || (RawHeader.m_Flags & 2) != (st->m_BitSize & 2)
            || (RawHeader.m_Flags & 1) != (st->m_Stereo & 1)) {
            captainslog_debug("Play_Sample_Handle - Changing sample format.");
            st->m_Active = false;
            st->m_Service = 0;
            st->m_MoreSource = false;
            g_DumpBuffer = st->m_PlayBuffer;

            // Querry the playback status.
            do {
                return_code = st->m_PlayBuffer->GetStatus(&status);
                if (return_code == DSERR_BUFFERLOST && !Attempt_Audio_Restore(st->m_PlayBuffer)) {
                    captainslog_debug("Play_Sample_Handle - Unable to get PlaybBuffer status!");
                    return INVALID_AUDIO_HANDLE;
                }
            } while (return_code == DSERR_BUFFERLOST);

            // Stop the sample if its already playing.
            // TODO: Investigate this, logics here are possibly wrong.
            // - What happens when we call Restore when we have stopped the the buffer?
            // - Stop return isn't checked, in TS it checks for DSERR_BUFFERLOST, but thats not a valid Stop return.
            if (status & (DSBSTATUS_PLAYING | DSBSTATUS_LOOPING)
                && (st->m_PlayBuffer->Stop(), return_code == DSERR_BUFFERLOST) && !Attempt_Audio_Restore(st->m_PlayBuffer)) {
                captainslog_debug("Play_Sample_Handle - Unable to stop buffer!");
                return INVALID_AUDIO_HANDLE;
            }

            st->m_PlayBuffer->Release();
            st->m_PlayBuffer = nullptr;
            g_DsBuffFormat.nSamplesPerSec = RawHeader.m_Rate;
            g_DsBuffFormat.nChannels = (RawHeader.m_Flags & 1) + 1;
            g_DsBuffFormat.wBitsPerSample = RawHeader.m_Flags & 2 ? 16 : 8;
            g_DsBuffFormat.nBlockAlign = g_DsBuffFormat.nChannels * g_DsBuffFormat.wBitsPerSample / 8;
            g_DsBuffFormat.nAvgBytesPerSec = g_DsBuffFormat.nBlockAlign * g_DsBuffFormat.nSamplesPerSec;

            // Attempt to create a new buffer for this new sample.
            return_code = g_SoundObject->CreateSoundBuffer(&g_BufferDesc, &st->m_PlayBuffer, 0);
            if (return_code == DSERR_BUFFERLOST && !Attempt_Audio_Restore(st->m_PlayBuffer)) {
                return INVALID_AUDIO_HANDLE;
            }

            // We failed to create the buffer, bail!
            if (return_code != DS_OK && return_code != DSERR_BUFFERLOST) {
                st->m_PlaybackRate = 0;
                st->m_Stereo = false;
                st->m_BitSize = 0;
                captainslog_debug("Play_Sample_Handle - Bad sample format!");
                return INVALID_AUDIO_HANDLE;
            }

            // Set the new sample info.
            st->m_PlaybackRate = RawHeader.m_Rate;
            st->m_Stereo = RawHeader.m_Flags & 1;
            st->m_BitSize = RawHeader.m_Flags & 2;
        }

        // If the sample tracker matches the loaded file we load the samples.
        do {
            g_DumpBuffer = st->m_PlayBuffer;
            return_code = g_DumpBuffer->GetStatus(&status);
            if (return_code == DSERR_BUFFERLOST && !Attempt_Audio_Restore(st->m_PlayBuffer)) {
                captainslog_debug("Play_Sample_Handle - Can't get DumpBuffer status.");
                return INVALID_AUDIO_HANDLE;
            }
        } while (return_code == DSERR_BUFFERLOST);

        // If the sample is already playing stop it.
        if (status & (DSBSTATUS_PLAYING | DSBSTATUS_LOOPING)) {
            st->m_Active = false;
            st->m_Service = 0;
            st->m_MoreSource = false;
            st->m_PlayBuffer->Stop();
        }

        LPVOID play_buffer_ptr;
        DWORD lock_length1;
        LPVOID dummy_buffer_ptr;
        DWORD lock_length2;

        do {
            return_code = st->m_PlayBuffer->Lock(
                0, BUFFER_BYTES, &play_buffer_ptr, &lock_length1, &dummy_buffer_ptr, &lock_length2, 0);
            if (return_code == DSERR_BUFFERLOST && !Attempt_Audio_Restore(st->m_PlayBuffer)) {
                captainslog_debug("Play_Sample_Handle - Unable to lock buffer! Buffer lost.");
                return INVALID_AUDIO_HANDLE;
            }
        } while (return_code == DSERR_BUFFERLOST);

        if (return_code != DS_OK) {
            captainslog_debug("Play_Sample_Handle - Unable to lock buffer! Unknown error.");
            return INVALID_AUDIO_HANDLE;
        }

        st->m_DestPtr = Sample_Copy(st,
            &st->m_Source,
            &st->m_Remainder,
            &st->m_QueueBuffer,
            &st->m_QueueSize,
            play_buffer_ptr,
            BUFFER_SIZE,
            st->m_Compression,
            st->m_Trailer,
            &st->m_TrailerLen);

        if (st->m_DestPtr == BUFFER_SIZE) {
            st->m_MoreSource = true;
            st->m_Service = 1;
            st->m_OneShot = false;
        } else {
            st->m_MoreSource = false;
            st->m_OneShot = true;
            st->m_Service = 1;
            memset(Audio_Add_Long_To_Pointer(play_buffer_ptr, st->m_DestPtr), 0, BUFFER_SIZE);
        }

        st->m_PlayBuffer->Unlock(play_buffer_ptr, lock_length1, dummy_buffer_ptr, lock_length2);
        st->m_Volume = volume * 128;

        do {
            st->m_PlayBuffer->SetVolume(Convert_HMI_To_Direct_Sound_Volume((volume * g_LockedData.m_SoundVolume) / 256));
            if (return_code == DSERR_BUFFERLOST && !Attempt_Audio_Restore(st->m_PlayBuffer)) {
                captainslog_debug("Play_Sample_Handle - Can't set volume!");
                return INVALID_AUDIO_HANDLE;
            }
        } while (return_code == DSERR_BUFFERLOST);

        if (!Start_Primary_Sound_Buffer(false)) {
            captainslog_debug("Play_Sample_Handle - Can't start primary buffer!");
            return INVALID_AUDIO_HANDLE;
        }

        // Reset buffer playback position.
        do {
            return_code = st->m_PlayBuffer->SetCurrentPosition(0);
            if (return_code == DSERR_BUFFERLOST && !Attempt_Audio_Restore(st->m_PlayBuffer)) {
                captainslog_debug("Play_Sample_Handle - Can't set current position!");
                return INVALID_AUDIO_HANDLE;
            }
        } while (return_code == DSERR_BUFFERLOST);

        /*
        while (1) {
            return_code = st->m_PlayBuffer->Play(0, 0, 1);
            if (return_code == DS_OK) {
                break;
            }

            if (return_code != DSERR_BUFFERLOST) {
                st->m_Active = false;
                return INVALID_AUDIO_HANDLE;
            }

            if (!Attempt_Audio_Restore(st->m_PlayBuffer)) {
                return INVALID_AUDIO_HANDLE;
            }
        }

        if (!g_AudioDone) {
            EnterCriticalSection(&g_GlobalAudioCriticalSection);
        }

        st->m_Active = true;
        st->m_DontTouch = false;
        st->m_Handle = id;

        if (!g_AudioDone) {
            LeaveCriticalSection(&g_GlobalAudioCriticalSection);
        }
        return st->m_Handle;
        */
        return Attempt_To_Play_Buffer(id);
    }

    return INVALID_AUDIO_HANDLE;
}

/**
 * Restores primary and all sample tracker sound buffers lost on a focus loss.
 * DEBUGGED
 */
void Restore_Sound_Buffers()
{
    captainslog_debug(__FUNCTION__);
    if (g_PrimaryBufferPtr != nullptr) {
        g_PrimaryBufferPtr->Restore();
    }

    for (int i = 0; i < MAX_SAMPLE_TRACKERS; ++i) {
        if (g_LockedData[i].m_PlayBuffer != nullptr) {
            g_LockedData[i].m_PlayBuffer->Restore();
        }
    }
}

/**
 * Sets regular sound volume.
 * DEBUGGED
 */
int Set_Sound_Vol(int vol)
{
    captainslog_debug(__FUNCTION__);
    int oldvol = g_LockedData.m_SoundVolume;
    g_LockedData.m_SoundVolume = vol;
    return oldvol;
}

/**
 * Sets score(music) volume.
 * DEBUGGED
 */
int Set_Score_Vol(int volume)
{
    // DEFINE_CALL(func, 0x005BF940, int, int);
    // return func(volume);

    captainslog_debug(__FUNCTION__);

    int old = g_LockedData.m_ScoreVolume;
    g_LockedData.m_ScoreVolume = volume;

    for (int i = 0; i < MAX_SAMPLE_TRACKERS; ++i) {
        SampleTrackerType *st = &g_LockedData[i];
        if (st->m_IsScore & st->m_Active) {
            st->m_PlayBuffer->SetVolume(
                Convert_HMI_To_Direct_Sound_Volume((g_LockedData.m_ScoreVolume * (st->m_Volume / 128)) / 256));
        }
    }
    return old;
}

/**
 * Gradual fade a sample.
 * DEBUGGED
 */
void Fade_Sample(int index, int ticks)
{
    // DEFINE_CALL(func, 0x005BF9B0, void, int, int);
    // func(index, ticks);
    // return;

    captainslog_debug(__FUNCTION__);

    if (Sample_Status(index)) {
        SampleTrackerType *st = &g_LockedData[index];

        if (ticks > 0 && !st->m_Loading) {
            st->m_Reducer = ((st->m_Volume / ticks) + 1);
        } else {
            Stop_Sample(index);
        }
    }
}

/**
 * Gradual unfade a faded sample.
 * DONE
 */
void Unfade_Sample(int index, int ticks)
{
    captainslog_debug(__FUNCTION__);
    if (Sample_Status(index)) {
        SampleTrackerType *st = &g_LockedData[index];

        if (ticks > 0 && !st->m_Loading) {
            st->m_Reducer -= ((st->m_Volume / ticks) + 1);
        } else {
            st->m_Reducer = 0;
        }
    }
}

/**
 *
 * DONE
 */
int Get_Digi_Handle()
{
    return g_LockedData.m_DigiHandle;
}

/**
 * Determines sample length.
 * DONE
 */
unsigned int Sample_Length(void *sample)
{
    captainslog_debug(__FUNCTION__);

    if (sample == nullptr) {
        return 0;
    }

    AUDHeaderStruct header;
    memcpy(&header, sample, sizeof(header));
    unsigned int time = header.m_UncompSize;

    if (header.m_Flags & 2) {
        time /= 2;
    }

    if (header.m_Flags & 1) {
        time /= 2;
    }

    if (header.m_Rate / 60 > 0) {
        time /= header.m_Rate / 60;
    }
    return time;
}

/**
 * Starts playback of the primary buffer.
 * DEBUGGED
 */
int Start_Primary_Sound_Buffer(BOOL forced)
{
    // DEFINE_CALL(func, 0x005BFA70, int, int);
    // return func(forced);

    // captainslog_debug(__FUNCTION__);

    if (g_PrimaryBufferPtr == nullptr || !g_GameInFocus) {
        return false;
    }

    if (forced) {
        g_PrimaryBufferPtr->Play(0, 0, DSBPLAY_LOOPING);
        return true;
    }

    DWORD status = 0;
    if (g_PrimaryBufferPtr->GetStatus(&status) != DS_OK) {
        return false;
    }

    // Primary buffer has to be set as looping to qualify as playing.
    if (status & DSBSTATUS_PLAYING | DSBSTATUS_LOOPING) {
        return true;
    }

    g_PrimaryBufferPtr->Play(0, 0, DSBPLAY_LOOPING);
    return true;
}

/**
 * Stops playback of the primary buffer and associated samples.
 * DEBUGGED?
 */
void Stop_Primary_Sound_Buffer()
{
    captainslog_debug(__FUNCTION__);
    /*
    if (g_PrimaryBufferPtr != nullptr) {
        // why
        g_PrimaryBufferPtr->Stop();
        g_PrimaryBufferPtr->Stop();
        g_PrimaryBufferPtr->Stop();
        g_PrimaryBufferPtr->Stop();
    }

    for (int i = 0; i < MAX_SAMPLE_TRACKERS; ++i) {
        Stop_Sample(i);
    }
    */

    // BUGFIX: Original code is above, Blade Runner has this function like this.
    for (int i = 0; i < MAX_SAMPLE_TRACKERS; ++i) {
        Stop_Sample(i);
    }

    if (g_PrimaryBufferPtr != nullptr) {
        g_PrimaryBufferPtr->Stop();
    }
}

/**
 * Suspends a active audio thread.
 * DEBUGGED
 */
void Suspend_Audio_Thread()
{
    captainslog_debug(__FUNCTION__);
    if (g_SoundThreadActive) {
        timeKillEvent(g_SoundTimerHandle);
        g_SoundTimerHandle = 0;
        g_SoundThreadActive = false;
    }
}

/**
 * Resumes a suspended audio thread.
 * DEBUGGED
 */
void Resume_Audio_Thread()
{
    captainslog_debug(__FUNCTION__);
    if (!g_SoundThreadActive) {
        g_SoundTimerHandle = timeSetEvent(TIMER_DELAY, TIMER_RESOLUTION, Sound_Timer_Callback, g_SoundThreadActive, 1);
        g_SoundThreadActive = true;
    }
}

void maintenance_callback()
{
    // captainslog_debug(__FUNCTION__);
    // DEFINE_CALL(func, 0x005D8670, void);
    // func();
    // return;
    /*
        SampleTrackerType *st = nullptr;

        for (int i = 0; i < MAX_SAMPLE_TRACKERS; ++i) {
            st = &g_LockedData[i];
            if (st->m_Active) {
                if (st->m_Service && !st->m_DontTouch) {
                    st->m_DontTouch = true;

                    DWORD play_cursor;
                    DWORD write_cursor;
                    HRESULT return_code = st->m_PlayBuffer->GetCurrentPosition(&play_cursor, &write_cursor);

                    if (return_code != DS_OK) {
                        captainslog_debug("maintenance_callback - Failed to get Buffer play position!");
                        if (return_code == DSERR_BUFFERLOST) {
                            if (g_AudioFocusLoss) {
                                g_AudioFocusLoss();
                            }
                        }
                        return;
                    }

                    if (st->m_MoreSource) {
                        bool write_more = false;

                        if (play_cursor < st->m_DestPtr) {
                            if (st->m_DestPtr - play_cursor <= BUFFER_SIZE) {
                                write_more = true;
                            }
                        } else if (play_cursor > BUFFER_BYTES - BUFFER_SIZE && st->m_DestPtr == 0) {
                            write_more = true;
                        }

                        LPVOID play_buffer_ptr;
                        LPVOID dummy_buffer_ptr;
                        DWORD lock_length1;
                        DWORD lock_length2;

                        if (write_more
                            && st->m_PlayBuffer->Lock(st->m_DestPtr,
                                   BUFFER_BYTES / 2,
                                   &play_buffer_ptr,
                                   &lock_length1,
                                   &dummy_buffer_ptr,
                                   &lock_length2,
                                   0)
                                == DS_OK) {
                            int bytes_copied = Sample_Copy(st,
                                &st->m_Source,
                                &st->m_Remainder,
                                &st->m_QueueBuffer,
                                &st->m_QueueSize,
                                play_buffer_ptr,
                                BUFFER_SIZE,
                                st->m_Compression,
                                st->m_Trailer,
                                &st->m_TrailerLen);

                            if (bytes_copied != BUFFER_SIZE) {
                                st->m_MoreSource = false;

                                memset(Audio_Add_Long_To_Pointer(play_buffer_ptr, bytes_copied), 0, BUFFER_SIZE -
       bytes_copied);

                                if (st->m_DestPtr == BUFFER_BYTES - BUFFER_SIZE) {
                                    if (dummy_buffer_ptr && lock_length2) {
                                        memset(dummy_buffer_ptr, 0, lock_length2);
                                    }
                                } else {
                                    memset(Audio_Add_Long_To_Pointer(play_buffer_ptr, BUFFER_SIZE), 0, BUFFER_SIZE);
                                }
                            }

                            st->m_DestPtr = (int)Audio_Add_Long_To_Pointer((void *)st->m_DestPtr, bytes_copied);

                            if (st->m_DestPtr >= BUFFER_BYTES) {
                                st->m_DestPtr = (int)Audio_Add_Long_To_Pointer((void *)st->m_DestPtr, -BUFFER_BYTES);
                            }

                            st->m_PlayBuffer->Unlock(play_buffer_ptr, lock_length1, dummy_buffer_ptr, lock_length2);
                            // this code is in versions post RA
                            //if (!st->m_MoreSource) {
                            //    st->m_Remainder = st->m_OriginalSize - sizeof(AUDHeaderStruct);
                            //    st->m_Source = Audio_Add_Long_To_Pointer(st->m_Original, sizeof(AUDHeaderStruct));
                            //    st->m_MoreSource = true;
                            //}
                        }

                    } else if (play_cursor >= st->m_DestPtr && (play_cursor - st->m_DestPtr) < BUFFER_SIZE
                        || !st->m_OneShot && play_cursor < st->m_DestPtr
                            && (st->m_DestPtr - play_cursor) > BUFFER_BYTES - BUFFER_SIZE) {
                        st->m_PlayBuffer->Stop();
                        st->m_Service = 0;
                        Stop_Sample(i);
                    }
                    st->m_DontTouch = false;
                }

                if (!st->m_DontTouch && !st->m_QueueBuffer && st->m_FilePending) {
                    st->m_QueueBuffer = Audio_Add_Long_To_Pointer(
                        st->m_FileBuffer, (g_LockedData.m_StreamBufferSize * (st->m_Odd %
       g_LockedData.m_StreamBufferCount)));
                    --st->m_FilePending;
                    ++st->m_Odd;

                    if (st->m_FilePending == 0) {
                        st->m_QueueSize = st->m_FilePendingSize;
                    } else {
                        st->m_QueueSize = g_LockedData.m_StreamBufferSize;
                    }
                }
            }
        }

        if (g_LockedData.m_LockCount == 0) {
            ++g_LockedData.m_LockCount;
            // Set volume for all samples.
            for (int j = 0; j < MAX_SAMPLE_TRACKERS; ++j) {
                st = &g_LockedData[j];

                if (st->m_Active && st->m_Reducer > 0 && st->m_Volume > 0) {
                    if (st->m_Reducer >= st->m_Volume) {
                        st->m_Volume = VOLUME_MIN;
                    } else {
                        st->m_Volume -= st->m_Reducer;
                    }

                    int vol = st->m_IsScore ? (g_LockedData.m_ScoreVolume * (st->m_Volume / 128)) :
                                              (g_LockedData.m_SoundVolume * (st->m_Volume / 128));

                    st->m_PlayBuffer->SetVolume(Convert_HMI_To_Direct_Sound_Volume(vol / 256));
                }
            }

            --g_LockedData.m_LockCount;
        }
    */
    SampleTrackerType *st; // esi@1
    void **queuebuffer; // ebp@1
    int i; // edi@1
    IDirectSoundBuffer *v3; // eax@5
    HRESULT return_code; // eax@5
    SampleTrackerType *st_1; // esi@11
    signed int j; // edi@11
    unsigned int v7; // ecx@13
    signed int write_more; // edx@13
    int bytes_copied; // eax@21
    char *v11; // eax@23
    size_t v12; // ebx@25
    void *v13; // eax@28
    unsigned int v14; // edx@31
    unsigned int v15; // edx@34
    __int16 v16; // bx@41
    int v17; // edx@41
    IDirectSoundBuffer *v18; // edx@46
    signed int v20; // eax@46
    IDirectSoundBuffer *v21; // edx@47
    signed int v23; // eax@47
    int reducer; // edx@50
    int volume; // ebx@51
    DWORD play_cursor;
    DWORD write_cursor;
    LPVOID play_buffer_ptr;
    LPVOID dummy_buffer_ptr;
    DWORD lock_length1;
    DWORD lock_length2;
    __int16 *TrailerLen; // [sp+54h] [bp-24h]@1
    char *Trailer; // [sp+58h] [bp-20h]@1
    int *Remainder; // [sp+5Ch] [bp-1Ch]@1
    void **Source; // [sp+60h] [bp-18h]@1
    int *QueueSize; // [sp+64h] [bp-14h]@1

    st = g_LockedData.m_SampleTracker;
    Source = (void **)&g_LockedData.m_SampleTracker[0].m_Source;
    queuebuffer = &g_LockedData.m_SampleTracker[0].m_QueueBuffer;
    Remainder = &g_LockedData.m_SampleTracker[0].m_Remainder;
    QueueSize = &g_LockedData.m_SampleTracker[0].m_QueueSize;
    Trailer = g_LockedData.m_SampleTracker[0].m_Trailer;
    i = 0;
    TrailerLen = &g_LockedData.m_SampleTracker[0].m_TrailerLen;
    do {
        if (!st->m_Active) {
            goto get_next_tracker;
        }
        if (st->m_Service && !st->m_DontTouch) {
            st->m_DontTouch = 1;
            return_code = st->m_PlayBuffer->GetCurrentPosition(&play_cursor, &write_cursor);
            if (return_code) {
                if (return_code != DSERR_BUFFERLOST) {
                    return;
                }
                if (g_AudioFocusLoss) {
                    g_AudioFocusLoss();
                }
                return;
            }
            if (!st->m_MoreSource) {
                v14 = st->m_DestPtr;
                if (play_cursor >= v14 && play_cursor - v14 < BUFFER_SIZE
                    || !st->m_OneShot && (v15 = st->m_DestPtr, play_cursor < v15) && v15 - play_cursor > 24576) {
                    st->m_PlayBuffer->Stop();
                    st->m_Service = 0;
                    Stop_Sample(i);
                }
                goto skipwrite;
            }
            v7 = st->m_DestPtr;
            write_more = 0;
            if (play_cursor < v7) {
                if (v7 - play_cursor <= 0x2000) {
                LABEL_18:
                    write_more = 1;
                    goto callock;
                }
            } else if ((signed int)play_cursor > 24576 && !v7) {
                goto LABEL_18;
            }
        callock:
            if (!write_more
                || st->m_PlayBuffer->Lock(
                    st->m_DestPtr, 16384, &play_buffer_ptr, &lock_length1, &dummy_buffer_ptr, &lock_length2, 0)) {
            skipwrite:
                st->m_DontTouch = 0;
                goto LABEL_38;
            }
            bytes_copied = Sample_Copy(st,
                Source,
                Remainder,
                queuebuffer,
                QueueSize,
                play_buffer_ptr,
                BUFFER_SIZE,
                st->m_Compression,
                Trailer,
                TrailerLen);
            if (bytes_copied != BUFFER_SIZE) {
                st->m_MoreSource = 0;
                memset((char *)play_buffer_ptr + bytes_copied, 0, BUFFER_SIZE - bytes_copied);
                if (st->m_DestPtr == 24576) {
                    v11 = (char *)dummy_buffer_ptr;
                    if (!dummy_buffer_ptr || !lock_length2) {
                        goto nextptr;
                    }
                    v12 = lock_length2;
                } else {
                    v12 = BUFFER_SIZE;
                    v11 = (char *)play_buffer_ptr + BUFFER_SIZE;
                }
                memset(v11, 0, v12);
            }
        nextptr:
            int v13 = (int)Audio_Add_Long_To_Pointer((void *)st->m_DestPtr, bytes_copied);
            st->m_DestPtr = v13;
            if ((unsigned int)v13 >= BUFFER_BYTES) {
                st->m_DestPtr = (int)Audio_Add_Long_To_Pointer((void *)v13, -BUFFER_BYTES);
            }
            st->m_PlayBuffer->Unlock(play_buffer_ptr, lock_length1, dummy_buffer_ptr, lock_length2);
            goto skipwrite;
        }
    LABEL_38:
        if (!st->m_DontTouch && !st->m_QueueBuffer && st->m_FilePending) {
            st->m_QueueBuffer = Audio_Add_Long_To_Pointer(
                st->m_FileBuffer, g_LockedData.m_StreamBufferSize * (st->m_Odd % g_LockedData.m_StreamBufferCount));
            v16 = st->m_Odd;
            v17 = --st->m_FilePending;
            st->m_Odd = v16 + 1;
            if (!v17) {
                st->m_QueueSize = st->m_FilePendingSize;
            } else {
                st->m_QueueSize = g_LockedData.m_StreamBufferSize;
            }
        }
    get_next_tracker:
        queuebuffer = (void **)((char *)queuebuffer + sizeof(SampleTrackerType));
        ++st;
        ++i;
        Source = (void **)((char *)Source + sizeof(SampleTrackerType));
        Remainder = (int *)((char *)Remainder + sizeof(SampleTrackerType));
        QueueSize = (int *)((char *)QueueSize + sizeof(SampleTrackerType));
        Trailer += sizeof(SampleTrackerType);
        TrailerLen = (__int16 *)((char *)TrailerLen + sizeof(SampleTrackerType));
    } while (i < 5);
    if (!g_LockedData.m_LockCount) {
        st_1 = g_LockedData.m_SampleTracker;
        j = 0;
        ++g_LockedData.m_LockCount;
        do {
            if (st_1->m_Active) {
                if (st_1->m_Reducer > 0 && st_1->m_Volume > 0) {
                    if (st_1->m_Reducer >= st_1->m_Volume) {
                        st_1->m_Volume = 0;
                    } else {
                        st_1->m_Volume = st_1->m_Volume - st_1->m_Reducer;
                    }

                    if (!st_1->m_IsScore) {
                        st_1->m_PlayBuffer->SetVolume(Convert_HMI_To_Direct_Sound_Volume(
                            (unsigned int)(g_LockedData.m_SoundVolume * (st_1->m_Volume >> 7)) >> 8));
                    } else {
                        st_1->m_PlayBuffer->SetVolume(Convert_HMI_To_Direct_Sound_Volume(
                            (unsigned int)(g_LockedData.m_ScoreVolume * (st_1->m_Volume >> 7)) >> 8));
                    }
                }
            }
            ++j;
            ++st_1;
        } while (j < 5);
        --g_LockedData.m_LockCount;
    }
}

/**
 * Simple copies one buffer to another.
 * DEBUGGED
 */
int Simple_Copy(void **source, int *ssize, void **alternate, int *altsize, void **dest, int size)
{
    // DEFINE_CALL(func, 0x005D8360, int, void **, int *, void **, int *, void **, int);
    // return func(source, ssize, alternate, altsize, dest, size);
    // captainslog_debug(__FUNCTION__);

    int out = 0;

    if (*ssize == 0) {
        *source = *alternate;
        *ssize = *altsize;
        *alternate = nullptr;
        *altsize = 0;
    }

    if (*source == nullptr || *ssize == 0) {
        return out;
    }

    int s = size;

    if (*ssize < size) {
        s = *ssize;
    }

    memcpy(*dest, *source, s);
    *source = Audio_Add_Long_To_Pointer(*source, s);
    *ssize -= s;
    *dest = Audio_Add_Long_To_Pointer(*dest, s);
    out = s;

    if ((size - s) == 0) {
        return out;
    }

    *source = *alternate;
    *ssize = *altsize;
    *alternate = nullptr;
    *altsize = 0;

    out = Simple_Copy(source, ssize, alternate, altsize, dest, (size - s)) + s;

    return out;
}

/**
 * Copies one buffer to another, decompressing if needed.
 * DEBUGGED
 */
int Sample_Copy(SampleTrackerType *st, void **source, int *ssize, void **alternate, int *altsize, void *dest, int size,
    SoundCompressType scomp, void *trailer, short *trailersize)
{
    // DEFINE_CALL(func,
    //   0x005D8440,
    //   int,
    //   SampleTrackerType *,
    //   void **,
    //   int *,
    //   void **,
    //   int *,
    //   void *,
    //   int,
    //   SoundCompressType,
    //   void *,
    //   short *);
    // return func(st, source, ssize, alternate, altsize, dest, size, scomp, trailer, trailersize);
    // captainslog_debug(__FUNCTION__);

    int datasize = 0;

    // There is no compression or it doesn't match any of the supported compressions so we just copy the data over.
    if (scomp == COMP_NONE || scomp != COMP_ZAP && scomp != COMP_ADPCM) {
        return Simple_Copy(source, ssize, alternate, altsize, &dest, size);
    }

    ADPCMStreamType *s = &st->m_StreamInfo;

    while (size > 0) {
        unsigned short fsize;
        unsigned short dsize;
        unsigned int magic;

        void *fptr = &fsize;
        void *dptr = &dsize;
        void *mptr = &magic;

        // Verify and seek over the chunk header.
        if (Simple_Copy(source, ssize, alternate, altsize, &fptr, sizeof(fsize)) < sizeof(fsize)) {
            break;
        }

        if (Simple_Copy(source, ssize, alternate, altsize, &dptr, sizeof(dsize)) < sizeof(dsize) || dsize > size) {
            break;
        }

        if (Simple_Copy(source, ssize, alternate, altsize, &mptr, sizeof(magic)) < sizeof(magic)
            || magic != g_LockedData.m_MagicNumber) {
            break;
        }

        if (fsize == dsize) {
            // File size matches size to decompress, so there's nothing to do other than copy the buffer over.
            if (Simple_Copy(source, ssize, alternate, altsize, &dest, fsize) < dsize) {
                return datasize;
            }
        } else {
            // Else we need to decompress it.
            void *uptr = g_LockedData.m_UncompBuffer;
            if (Simple_Copy(source, ssize, alternate, altsize, &uptr, fsize) < fsize) {
                return datasize;
            }

            if (scomp == COMP_ZAP) {
                Audio_Unzap(g_LockedData.m_UncompBuffer, dest, dsize);
            } else {
                s->m_Source = g_LockedData.m_UncompBuffer;
                s->m_Dest = dest;
                if (s->m_BitsPerSample == 16 && s->m_Channels == 1) {
                    ADPCM_Decompress(s, dsize);
                } else {
                    ADPCM_Decompress(s, dsize);
                }
            }

            dest = Audio_Add_Long_To_Pointer(dest, dsize);
        }

        datasize += dsize;
        size -= dsize;
    }

    return datasize;
}
