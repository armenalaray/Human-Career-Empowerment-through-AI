
#include "Windows.h"

#include "ai_platform.h"
#include "win32_ai.h"

global_variable b32 GlobalRunning = true;
global_variable win32_bitmap_buffer Win32GlobalBackBuffer;
//TODO(Alex): Do we want GlobalInput? 

struct win32_memory_arena
{
    void * Base;
    u32 Size;
    u32 Used;
};

internal void * 
Win32AllocateMemory(memory_index Size)
{
    void * Result = 0;
    //TODO(Alex): Alignment?
    //Assert(IsPower2(Alignment));
    if(Size)
    {
        Result = VirtualAlloc(0, Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE); 
    }
    
    return Result;
}

internal void
Win32DeallocateMemory(void * Memory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

WRITE_ENTIRE_FILE(Win32WriteEntireFile)
{
    b32 Result = false;
    HANDLE Handle = CreateFileA(FileName,
                                GENERIC_WRITE,
                                FILE_SHARE_WRITE,
                                0,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                0);
    if(Handle)
    {
        DWORD BytesWritten = 0;
        if(WriteFile(Handle, Data, BytesToWrite, &BytesWritten, 0))
        {
            Result = true;
        }
        else
        {
            //TODO(Alex): Logging
        }
        
        CloseHandle(Handle);
    }
    else
    {
        //TODO(Alex): Check if ERROR_FILE_NOT_FOUND 
    }
    
    return Result;
}


READ_ENTIRE_FILE(Win32ReadEntireFile)
{
    debug_file_content Result = {};
    HANDLE Handle = CreateFileA(FileName,
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                0,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                0);
    if(Handle)
    {
        LARGE_INTEGER U64Size = {};
        if(GetFileSizeEx(Handle, &U64Size))
        {
            u32 U32Size = TruncateU64ToU32(U64Size.QuadPart);
            DWORD BytesRead = 0;
            
            void * Content = Win32AllocateMemory(U32Size);
            if(ReadFile(Handle, Content, U32Size, &BytesRead,0) && 
               (BytesRead == U32Size))
            {
                Result.Content = Content;
                Result.Size = U32Size;
            }
            else
            {
                Win32DeallocateMemory(Content);
            }
        }
        else
        {
            //TODO(Alex): logging
        }
        
        CloseHandle(Handle);
    }
    else
    {
        //TODO(Alex): Check if ERROR_FILE_NOT_FOUND 
    }
    
    return Result;
}



internal void
Win32ResizeDIBSection(win32_bitmap_buffer * Buffer, s32 Width, s32 Height)
{
    if(Buffer->Data)
    {
        Win32DeallocateMemory(Buffer->Data);
        Buffer->Data = 0;
    }
    
    Buffer->Width = Width;
    Buffer->Height = (Height >= 0) ? Height : -Height;
    
    Buffer->Data = Win32AllocateMemory(Buffer->Width * Buffer->Height * BITMAP_BYTES_PER_PIXEL);
    Buffer->BytesPerPixel = 32;
    Buffer->Info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    Buffer->Info.bmiHeader.biWidth = Width;
    Buffer->Info.bmiHeader.biHeight = Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = Buffer->BytesPerPixel;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    Buffer->Info.bmiHeader.biSizeImage = 0;
    Buffer->Info.bmiHeader.biXPelsPerMeter = 0;
    Buffer->Info.bmiHeader.biYPelsPerMeter = 0;
    Buffer->Info.bmiHeader.biClrUsed = 0;
    Buffer->Info.bmiHeader.biClrImportant = 0;
}

internal void
Win32UpdateWindow(HDC DestDC, win32_window_dimendion DestDim, win32_bitmap_buffer * Buffer)
{
    //NOTE(Alex):Return value from StretchDIBits can be negative for mirrored content
    //TODO(Alex): Check if SRCCOPY copies without stretching at all
    //TODO(Alex): Full screen mode
    
    u32 OffsetX = 0;
    u32 OffsetY = 0;
    
    int ScanLinesCopied = StretchDIBits(DestDC,
                                        OffsetX,
                                        OffsetY,
                                        Buffer->Width,
                                        Buffer->Height,
                                        0,
                                        0,
                                        Buffer->Width,
                                        Buffer->Height,
                                        Buffer->Data,
                                        &Buffer->Info,
                                        DIB_RGB_COLORS,
                                        SRCCOPY);
    
}

internal win32_window_dimendion
Win32GetWindowDimension(HWND Window)
{
    win32_window_dimendion Result = {};
    RECT ClientRect = {};
    GetClientRect(Window, &ClientRect);
    
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;
    return Result;
};

internal void 
ConcatStrings(char * SourceA, u32 CountA, 
              char * SourceB, u32 CountB, 
              char * Dest, u32 CountDest)
{
    Assert(CountA  + CountB < CountDest);
    
    for(u32 Index = 0; 
        Index < CountA;
        ++Index)
    {
        *Dest++ = *SourceA++;
    }
    
    for(u32 Index = CountA; 
        Index < CountA + CountB;
        ++Index)
    {
        *Dest++ = *SourceB++;
    }
    
    //NOTE(Alex): Add Null terminator
    *Dest++ = 0;
}

inline u32 
StringLength(char * String)
{
    u32 Result = 0;
    while(*String++)
    {
        ++Result;
    }
    
    return Result;
}

inline b32
Win32IsValidHandle(HANDLE Handle)
{
    b32 Result = ((Handle != INVALID_HANDLE_VALUE) 
                  && (Handle != 0));
    return Result;
}

internal void
Win32ProcessInputMessage(debug_button_state * FirstButtonState,
                         b32 IsDown, 
                         u32 Key)
{
    Assert(FirstButtonState);
    debug_button_state * ButtonState = (FirstButtonState + Key);
    if(ButtonState->ButtonEndedDown != IsDown)
    {
        ++ButtonState->TransitionCount;
        ButtonState->ButtonEndedDown = IsDown;
    }
}

internal void
Win32ProcessKeyboardMessage(debug_input * Input, b32 IsDown, debug_virtual_key Key)
{
    Win32ProcessInputMessage(Input->Keyboard.Buttons, IsDown, Key);
}

internal void
Win32ProcessMouseMessage(debug_input * Input, b32 IsDown, debug_mouse_key Key)
{
    Win32ProcessInputMessage(Input->MouseButtons, IsDown, Key);
}

internal void
Win32AddControlKeyFlag(debug_input * Input, b32 IsDown, debug_control_key_flags ControlFlags)
{
    if(IsDown)
    {
        Input->ControlKeyFlags |= ControlFlags;
    }
    else
    {
        u8 Mask = (u8)(~ControlFlags);
        Input->ControlKeyFlags &= Mask; 
    }
}

internal LRESULT CALLBACK
Win32WindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;
    switch(Message)
    {
        case WM_QUIT:
        {
            GlobalRunning = false;
        }break;
        case WM_CREATE:
        {
            RAWINPUTDEVICE Devs[2];
            Devs[0].usUsagePage = 1;
            Devs[0].usUsage = 6;
            Devs[0].dwFlags = 0;//NOTE(Alex): Disables standard WM_APPCOMMAND input data
            Devs[0].hwndTarget = Window;
            
            Devs[1].usUsagePage = 1;
            Devs[1].usUsage = 2;
            Devs[1].dwFlags = 0;//NOTE(Alex): Disables standard WM_APPCOMMAND input data
            Devs[1].hwndTarget = Window;
            
            RegisterRawInputDevices(Devs, 2, sizeof(Devs[0]));
            
        }break;
        case WM_DESTROY:
        {
            RAWINPUTDEVICE Devs[2];
            Devs[0].usUsagePage = 1;
            Devs[0].usUsage = 6;
            Devs[0].dwFlags = RIDEV_REMOVE;
            Devs[0].hwndTarget = Window;
            
            Devs[1].usUsagePage = 1;
            Devs[1].usUsage = 2;
            Devs[1].dwFlags = RIDEV_REMOVE;
            Devs[1].hwndTarget = Window;
            
            RegisterRawInputDevices(Devs, 2, sizeof(Devs[0]));
            
            GlobalRunning = false;
        }break;
        
#if 0        
        case WM_ACTIVATE:
        case WM_ACTIVATEAPP:
        {
            
        }break;
#endif
        
        case WM_PAINT:
        {
            win32_window_dimendion WindowDim = Win32GetWindowDimension(Window);
            PAINTSTRUCT PaintStruct = {};
            HDC DeviceContext =  BeginPaint(Window, &PaintStruct);
            Win32UpdateWindow(DeviceContext, WindowDim, &Win32GlobalBackBuffer);
            EndPaint(Window, &PaintStruct);
        }break;
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        }break;
        
    }
    
    return (Result);
}

