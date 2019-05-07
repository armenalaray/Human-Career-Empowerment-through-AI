#ifndef WIN32_EAGLEFLY_H

//TODO(Alex): Parse win32_eaglefly.cpp to know how many buttons are we procesing or 
//vice-versa parse win32_eaglefly.h to see button_key enum and add the corresponding 
//key stroke processing in win32_eaglefly.cpp?

//NOTE(Alex): Shall we assume that bitmap format is AA-RR-GG-BB?
struct win32_bitmap_buffer
{
    u16 BytesPerPixel;
    
    s32 Width;
    s32 Height;
    
    BITMAPINFO Info;
    void * Data;
};

struct win32_window_dimendion
{
    u32 Width;
    u32 Height;
};


struct win32_clamped_string
{
    u32 Count;
    char Buffer[2048];
};

struct win32_memory_block
{
    memory_index Size;
    void * Data;
};


struct win32_state
{
    HMODULE Handle;
    FILETIME LastWriteTime;
    
    ai_update_and_render * AIUpdateAndRender;
    b32 LoopIsPlaying;
    b32 IsRecording;
    
    HANDLE EFDMMap;
    win32_memory_block EFDViewBlock;
    
    HANDLE InputDataFileH;
    char * InputDataFullPath;
    u32 InputStructSize;
    
};

#define WIN32_EAGLEFLY_H
#endif







