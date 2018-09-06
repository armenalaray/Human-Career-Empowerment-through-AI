
/*
NOTE(Alex): Implementation of a Logistic Regression in C

Considerations: 
I could write out data (weights, lambdas, regularization variables, etc) out into a file for test if any  

Input Training Set Input 1000's X 23
Y Vector  10000's X 1;

-We use double precision for more accurate activation of model units.
The size intended for the table(1,000 - 100,000 examples);

Think on how to reliably adjust to bigger data sizes( >= 1,0000,0000 examples) 

Do we want it to be multiplatform?
*/

//TODO(Alex): Implement functions yourself later 
#include "ai_platform.h"
#include "ai_main.h"
#include "ai_generated.cpp"
#include "ai_math.cpp"
#include "ai_render.cpp"

#define STB_TRUETYPE_IMPLEMENTATION 
#include "stb_truetype.h"

//TODO(Alex): Remove this!
global_variable matrix_2 GlobalDataMatrix;
//#define MAX_NUM_TEST 8


//TODO(Alex): remove this global, if possible.
//NOTE(Alex): This is here for operator overloading
global_variable ai_state * GlobalAIState;
inline ai_state * GetAIState(void)
{
    return GlobalAIState;
}

internal b32
InputParamsAreValid(matrix_2 * X, matrix_2 * y)
{
    b32 Result = (X->Data && y->Data && 
                  X->CountX && X->CountY,
                  y->CountX && y->CountY);
    
    return Result;
}

internal void
ai_LearningStep(ai_state * AIState, r64 * Out_Cost)
{
    if(GlobalAIState && 
       (AIState->IterIndex < MAX_LEARNING_ITERS))
    {
        temp_memory TempMemory = BeginTempMemory(&AIState->TranArena);
        matrix_2 Grads = GetGradient(AIState, &AIState->TrainingSetX, &AIState->TrainingSety, AIState->RegParam); 
        AIState->Theta = (AIState->Theta - (AIState->h * Grads));
        *Out_Cost = CostOfTheta(AIState, &AIState->TrainingSetX, &AIState->TrainingSety, AIState->RegParam);
        
        EndTempMemory(&TempMemory);
        ++AIState->IterIndex;
    }
}


//TODO(Alex): Move this calculations outside the loop, they dont need to be calculated twice!
internal r64 FindMaxCost(ai_graph_entries * Entries, u32 EntryCount)
{
    r64 Result = MinR64;
    for(u32 Index = 0;
        Index < EntryCount;
        ++Index)
    {
        ai_graph_entries * Entry = Entries + Index;
        EXCHANGE_MAX_R64(Result, Entry->TrainingCost);
        EXCHANGE_MAX_R64(Result, Entry->CVCost);
    }
    
    return Result;
}

struct graph_point
{
    vector_2 P;
    vector_4 Color;
};

struct graph_point_result
{
    u32 PointCount;
    graph_point * Points;
};

internal graph_point_result * 
MapGraphEntriesToBasis(memory_arena * Arena, ai_graph_entries * Entries, u32 EntryCount, vector_2 Origin, vector_2 XAxis, vector_2 YAxis)
{
    graph_point_result * Result = 0;
    
    uint32_t PointIndex = 0;
    Result = PushStruct(Arena, graph_point_result);
    Result->PointCount = (EntryCount << 1);
    Result->Points = PushArray(Arena, Result->PointCount, graph_point);
    
    if(Result && Result->Points)
    {
        //NOTE(Alex): Here we are assuming that the last entry has the biggest trainingcount; 
        ai_graph_entries * LastEntry = Entries + (EntryCount - 1);
        r32 InverseMaxTrainingCount = 1.0f / (r32)LastEntry->TrainCount;
        r32 MaxCost = (r32)FindMaxCost(Entries, EntryCount);
        r32 InverseMaxCost = 1.0f / MaxCost;
        
        for(u32 Index = 0;
            Index < EntryCount;
            ++Index)
        {
            Assert(PointIndex < Result->PointCount);
            
            ai_graph_entries * Entry = Entries + Index;
            graph_point * RTrain = Result->Points + PointIndex++;
            graph_point * RCV = Result->Points + PointIndex++;
            
            r32 DispX = ((r32)Entry->TrainCount * InverseMaxTrainingCount * XAxis.x);
            r32 DispYTrain = ((r32)Entry->TrainingCost * InverseMaxCost * YAxis.y);
            r32 DispYCV = ((r32)Entry->CVCost * InverseMaxCost * YAxis.y);
            
#if 0            
            Assert(DispX < XAxis.x);
            Assert(DispYTrain < YAxis.y);
            Assert(DispYCV < YAxis.y);
#endif
            
            RTrain->P.x = Origin.x + DispX;
            RTrain->P.y = Origin.y + DispYTrain;
            RTrain->Color = Vector4(1.0f,1.0f,0.0f,1.0f);
            
            RCV->P.x = Origin.x + DispX;
            RCV->P.y = Origin.y + DispYCV;
            RCV->Color = Vector4(0.0f,1.0f,1.0f,1.0f);
        }
    }
    
    return Result;
}