//TODO(Alex): Shall we expand these to multiple input files?
internal void
Win32StartRecording(win32_state * Win32State, 
                    win32_memory_block * DebugBlock)
{
    if(!Win32State->IsRecording && 
       DebugBlock->Data)
    {
        Assert(!Win32IsValidHandle(Win32State->InputDataFileH));
        
        Win32State->InputDataFileH = CreateFileA(Win32State->InputDataFullPath,
                                                 GENERIC_READ|GENERIC_WRITE,
                                                 FILE_SHARE_READ,
                                                 0,
                                                 CREATE_ALWAYS,
                                                 FILE_ATTRIBUTE_NORMAL,
                                                 0);
        
        CopyMemory(Win32State->EFDViewBlock.Data, DebugBlock->Data, DebugBlock->Size);
        Win32State->IsRecording = true;
    }
}

internal void
Win32EndRecording(win32_state * Win32State)
{
    if(Win32State->IsRecording)
    {
        Win32State->IsRecording = false;
    }
}

internal void
Win32StartPlaying(win32_state * Win32State, 
                  win32_memory_block * DebugBlock)
{
    Assert(!Win32State->IsRecording);
    
    if(DebugBlock->Data)
    {
        Win32State->LoopIsPlaying = true;
        CopyMemory(DebugBlock->Data, Win32State->EFDViewBlock.Data, Win32State->EFDViewBlock.Size);
    }
}

