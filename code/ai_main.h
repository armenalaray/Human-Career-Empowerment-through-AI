/*

TODO(Alex):

- Test theta parameters that you have right now! With the training model
- API cleaning, define the entry points to work with
- NN implementation - Implement Logistic NN and test with the student information


*/


#ifndef AI_MAIN_H

#define INCREASE_SET(Set) (++(Set)->MaxRowIndex)
#define GET_SET_COUNT(Set) ((Set)->MaxRowIndex - (Set)->MinRowIndex + 1); 


struct memory_arena
{
    void * Base;
    memory_index Size;
    memory_index Used;
};

struct temp_memory
{
    memory_arena  * Arena;
    memory_index Threshold;
    u32 TempCount;
};

internal temp_memory 
BeginTempMemory(memory_arena * Arena)
{
    temp_memory Result = {};
    Result.Arena = Arena;
    Result.Threshold = Arena->Used;
    ++Result.TempCount;
    
    
    return Result;
}

internal void
EndTempMemory(temp_memory * TempMemory)
{
    TempMemory->Arena->Used = TempMemory->Threshold;
    --TempMemory->TempCount;
    Assert(TempMemory->TempCount == 0);
}

internal void
InitializeArena(memory_arena * Arena, void * Base, u64 Size)
{
    Arena->Base = Base;
    Arena->Size = Size;
    Arena->Used = 0;
}


#define PushStruct(Arena, Type) (Type *)PushSize(Arena, sizeof(Type));
#define PushArray(Arena, Count, Type) (Type *)PushSize(Arena, Count * sizeof(Type));

//TODO(Alex): Add Alignment
internal void *
PushSize(memory_arena * Arena, u32  Size)
{
    Assert(Arena && Arena->Base);
    Assert((Arena->Used + Size)  < Arena->Size);
    
    void * Result = (char *)Arena->Base + Arena->Used;
    Arena->Used += Size;
    return Result;
}

struct ai_state;
#include "ai_math.h"
#include "ai_render.h"

//TODO(Alex): Find some way to modify this dinamically?
#define MAX_LEARNING_CURVE_ENTRIES 20
#define MAX_LEARNING_ITERS 700

#define INIT_TRAINING_BOUND_MAX 69
#define INIT_TEST_BOUND_MIN 90

struct ai_graph_entries
{
    r64 TrainingCost;
    r64 CVCost;
    u32 TrainCount;
};


struct ai_matrix_set
{
    matrix_2 * Matrix;
    u32 MinRowIndex;
    u32 MaxRowIndex;
};

struct ai_state
{
    memory_arena TranArena;
    memory_arena AIArena;
    
    u32 IterIndex;
    u32 TrainingSetSize;
    r64 InverseTrainingSetSize;
    
    r64 h;
    r64 RegParam;
    
    r64 Cost;
    matrix_2 X;
    matrix_2 y;
    matrix_2 Theta;
    
    u32 TestIndex;
    ai_graph_entries GraphEntries[MAX_LEARNING_CURVE_ENTRIES];
    
    ai_matrix_set TrainingSetX;
    ai_matrix_set TrainingSety;
    
    ai_matrix_set TestSetX;
    ai_matrix_set TestSety;
    
    b32 IsInitialized;
    
    //NOTE(Alex): Font/Glyph Data
    u32 FontCount;
    asset_font Fonts[8];
    
    //TODO(Alex): Do ScaleTable
#if 0    
    u32 GlyphSizeCount;
    u32 CurrentGlyphSize;
#endif
    
    r32 CurrentFontScale;
    codepoint_glyph CGlyphTable[4096]; 
    
    u32 GlyphCount;
    asset_glyph Glyphs[4096];
    kerning_pair * KerningTable[4096];
    
    r32 AtY;
    r32 AtX;
};

inline ai_state * GetAIState(void);

#define AI_MAIN_H
#endif
