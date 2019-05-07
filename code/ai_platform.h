#ifndef EAGLEFLY_PLATFORM_H

//NOTE(Alex): Remove Dependencies!
#include "stdio.h"
#include "stdint.h"

//TODO(Alex): Start filling this platform layer
#ifdef COMPILER_MSVC
#undef COMPILER_MSVC
#endif

#ifdef COMPILER_LLVM
#undef COMPILER_LLVM
#endif


#if _MSC_VER 
#define COMPILER_MSVC 1
#else
#define COMPILER_LLVM
#endif


#if defined(__cplusplus)
extern "C"{
#endif
    
#if COMPILER_MSVC
#include "float.h"
#include "limits.h"
    
#define MaxR64 DBL_MAX
#define MaxR32 DBL_MAX
#define MinR64 -DBL_MAX
#define MinR32 -FLT_MAX
    
#define MaxU64 _UI64_MAX
#define MaxU32 ULONG_MAX
#define MaxU16 USHRT_MAX
#define MaxU8 UCHAR_MAX
    
#define MaxS64 _I64_MAX
#define MinS64 _I64_MIN
#define MaxS32 LONG_MAX
#define MinS32 LONG_MIN
#define MaxS16 SHRT_MAX
#define MinS16 SHRT_MIN
#define MaxS8 SCHAR_MAX
#define MinS8 SCHAR_MIN
#endif
    
    typedef uint64_t u64;
    typedef int64_t s64;
    
    typedef uint32_t u32;
    typedef int32_t s32;
    typedef s32 b32;
    
    typedef uint16_t u16;
    typedef int16_t s16;
    
    typedef uint8_t u8;
    typedef int8_t s8;
    
    typedef float r32;
    typedef double r64;
    typedef size_t memory_index;
    
    typedef uintptr_t uptr;
    typedef intptr_t sptr;
    
    //TODO(Alex): Use intrinsic instead?
#define EXCHANGE_MAX_R64(Dest, Source) Dest = ((Dest) >= (Source)) ? (Dest) : (Source);
    
#define internal static 
#define global_variable static 
#define local_persist static 
    
#define BITMAP_BYTES_PER_PIXEL 4
#define Assert(Expression) if(!(Expression)){*(int*)(0) = 6;}
#define ArrayCount(Array) sizeof((Array)) / sizeof(Array[0]) 
    
#define Kilobytes(Size) ((Size) * 1024LL)
#define Megabytes(Size) (Kilobytes(Size) * 1024LL)
#define Gigabytes(Size) (Megabytes(Size) * 1024LL)
    
    
#define NUM_BUTTONS_PROCESSED  
#define MAX_NUM_FUNC_KEYS 12
    
    inline u32 TruncateR32ToU32(r32 Value)
    {
        u32 Result = (u32)Value;  
        return Result;
    }
    
    inline u32 RoundR32ToU32(r32 Value)
    {
        Assert(Value >= 0);
        u32 Result = (u32)(Value + 0.5f);  
        return Result;
    }
    
    inline s32 RoundR32ToS32(r32 Value)
    {
        s32 Result = (s32)(Value + 0.5f);  
        return Result;
    }
    
    inline u32 TruncateU64ToU32(u64 Value)
    {
        Assert(Value & 0xFFFFFFFF);
        u32 Result = (u32)Value;
        return Result;
    }
    
    enum debug_virtual_key
    {
        VKey_0,
        VKey_1,
        VKey_2,
        VKey_3,
        VKey_4,
        VKey_5,
        VKey_6,
        VKey_7,
        VKey_8,
        VKey_9,
        VKey_A,
        VKey_B,
        VKey_C,
        VKey_D,
        VKey_E,
        VKey_F,
        VKey_G,
        VKey_H,
        VKey_I,
        VKey_J,
        VKey_K,
        VKey_L,
        VKey_M,
        VKey_N,
        VKey_O,
        VKey_P,
        VKey_Q,
        VKey_R,
        VKey_S,
        VKey_T,
        VKey_U,
        VKey_V,
        VKey_W,
        VKey_X,
        VKey_Y,
        VKey_Z,
        
        VKey_F1,
        VKey_F2,
        VKey_F3,
        VKey_F4,
        VKey_F5,
        VKey_F6,
        VKey_F7,
        VKey_F8,
        VKey_F9,
        VKey_F10,
        VKey_F11,
        VKey_F12,
        
        VKey_OEM_1,
        VKey_OEM_2,
        VKey_OEM_3,
        VKey_OEM_4,
        VKey_OEM_5,
        VKey_OEM_6,
        VKey_OEM_7,
        VKey_OEM_8,
        VKey_OEM_102,
        
        VKey_OEM_PLUS,
        VKey_OEM_MINUS,
        VKey_OEM_COMMA,
        VKey_OEM_PERIOD,
        VKey_OEM_CLEAR,
        
        VKey_TAB,
        VKey_CAPITAL,
        VKey_SPACE,
        
        VKey_LEFT,
        VKey_UP,
        VKey_RIGHT,
        VKey_DOWN,
        
        VKey_PRIOR,
        VKey_NEXT,
        VKey_END,
        VKey_HOME,
        
        VKey_DELETE,
        VKey_BACK,
        VKey_Count,
    };
    
    enum debug_mouse_key
    {
        MouseKey_Left,
        MouseKey_Right,
        MouseKey_Middle,
        MouseKey_X1,
        MouseKey_X2,
        MouseKey_Count,
    };
    
    enum debug_control_key_flags
    {
        ControlKey_Shift = (1<<0),
        ControlKey_Alt = (1<<1),
        ControlKey_Ctrl = (1<<2),
    };
    
    struct debug_button_state
    {
        b32 ButtonEndedDown;
        u32 TransitionCount;
    };
    
    struct debug_keyboard_controller
    {
        debug_button_state Buttons[VKey_Count];
        debug_button_state FunctionKeys[MAX_NUM_FUNC_KEYS];
    };
    
    struct debug_input
    {
        debug_keyboard_controller Keyboard;
        debug_button_state MouseButtons[MouseKey_Count]; 
        r32 MouseX, MouseY, MouseZ;
        r32 WheelDelta;
        
        u8 ControlKeyFlags;
    };
    
    inline b32 ControlKeyIsToggled(debug_input * Input, debug_control_key_flags Flags)
    {
        b32 Result = (Input->ControlKeyFlags & Flags);
        return Result;
    }
    
    inline b32 WasPressed(debug_input *  Input, debug_virtual_key VKey)
    {
        b32 Result = false;
        
        debug_button_state BState = Input->Keyboard.Buttons[VKey];
        
        if((BState.TransitionCount > 1) || 
           ((BState.TransitionCount == 1)
            && BState.ButtonEndedDown))
        {
            Result = true;
        }
        
        return Result;
    }
    
    
    struct asset_bitmap
    {
        u32 BytesPerPixel;
        s32 Width;
        s32 Height;
        void * Data;
    };
    
    struct debug_memory;
    
#define AI_UPDATE_AND_RENDER(Name) void (Name)(asset_bitmap * FrameBuffer, debug_memory * Memory, debug_input * Input)
    typedef AI_UPDATE_AND_RENDER(ai_update_and_render);
    AI_UPDATE_AND_RENDER(AIUpdateAndRenderStub)
    {
        //NOTE(Alex): DebugUpdateAndRender not grabbed from DLL
    }
    
    struct debug_file_content
    {
        void * Content;
        u32 Size;
    };
    
#define READ_ENTIRE_FILE(Name) debug_file_content Name(char * FileName)
    typedef READ_ENTIRE_FILE(debug_read_entire_file);
    
#define WRITE_ENTIRE_FILE(Name) b32 Name(char * FileName, void * Data, u32 BytesToWrite)
    typedef WRITE_ENTIRE_FILE(debug_write_entire_file);
    
    struct debug_memory
    {
        memory_index AIStorageSize;
        void * AIStorage;
        
        memory_index TransientStorageSize;
        void * TransientStorage;
        
#if EAGLEFLY_INTERNAL
        debug_read_entire_file * DebugReadEntireFile;
        debug_write_entire_file * DebugWriteEntireFile;
#endif
    };
    
#if defined(__cplusplus)
}
#endif


#define EAGLEFLY_PLATFORM_H
#endif