internal void
Win32EndPlaying(win32_state * Win32State)
{
    if(Win32State->LoopIsPlaying)
    {
        Assert(Win32IsValidHandle(Win32State->InputDataFileH));
        
        CloseHandle(Win32State->InputDataFileH);
        Win32State->InputDataFileH = 0;
        
        DeleteFileA(Win32State->InputDataFullPath);
        Win32State->LoopIsPlaying = false;
    }
}


internal b32 
Win32WriteInputRecord(win32_state * Win32State, 
                      debug_input * DebugInput)
{
    b32 Result = false;
    if(Win32State->InputDataFileH)
    {
        DWORD BytesWritten = 0;
        if(WriteFile(Win32State->InputDataFileH,
                     DebugInput,
                     Win32State->InputStructSize,
                     &BytesWritten,
                     0) && (BytesWritten == Win32State->InputStructSize))
        {
            Result = true;
        }
    }
    
    return Result;
}

internal void 
Win32ReadInputRecord(win32_state * Win32State, 
                     win32_memory_block * DebugBlock,
                     debug_input * Input)
{
    if(Win32State->InputDataFileH)
    {
        debug_input Temp = {};
        DWORD BytesRead = 0;
        if (ReadFile(Win32State->InputDataFileH,
                     &Temp,
                     Win32State->InputStructSize,
                     &BytesRead,
                     0) && (BytesRead == Win32State->InputStructSize))
        {
            *Input = Temp;
        }
        else
        {
            CloseHandle(Win32State->InputDataFileH);
            Win32State->InputDataFileH = CreateFileA(Win32State->InputDataFullPath,
                                                     GENERIC_READ|GENERIC_WRITE,
                                                     FILE_SHARE_READ,
                                                     0,
                                                     OPEN_EXISTING,
                                                     FILE_ATTRIBUTE_NORMAL,
                                                     0);
            if(Win32State->InputDataFileH)
            {
                if (ReadFile(Win32State->InputDataFileH,
                             &Temp,
                             Win32State->InputStructSize,
                             &BytesRead,
                             0) && (BytesRead == Win32State->InputStructSize))
                {
                    *Input = Temp;
                }
            }
            
            Win32StartPlaying(Win32State, DebugBlock);
        }
    }
}


internal void 
Win32ProcessMessages(HWND Window, 
                     debug_input * Input)
{
    MSG Message = {};
    b32 ContinueProcessingMessages = true;
    while(PeekMessage(&Message,Window,0,0,PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_MOUSEWHEEL:
            {
                //TODO(Alex): Do we want more granularity on wheel delta?
                Input->WheelDelta = (r32)(Message.wParam >> 16);
            }break;
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP:
            case WM_SYSKEYDOWN:
            {
                u32 VirtualKey = (u32)Message.wParam;
                u32 Flags = (u32)Message.lParam;
                
                b32 AltWasPressed = (Flags & (1 << 29)); 
                b32 WasDownBefore = ((Flags & (1 << 30)) != 0);  
                b32 IsDown = ((Flags & (1 << 31)) == 0);  
                
                if(WasDownBefore != IsDown)
                {
                    switch(VirtualKey)
                    {
                        
                        case VK_ADD:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_0);
                        }break;
                        case VK_MULTIPLY:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_0);
                        }break;
                        //------------------------------------------------------------------
                        //NOTE(Alex): Numeric Keys
                        
                        case 0x30://0 key
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_0);
                        }break;
                        case 0x31://1 key
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_1);
                        }break;
                        case 0x32://0 key
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_2);
                        }break;
                        case 0x33://0 key
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_3);
                        }break;
                        case 0x34://0 key
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_4);
                        }break;
                        case 0x35://0 key
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_5);
                        }break;
                        case 0x36://0 key
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_6);
                        }break;
                        case 0x37://0 key
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_7);
                        }break;
                        case 0x38://9 key
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_8);
                        }break;
                        case 0x39://9 key
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_9);
                        }break;
                        
                        //------------------------------------------------------------------
                        //NOTE(Alex): Alpha Keys
                        case 0x41:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_A);
                        }break;
                        case 0x42:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_B);
                        }break;
                        case 0x43:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_C);
                        }break;
                        case 0x44:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_D);
                        }break;
                        case 0x45:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_E);
                        }break;
                        case 0x46:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_F);
                        }break;
                        case 0x47:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_G);
                        }break;
                        case 0x48:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_H);
                        }break;
                        case 0x49:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_I);
                        }break;
                        case 0x4A:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_J);
                        }break;
                        case 0x4B:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_K);
                        }break;
                        case 0x4C:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_L);
                        }break;
                        case 0x4D:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_M);
                        }break;
                        case 0x4E:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_N);
                        }break;
                        case 0x4F:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_O);
                        }break;
                        case 0x50:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_P);
                        }break;
                        case 0x51:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_Q);
                        }break;
                        case 0x52:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_R);
                        }break;
                        case 0x53:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_S);
                        }break;
                        case 0x54:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_T);
                        }break;
                        case 0x55:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_U);
                        }break;
                        case 0x56:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_V);
                        }break;
                        case 0x57:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_W);
                        }break;
                        case 0x58:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_X);
                        }break;
                        case 0x59:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown,  VKey_Y);
                        }break;
                        case 0x5A:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown, VKey_Z);
                        }break;
                        
                        //------------------------------------------------------------------
                        //NOTE(Alex): Function Keys
                        case VK_F1:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown, VKey_F1);
                        }break;
                        case VK_F2:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown, VKey_F2);
                        }break;
                        case VK_F3:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown, VKey_F3);
                        }break;
                        case VK_F4:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown, VKey_F4);
                        }break;
                        case VK_F5:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown, VKey_F5);
                        }break;
                        case VK_F6:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown, VKey_F6);
                        }break;
                        case VK_F7:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown, VKey_F7);
                        }break;
                        case VK_F8:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown, VKey_F8);
                        }break;
                        case VK_F9:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown, VKey_F9);
                        }break;
                        case VK_F10:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown, VKey_F10);
                        }break;
                        case VK_F11:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown, VKey_F11);
                        }break;
                        case VK_F12:
                        {
                            Win32ProcessKeyboardMessage(Input, IsDown, VKey_F12);
                        }break;
                        
                        //------------------------------------------------------------------
                        //NOTE(Alex): Control Keys
                        case VK_CONTROL:
                        {
                            Win32AddControlKeyFlag(Input, IsDown, ControlKey_Ctrl);
                        }break;
                        case VK_MENU:
                        {
                            Win32AddControlKeyFlag(Input, IsDown, ControlKey_Alt);
                        }break;
                        case VK_SHIFT:
                        {
                            Win32AddControlKeyFlag(Input, IsDown, ControlKey_Shift);
                        }break;
                    }
                    
                    if(AltWasPressed && (VirtualKey == VK_F4))
                    {
                        GlobalRunning = false;
                    }
                }
            }break;
            default:
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            }break;
        }
    }
}