internal void
DebugPlot2D(ai_state * AIState, asset_bitmap * Buffer, ai_graph_entries * Entries, u32 EntryCount)
{
    vector_2 Origin = {300,100};  
    vector_2 YAxis = Vector2(0, 400.0f);  
    vector_2 XAxis = Vector2(400.0f, 0);  
    
    r32  BoxWidth, BoxHeight, Red, Green, Blue;
    BoxWidth = BoxHeight = 8.0f;
    Blue = Green = Red = 1.0f;
    
    graph_point_result * Graph = MapGraphEntriesToBasis(&AIState->TranArena, Entries, EntryCount, Origin, XAxis, YAxis);
    if(Graph)
    {
        
        rectangle_2 OriginRect = RectCenterDim(Origin, Vector2(BoxWidth, BoxHeight));
        DrawRectangleAt(Buffer, OriginRect, 0.0f, 1.0f, 0.0f);
        
        rectangle_2 XBasisRect = RectCenterDim(XAxis + Origin, Vector2(BoxWidth, BoxHeight));
        DrawRectangleAt(Buffer, XBasisRect, 1.0f, 0.0f, 0.0f);
        
        rectangle_2 YRect = RectCenterDim(YAxis  + Origin, Vector2(BoxWidth, BoxHeight));
        DrawRectangleAt(Buffer, YRect, 0.0f, 0.0f, 1.0f);
        
        for(u32 Index = 0;
            Index < Graph->PointCount;
            ++Index)
        {
            graph_point * Point = Graph->Points + Index;
            rectangle_2 Rect = RectCenterDim(Point->P, Vector2(BoxWidth, BoxHeight));
            DrawRectangleAt(Buffer, Rect, Point->Color.r, Point->Color.g, Point->Color.b, Point->Color.a);
        }
    }
}