inline b32
Win32FileTimesAreDifferent(FILETIME TimeA, FILETIME TimeB)
{
    b32 Result = ((TimeA.dwLowDateTime != TimeB.dwLowDateTime) ||
                  (TimeA.dwHighDateTime != TimeB.dwHighDateTime));
    return Result;
}

internal FILETIME
Win32GetFileLastWriteTime(char * FileName)
{
    FILETIME Result = {};
    WIN32_FILE_ATTRIBUTE_DATA FileData = {};
    
    if(GetFileAttributesEx(FileName,
                           GetFileExInfoStandard,
                           &FileData))
    {
        Result = FileData.ftLastWriteTime;  
    }
    
    return Result;
}

internal void
Win32UnloadDLL(win32_state * Win32State)
{
    if(Win32State->Handle)
    {
        FreeLibrary(Win32State->Handle);
        Win32State->Handle = 0;
    }
    
    Win32State->AIUpdateAndRender = 0;
}

internal void
Win32LoadDLL(win32_state * Win32State, char * DLLFullPath, char * TempDLLFullPath, char * LockFullPath)
{
    WIN32_FILE_ATTRIBUTE_DATA Ignored = {};
    if(!GetFileAttributesEx(LockFullPath,
                            GetFileExInfoStandard,
                            &Ignored))
    {
        Win32State->LastWriteTime = Win32GetFileLastWriteTime(DLLFullPath);
        
        CopyFile(DLLFullPath, TempDLLFullPath, FALSE);
        Win32State->Handle = LoadLibraryA(TempDLLFullPath);
        if(Win32State->Handle)
        {
            Win32State->AIUpdateAndRender = (ai_update_and_render*)GetProcAddress(Win32State->Handle, "AIUpdateAndRender");
        }
    }
    else
    {
        int x = 5;
    }
}

internal win32_clamped_string
Win32GetEXEPath()
{
    win32_clamped_string Result = {};
    win32_clamped_string ModuleFileName = {};
    
    DWORD CharCount = GetModuleFileNameA(0, ModuleFileName.Buffer, ArrayCount(ModuleFileName.Buffer));
    
    if(CharCount)
    {
        
        char * Char = ModuleFileName.Buffer + (CharCount - 1);
        for(;
            --Char;
            )
        {
            --CharCount;
            if((Char == ModuleFileName.Buffer) || 
               (*Char == '\\'))
            {
                break;
            }
        }
        
        for(u32 Index = 0;
            Index < CharCount;
            ++Index)
        {
            Result.Buffer[Index] = ModuleFileName.Buffer[Index];
        }
        
        Result.Count = CharCount;
    }
    
    return Result;
}

int WinMain(
HINSTANCE hInstance,
HINSTANCE hPrevInstance,
LPSTR     lpCmdLine,
int       nCmdShow)
{
    win32_clamped_string EXEPath = Win32GetEXEPath();
    
    char TempDLLFullPath[1024] = {0};
    ConcatStrings(EXEPath.Buffer, EXEPath.Count, 
                  "ai_temp.dll", StringLength("ai_temp.dll"), 
                  TempDLLFullPath, ArrayCount(TempDLLFullPath));
    
    char DLLFullPath[1024] = {0};
    ConcatStrings(EXEPath.Buffer, EXEPath.Count, 
                  "ai_main.dll", StringLength("ai_main.dll"), 
                  DLLFullPath, ArrayCount(DLLFullPath));
    
    char LockFullPath[1024] = {0};
    ConcatStrings(EXEPath.Buffer, EXEPath.Count, 
                  "lock.tmp", StringLength("lock.tmp"), 
                  LockFullPath, ArrayCount(LockFullPath));
    
    //TODO(Alex): Do we want more eaglefly_data_files?
    char DebugDataFullPath[1024] = {0};
    ConcatStrings(EXEPath.Buffer, EXEPath.Count, 
                  "ai_data_0.aid", StringLength("ai_data_0.aid"),
                  DebugDataFullPath, ArrayCount(DebugDataFullPath));
    
    char InputDataFullPath[1024] = {0};
    ConcatStrings(EXEPath.Buffer, EXEPath.Count, 
                  "ai_input_0.aid", StringLength("ai_input_0.aid"),
                  InputDataFullPath, ArrayCount(InputDataFullPath));
    
    
    char * WindowClassName = "AIClassName";
    char * WindowTitle = "AIEduscore";
    
    //Win32ResizeDIBSection(Window, &Win32GlobalBackBuffer, 1920, -1080);
    Win32ResizeDIBSection(&Win32GlobalBackBuffer, 960, 540);
    
    WNDCLASSA WindowClass = {};
    
    WindowClass.style = CS_HREDRAW|CS_VREDRAW;//|CS_OWNDC,
    WindowClass.lpfnWndProc = Win32WindowCallback;
    WindowClass.hInstance = hInstance;
    //TODO(Alex): Add an icon!
    WindowClass.hCursor = LoadCursor(0,IDC_ARROW); 
    WindowClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
    WindowClass.lpszClassName = WindowClassName;
    
    ATOM ClassID = RegisterClassA(&WindowClass);
    if(ClassID)
    {
        HWND Window = CreateWindowExA(0,
                                      WindowClassName,
                                      WindowTitle,
                                      WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      0,
                                      0, 
                                      hInstance,
                                      0);
        
        
        if(Window)
        {
            //TODO(Alex): Stable 30 - 60 fps
            //TODO(Alex): Adjust Canvas Rect
            win32_state Win32State = {};
            SYSTEM_INFO SystemInfo = {};
            GetSystemInfo(&SystemInfo);
            
            debug_memory AIMemory = {};
            AIMemory.TransientStorageSize = Megabytes(500);
            AIMemory.AIStorageSize = Gigabytes(1);
            memory_index TotalStorageSize = AIMemory.TransientStorageSize + AIMemory.AIStorageSize;
            AIMemory.AIStorage = Win32AllocateMemory(TotalStorageSize);
            AIMemory.TransientStorage = (char*)AIMemory.AIStorage + AIMemory.AIStorageSize; 
            
            AIMemory.DebugReadEntireFile = &Win32ReadEntireFile;
            AIMemory.DebugWriteEntireFile = &Win32WriteEntireFile;
            
            win32_memory_block AIMemoryBlock = {TotalStorageSize, AIMemory.AIStorage}; 
            if(AIMemory.AIStorage)
            {
                HANDLE DebugDataFileH = CreateFileA(DebugDataFullPath,
                                                    GENERIC_READ|GENERIC_WRITE,
                                                    FILE_SHARE_READ,
                                                    0,
                                                    CREATE_ALWAYS,
                                                    FILE_ATTRIBUTE_NORMAL,
                                                    0);
                
                
                if(DebugDataFileH)
                {
                    u32 TotalStorageSizeU32 = TruncateU64ToU32(TotalStorageSize);
                    Win32State.EFDMMap = CreateFileMappingA(DebugDataFileH,
                                                            0,
                                                            PAGE_READWRITE,
                                                            0,
                                                            TotalStorageSizeU32,
                                                            0);
                    
                    
                    if(Win32State.EFDMMap)
                    {
                        Win32State.EFDViewBlock.Data = MapViewOfFile(Win32State.EFDMMap,
                                                                     FILE_MAP_ALL_ACCESS,
                                                                     0,
                                                                     0,
                                                                     0);
                        Win32State.EFDViewBlock.Size = TotalStorageSize; 
                        Win32State.InputStructSize = sizeof(debug_input); 
                        Win32State.InputDataFullPath = InputDataFullPath; 
                    }
                    else
                    {
                        Win32State.EFDMMap = 0;
                        Win32State.InputDataFileH = 0;
                    }
                    
                    
                    CloseHandle(DebugDataFileH);
                }
                
                
                debug_input  DebugInput[2] = {0};
                debug_input * LastInput = &DebugInput[0];
                debug_input * NewInput = &DebugInput[1];
                
                debug_input LoopInput = {};
                
                Win32LoadDLL(&Win32State, DLLFullPath, TempDLLFullPath, LockFullPath);
                
                while(GlobalRunning)
                {
                    debug_keyboard_controller * LastKeyboard = &LastInput->Keyboard;
                    debug_keyboard_controller * NewKeyboard = &NewInput->Keyboard;
                    
                    
                    for(u32 KeyIndex = 0;
                        KeyIndex < VKey_Count;
                        ++KeyIndex)
                    {
                        debug_button_state * NewState = &NewKeyboard->Buttons[KeyIndex];
                        debug_button_state * LastState = &LastKeyboard->Buttons[KeyIndex];
                        
                        NewState->TransitionCount = 0;
                        NewState->ButtonEndedDown = LastState->ButtonEndedDown;
                    }
                    
                    Win32ProcessMessages(Window, NewInput);
                    
                    if(ControlKeyIsToggled(NewInput, ControlKey_Ctrl))
                    {
                        if(WasPressed(NewInput, VKey_L))
                        {
                            if(!Win32State.LoopIsPlaying)
                            {
                                if(!Win32State.IsRecording)
                                {
                                    Win32StartRecording(&Win32State, &AIMemoryBlock);
                                }
                                else
                                {
                                    Win32EndRecording(&Win32State);
                                    Win32StartPlaying(&Win32State, &AIMemoryBlock);
                                }
                            }
                            else
                            {
                                Win32EndPlaying(&Win32State);
                            }
                        }
                    }
                    
                    
                    DWORD VirtualKeys[] = 
                    {
                        VK_LBUTTON,
                        VK_RBUTTON,
                        VK_MBUTTON,
                        VK_XBUTTON1,
                        VK_XBUTTON2,
                    };
                    
                    for(u32 KeyIndex = 0;
                        KeyIndex < MouseKey_Count;
                        ++KeyIndex)
                    {
                        debug_button_state * NewState = &NewInput->MouseButtons[KeyIndex];
                        debug_button_state * LastState = &LastInput->MouseButtons[KeyIndex];
                        
                        NewState->TransitionCount = 0;
                        NewState->ButtonEndedDown = LastState->ButtonEndedDown;
                        
                        u16 Flags = GetKeyState(VirtualKeys[KeyIndex]);
                        b32 IsDown = (Flags >> 15); 
                        
                        Win32ProcessMouseMessage(NewInput, IsDown, (debug_mouse_key)KeyIndex);
                    }
                    
                    POINT MousePos = {};
                    GetCursorPos(&MousePos);
                    //TODO(Alex): Transform this according to canvas
                    NewInput->MouseX = (r32)MousePos.x;
                    NewInput->MouseY = (r32)MousePos.y;
                    
                    if(Win32State.IsRecording)
                    {
                        Win32WriteInputRecord(&Win32State, NewInput);
                    }
                    if(Win32State.LoopIsPlaying)
                    {
                        Win32ReadInputRecord(&Win32State, &AIMemoryBlock, &LoopInput);
                    }
                    
                    
                    FILETIME LastWriteTime = Win32GetFileLastWriteTime(DLLFullPath);
                    if(CompareFileTime(&LastWriteTime, &Win32State.LastWriteTime) != 0)
                    {
                        Win32UnloadDLL(&Win32State);
                        Win32LoadDLL(&Win32State, DLLFullPath, TempDLLFullPath, LockFullPath);
                    }
                    
                    if(Win32State.AIUpdateAndRender)
                    {
                        asset_bitmap DebugBuffer = {};
                        DebugBuffer.BytesPerPixel = Win32GlobalBackBuffer.BytesPerPixel;
                        DebugBuffer.Width = Win32GlobalBackBuffer.Width;
                        DebugBuffer.Height = Win32GlobalBackBuffer.Height;
                        DebugBuffer.Data = Win32GlobalBackBuffer.Data;
                        Win32State.AIUpdateAndRender(&DebugBuffer, &AIMemory, 
                                                     (Win32State.LoopIsPlaying) ? &LoopInput : NewInput);
                    }
                    
                    
                    HDC WindowDC = GetDC(Window);
                    //win32_window_dimendion WindowDim = Win32GetWindowDimension(Window);
                    RECT ClientRect = {};
                    GetClientRect(Window, &ClientRect);
                    InvalidateRect(Window, &ClientRect, FALSE);
                    UpdateWindow(Window);
                    ReleaseDC(Window, WindowDC);
                    //Win32UpdateWindow(WindowDC, WindowDim, &Win32GlobalBackBuffer);
                    
                    debug_input * Temp = LastInput;
                    LastInput = NewInput;
                    NewInput = Temp;
                }
            }
        }
        else
        {
            OutputDebugString("Error registering the WindowClass");
        }
    }
    
    return 0;
}