extern "C" AI_UPDATE_AND_RENDER(AIUpdateAndRender)
{
    ai_state * AIState = (ai_state*)Memory->AIStorage;
    GlobalAIState = AIState;
    
    if(!AIState->IsInitialized)
    {
        InitializeArena(&AIState->AIArena, (char*)Memory->AIStorage + sizeof(ai_state), Megabytes(500));
        InitializeArena(&AIState->TranArena, (char*)Memory->TransientStorage, Megabytes(500));
        
        META_MATRIX_X(MX, &AIState->AIArena);
        META_MATRIX_y(My, &AIState->AIArena);
        //NOTE(Alex): Matrix m x n and Theta is a n x 1 
        if(InputParamsAreValid(&MX, &My))
        {
            MX = AddBiasUnit(&AIState->AIArena, &MX);
            AIState->y = My;
            AIState->X = MX;
            AIState->Theta = MatrixXY(&AIState->AIArena, AIState->X.CountY, 1);
            AIState->TrainingSetSize = AIState->X.CountX;
            AIState->InverseTrainingSetSize = ((r64)1.0 / (r64)AIState->TrainingSetSize);
            AIState->IterIndex = 0;
            
            //NOTE(Alex): User parameters 
            AIState->h = 0.01;
            AIState->RegParam = 1.0f;
            
            AIState->TrainingSetX.Matrix = &AIState->X; 
            AIState->TrainingSetX.MinRow = 0; 
            AIState->TrainingSetX.MaxRow = INIT_TRAINING_BOUND_MAX; 
            
            AIState->TrainingSety.Matrix = &AIState->y; 
            AIState->TrainingSety.MinRow = 0; 
            AIState->TrainingSety.MaxRow = INIT_TRAINING_BOUND_MAX; 
            
            AIState->TestSetX.Matrix = &AIState->X; 
            AIState->TestSetX.MinRow = INIT_TEST_BOUND_MIN; 
            AIState->TestSetX.MaxRow = AIState->X.CountX;
            
            AIState->TestSety.Matrix = &AIState->y; 
            AIState->TestSety.MinRow = INIT_TEST_BOUND_MIN; 
            AIState->TestSety.MaxRow = AIState->y.CountX;
            
            
            AIState->IsInitialized = true;
        }
        
        //TODO(Alex): Subpixel alignment
        //TODO(Alex): Formalize codepoint sizes
        //NOTE(Alex): We keep zero index for null or invalid glyph indices
        AIState->GlyphCount = 1;
        stbtt_fontinfo FontInfo = {};
        //debug_file_content TTFContent = Memory->DebugReadEntireFile("C:/Windows/Fonts/liberation-mono.ttf");
        debug_file_content TTFContent = Memory->DebugReadEntireFile("C:/Windows/Fonts/arial.ttf");
        b32 FontInit = stbtt_InitFont(&FontInfo, (const unsigned char *)TTFContent.Content, 0);//stbtt_GetFontOffsetForIndex((const unsiged char*)TTFContent.Content, 1));
        if(FontInit)
        {
            asset_font * Font = InitAssetFont(AIState);
            s32 Ascent = 0;
            s32 Descent = 0;
            s32 LineGap = 0;
            r32 PixelHeight = 30.0f;
            r32 Scale = stbtt_ScaleForPixelHeight(&FontInfo, PixelHeight);
            
            AIState->CurrentFontScale = Scale;
            stbtt_GetFontVMetrics(&FontInfo, &Ascent, &Descent, &LineGap); 
            
            Font->YAdvance = Scale * (Ascent - Descent + LineGap);
            r32 Baseline = Scale * Ascent;
            
            for(u32 CodePoint = Min_Codepoint_Extract;
                CodePoint <= Max_Codepoint_Extract;
                ++CodePoint)
            {
                
#if 1
                s32 Width;
                s32 Height;
                s32 XOffset;
                s32 YOffset;
                
                unsigned char * MonoBitmap = stbtt_GetCodepointBitmap(&FontInfo, 0, Scale, CodePoint, &Width, &Height, &XOffset, &YOffset);
                
                s32 HAdvance;
                s32 LeftSideBearing;
                stbtt_GetCodepointHMetrics(&FontInfo, 
                                           CodePoint, 
                                           &HAdvance, 
                                           &LeftSideBearing);
                
                s32 x0, y0, x1, y1;
                stbtt_GetCodepointBitmapBox(&FontInfo, CodePoint, 
                                            Scale, Scale, 
                                            &x0, &y0, &x1, &y1);
                
                Assert((Width == (x1 - x0)) && 
                       (Height == (y1 - y0)));
                
#else
                //stbtt_GetCodepointBitmapBoxSubpixel(&FontInfo, CodePoint, Scale, Scale, 0, 0, &x0, &y0, &x1, &y1);
                s32 Width = x1 - x0;
                s32 Height = y1 - y0;
                
                unsigned char * MonoBitmap = (unsigned char*)PushSize(&AIState->TranArena, Height * Width);
                unsigned char * P = MonoBitmap + (-y0 * Width) + (u32)(LeftSideBearing * Scale);
                stbtt_MakeCodepointBitmapSubpixel(&FontInfo, P, 
                                                  Width, Height,
                                                  0, 
                                                  Scale, Scale,
                                                  0,0,
                                                  CodePoint);
#endif
                
                asset_glyph * Glyph = AddNextGlyphIndex(AIState, Font, CodePoint, HAdvance, LeftSideBearing, x0, y0, x1, y1);
                Glyph->Bitmap.Data = PushSize(&AIState->AIArena, Width * Height * BITMAP_BYTES_PER_PIXEL); 
                Glyph->Bitmap.Width = Width;
                Glyph->Bitmap.Height = Height;
                Glyph->Bitmap.BytesPerPixel = BITMAP_BYTES_PER_PIXEL;
                
                if(MonoBitmap)
                {
                    u32 * Dest = (u32*)Glyph->Bitmap.Data;
                    unsigned char * Source = MonoBitmap + (Height - 1) * Width;
                    //unsigned char * Source = MonoBitmap;
                    u32 * DestRow = Dest; 
                    unsigned char * SourceRow = Source; 
                    
                    for(s32 Y = 0;
                        Y < Height;
                        ++Y)
                    {
                        Source = SourceRow;
                        Dest= DestRow;
                        for(s32 X = 0;
                            X < Width;
                            ++X)
                        {
                            u32 Pixel = MapMonoTo4Channel(*Source);
                            *Dest = Pixel;
                            ++Source;
                            ++Dest;
                        }
                        
                        DestRow += Width;
                        SourceRow -= Width;
                    }
                }
                
                for(u32 NextCodePoint = Min_Codepoint_Extract;
                    NextCodePoint < Max_Codepoint_Extract;
                    ++NextCodePoint)
                {
                    s32 KerningValue = stbtt_GetCodepointKernAdvance(&FontInfo, CodePoint, NextCodePoint);
                    if(KerningValue != 0) 
                    {
                        int x = 5;
                    }
                    GetKerningForPair(AIState, CodePoint, NextCodePoint, (r32)KerningValue);
                }
                
                stbtt_FreeBitmap(MonoBitmap, 0);
            }
        }
    }
    
    
    r64 Cost = {};
    if(AIState->TestIndex < MAX_LEARNING_CURVE_ENTRIES)
    {
        //TODO(Alex): Run this on a second thread?
        ai_LearningStep(AIState, &Cost);
        if(AIState->IterIndex == MAX_LEARNING_ITERS)
        {
            u32 TestIndex = AIState->TestIndex++;
            ai_graph_entries * Entry = AIState->GraphEntries + TestIndex;
            
            Entry->TrainingCost = Cost;
            Entry->CVCost = CostOfTheta(AIState, &AIState->TestSetX, &AIState->TestSety, AIState->RegParam, INIT_TEST_BOUND_MIN);
            Entry->TrainCount = GET_SET_COUNT(&AIState->TrainingSetX); 
            
            AIState->Theta = Zeroes(AIState->Theta.CountX, AIState->Theta.CountY);
            INCREASE_SET(&AIState->TrainingSetX);
            INCREASE_SET(&AIState->TrainingSety);
            
            AIState->IterIndex = 0;
        }
    }
    
    ClearScreen(FrameBuffer, 0.2f, 0.2f, 0.2f, 1.0f);
    
    //TODO(Alex): Control writing positioning
    AIState->AtX = 0;
    AIState->AtY = (r32)FrameBuffer->Height - AIState->Fonts[0].YAdvance;
    
    temp_memory RenderTempMem = BeginTempMemory(&AIState->TranArena);
    
    char BThetasText[4096] = {0};
    sprintf_s(BThetasText, ArrayCount(BThetasText), "Thetas: ");
    RenderText(AIState, FrameBuffer, BThetasText);
    
    char BThetas[4096] = {0};
    u32 Count = ElementCount(&AIState->Theta);
    for(u32 Index = 0;
        Index < Count;
        ++Index)
    {
        sprintf_s(BThetas, ArrayCount(BThetas), "%f ", AIState->Theta.Data[Index]);
        RenderText(AIState, FrameBuffer, BThetas);
    }
    
    char BCost[4096] = {0};
    sprintf_s(BCost, ArrayCount(BCost), "\nCost: %f\n", Cost);
    RenderText(AIState, FrameBuffer, BCost);
    
    if(AIState->TestIndex >= MAX_LEARNING_CURVE_ENTRIES)
    {
        RenderText(AIState, FrameBuffer,  "Learning Curves");
        DebugPlot2D(AIState, FrameBuffer, AIState->GraphEntries, ArrayCount(AIState->GraphEntries));
    }
    
    EndTempMemory(&RenderTempMem);
}