#if 0
case WM_INPUT:
{
    //TODO(Alex): Handle 
    u32 DataSize = 0;
    GetRawInputData((HRAWINPUT)Message.lParam,  
                    RID_INPUT,
                    0,
                    &DataSize,
                    sizeof(RAWINPUTHEADER));
    
    RAWINPUT * RawInput = (RAWINPUT *)malloc(DataSize);
    
    GetRawInputData((HRAWINPUT)Message.lParam,  
                    RID_INPUT,
                    RawInput,
                    &DataSize,
                    sizeof(RAWINPUTHEADER));
    
    u32 MinSize = 0; 
    s32 BytesCopied0 = GetRawInputDeviceInfoA(RawInput->header.hDevice, 
                                              RIDI_DEVICEINFO, 
                                              0, 
                                              &MinSize);
    
    RID_DEVICE_INFO * DeviceInfo = (RID_DEVICE_INFO *)malloc(MinSize);
    DeviceInfo->cbSize = sizeof(RID_DEVICE_INFO);
    s32 BytesCopied1 = GetRawInputDeviceInfoA(RawInput->header.hDevice, 
                                              RIDI_DEVICEINFO, 
                                              DeviceInfo, 
                                              &MinSize);
    
    
    if(RawInput->header.dwType == RIM_TYPEKEYBOARD)
    {
        b32 IsDown = !(RawInput->data.keyboard.Flags & RI_KEY_BREAK);
        switch(RawInput->data.keyboard.VKey)
        {
            case VK_ADD:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_0);
            }break;
            case VK_MULTIPLY:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_0);
            }break;
            
            //------------------------------------------------------------------
            //NOTE(Alex): Numeric Keys
            
            case 0x30://0 key
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_0);
            }break;
            case 0x31://1 key
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_1);
            }break;
            case 0x32://0 key
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_2);
            }break;
            case 0x33://0 key
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_3);
            }break;
            case 0x34://0 key
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_4);
            }break;
            case 0x35://0 key
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_5);
            }break;
            case 0x36://0 key
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_6);
            }break;
            case 0x37://0 key
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_7);
            }break;
            case 0x38://9 key
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_8);
            }break;
            case 0x39://9 key
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_9);
            }break;
            
            //------------------------------------------------------------------
            //NOTE(Alex): Alpha Keys
            case 0x41:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_A);
            }break;
            case 0x42:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_B);
            }break;
            case 0x43:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_C);
            }break;
            case 0x44:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_D);
            }break;
            case 0x45:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_E);
            }break;
            case 0x46:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_F);
            }break;
            case 0x47:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_G);
            }break;
            case 0x48:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_H);
            }break;
            case 0x49:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_I);
            }break;
            case 0x4A:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_J);
            }break;
            case 0x4B:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_K);
            }break;
            case 0x4C:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_L);
            }break;
            case 0x4D:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_M);
            }break;
            case 0x4E:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_N);
            }break;
            case 0x4F:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_O);
            }break;
            case 0x50:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_P);
            }break;
            case 0x51:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_Q);
            }break;
            case 0x52:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_R);
            }break;
            case 0x53:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_S);
            }break;
            case 0x54:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_T);
            }break;
            case 0x55:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_U);
            }break;
            case 0x56:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_V);
            }break;
            case 0x57:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_W);
            }break;
            case 0x58:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_X);
            }break;
            case 0x59:
            {
                Win32ProcessKeyboardMessage(Input, IsDown,  VKey_Y);
            }break;
            case 0x5A:
            {
                Win32ProcessKeyboardMessage(Input, IsDown, VKey_Z);
            }break;
            
            //------------------------------------------------------------------
            //NOTE(Alex): Function Keys
            case VK_F1:
            {
                Win32ProcessKeyboardMessage(Input, IsDown, VKey_F1);
            }break;
            case VK_F2:
            {
                Win32ProcessKeyboardMessage(Input, IsDown, VKey_F2);
            }break;
            case VK_F3:
            {
                Win32ProcessKeyboardMessage(Input, IsDown, VKey_F3);
            }break;
            case VK_F4:
            {
                Win32ProcessKeyboardMessage(Input, IsDown, VKey_F4);
            }break;
            case VK_F5:
            {
                Win32ProcessKeyboardMessage(Input, IsDown, VKey_F5);
            }break;
            case VK_F6:
            {
                Win32ProcessKeyboardMessage(Input, IsDown, VKey_F6);
            }break;
            case VK_F7:
            {
                Win32ProcessKeyboardMessage(Input, IsDown, VKey_F7);
            }break;
            case VK_F8:
            {
                Win32ProcessKeyboardMessage(Input, IsDown, VKey_F8);
            }break;
            case VK_F9:
            {
                Win32ProcessKeyboardMessage(Input, IsDown, VKey_F9);
            }break;
            case VK_F10:
            {
                Win32ProcessKeyboardMessage(Input, IsDown, VKey_F10);
            }break;
            case VK_F11:
            {
                Win32ProcessKeyboardMessage(Input, IsDown, VKey_F11);
            }break;
            case VK_F12:
            {
                Win32ProcessKeyboardMessage(Input, IsDown, VKey_F12);
            }break;
            
            //------------------------------------------------------------------
            //NOTE(Alex): Control Keys
            case VK_CONTROL:
            {
                Win32AddControlKeyFlag(Input, IsDown, ControlKey_Ctrl);
            }break;
            case VK_MENU:
            {
                Win32AddControlKeyFlag(Input, IsDown, ControlKey_Alt);
            }break;
            case VK_SHIFT:
            {
                Win32AddControlKeyFlag(Input, IsDown, ControlKey_Shift);
            }break;
            
            default:
            {
                DefRawInputProc(&RawInput, 1, sizeof(RAWINPUTHEADER));
            }break;
        }
        
        if(ControlKeyIsToggled(Input, ControlKey_Alt) && WasPressed(Input, VKey_F4))
        {
            GlobalRunning = false;
        }
        
        
        win32_clamped_string Prefix = {};
        if(RawInput->data.keyboard.Flags & RI_KEY_E0)
        {
            u32 TextBCount = StringLength("E0_");
            ConcatStrings(Prefix.Buffer, Prefix.Count, 
                          "E0_", TextBCount, 
                          Prefix.Buffer, sizeof(Prefix));
            Prefix.Count += TextBCount;
        }
        
        if(RawInput->data.keyboard.Flags & RI_KEY_E1)
        {
            u32 TextBCount = StringLength("E1_");
            ConcatStrings(Prefix.Buffer, Prefix.Count, 
                          "E1_", TextBCount, 
                          Prefix.Buffer, sizeof(Prefix));
            Prefix.Count += TextBCount;
        }
        
        TCHAR OutputBuffer[256];
        _snprintf_s(OutputBuffer, ArrayCount(OutputBuffer),
                    "%p, msg=%04x, vk=%04x, scanCode=%.*s%02x, %s\n",
                    RawInput->header.hDevice,
                    RawInput->data.keyboard.Message,
                    RawInput->data.keyboard.VKey,
                    Prefix.Count,
                    Prefix.Buffer,
                    RawInput->data.keyboard.MakeCode,
                    (RawInput->data.keyboard.Flags & RI_KEY_BREAK) ? "release" : "press");
        
        OutputDebugStringA(OutputBuffer);
        
    }
    else if(RawInput->header.dwType == RIM_TYPEMOUSE)
    {
        u16 MouseButtonFlags = RawInput->data.mouse.usFlags;
        
        u32 FlagCheck = 1;
        for(u32 MouseButtonIndex = 0;
            MouseButtonIndex < ButtonKey_MouseCount;
            ++MouseButtonIndex)
        {
            if(MouseButtonFlags & FlagCheck)
            {
                Win32ProcessMouseMessage(Input, true, (debug_mouse_key)MouseButtonIndex);
            }
            
            FlagCheck <<= 1;
            if(MouseButtonFlags & FlagCheck)
            {
                Win32ProcessMouseMessage(Input, false, (debug_mouse_key)MouseButtonIndex);
            }
            FlagCheck <<= 1;
        }
        
#if 0                    
        win32_clamped_string MouseMovementType = {};
        //TODO(Alex): MOUSE_VIRTUAL_DESKTOP?
        //NOTE(Alex):Mouse movement data is relative to the last mouse position.
        if(!RawInput->data.mouse.usFlags)
        {
            u32 TextBCount = StringLength("MOUSE_RELATIVE");
            ConcatStrings(MouseMovementType.Buffer, MouseMovementType.Count, 
                          "MOUSE_RELATIVE", TextBCount, 
                          MouseMovementType.Buffer, sizeof(MouseMovementType));
            MouseMovementType.Count += TextBCount;
        }
        
        //NOTE(Alex):Mouse movement data is based on absolute position.
        if(RawInput->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
        {
            u32 TextBCount = StringLength("MOUSE_ABSOLUTE");
            ConcatStrings(MouseMovementType.Buffer, MouseMovementType.Count, 
                          "MOUSE_ABSOLUTE", TextBCount, 
                          MouseMovementType.Buffer, sizeof(MouseMovementType));
            MouseMovementType.Count += TextBCount;
        }
        
        TCHAR OutputBuffer[256];
        _snprintf_s(OutputBuffer, ArrayCount(OutputBuffer),
                    "%p %.*s ButtonFlags=%i, ButtonData=%04x, RawButtons=%04x,X: %02x Y: %02x\n",
                    RawInput->header.hDevice,
                    MouseMovementType.Count,
                    MouseMovementType.Buffer,
                    RawInput->data.mouse.usButtonFlags,
                    RawInput->data.mouse.usButtonData,
                    RawInput->data.mouse.ulRawButtons,
                    RawInput->data.mouse.lLastX,
                    RawInput->data.mouse.lLastY);
        
        OutputDebugStringA(OutputBuffer);
#endif
        
    }
    
    free(DeviceInfo);
    free(RawInput);
    
}break;
#endif