//NOTE(Alex): Multiple Learning rates test 
#if 0
for(u32 TestIndex = 0;
    TestIndex < MAX_NUM_TEST;
    ++TestIndex)
{
    Theta[TestIndex] = MatrixXY(&AIState->AIArena, X.CountY, 1);
    for(u32 Index = 0;
        Index < ElementCount(&Theta[TestIndex]);
        ++Index)
    {
        Theta[TestIndex].Data[Index] = 30.0;
    }
}

r64 StepSize[MAX_NUM_TEST] = 
{
    0.001,
    0.003,
    0.01,
    0.03,
    0.1,
    0.3,
    1.0,
    3.0,
};

r64 InitCost = CostOfTheta(AIState, Theta[0], X, y, InverseTrainingSetSize);
matrix_2 InitGrads = GetGradient(AIState, X, y, Theta[0], InverseTrainingSetSize); 
PrintModelData(&Theta[0], &InitGrads, InitCost);

//NOTE(Alex): Close to optimal values of Theta that maxmimizes CostOfTheta(); 
Theta[0].Data[0] = -24.0;
Theta[0].Data[1] = 0.2;
Theta[0].Data[2] = 0.2;

r64 CVCost = CostOfTheta(AIState, Theta[0], X, y, InverseTrainingSetSize);
matrix_2 TestGrads = GetGradient(AIState, X, y, Theta[0], InverseTrainingSetSize); 
#endif


















