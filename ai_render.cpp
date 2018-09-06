/*
Alejandro Armenta Ayala 

ai_render.cpp
*/


inline b32 
IsNormalized(r32 Value)
{
    b32 Result = ((Value >= 0) && (Value <= 1.0f));
    return Result;
}

inline r32
LinearBlend(r32 A, r32 t, r32 B)
{
    r32 Result = ((1 - t) * A) + (t * B);
    return Result;
}

//TODO(ALex): Optimize this!
internal void
DrawBitmapAt(asset_bitmap * Buffer, 
             asset_bitmap * Bitmap, 
             r32 AtX, 
             r32 AtY,
             r32 Red = 1.0f, 
             r32 Green = 1.0f, 
             r32 Blue = 1.0f, 
             r32 Alpha = 1.0f)
{
    //NOTE(Alex): AtX, AtY can be outside of the Buffer 
    //We need to map Screen Coordinates - to bitmap coordinates
    //TODO(Alex): Do we want to un-normalized Color values
    Assert(Buffer && 
           Bitmap && 
           Buffer->Data 
           && Bitmap->Data);
    
    Assert(IsNormalized(Red) &&
           IsNormalized(Green) &&
           IsNormalized(Blue) &&
           IsNormalized(Alpha));  
    
    
    Assert(Buffer->Data);
    s32 MinBufferX = RoundR32ToS32(AtX);
    s32 MinBufferY = RoundR32ToS32(AtY);
    s32 MaxBufferX = RoundR32ToS32(AtX + (r32)Bitmap->Width);
    s32 MaxBufferY = RoundR32ToS32(AtY + (r32)Bitmap->Height);
    
    s32 BufferMaxHeight =  (Buffer->Height - 1);
    s32 BufferMaxWidth =  (Buffer->Width - 1);
    
    u32 BlueMask = 0x000000FF;
    u32 GreenMask = 0x0000FF00;
    u32 RedMask = 0x00FF0000;
    
    if(MaxBufferY > BufferMaxHeight)
    {
        MaxBufferY = BufferMaxHeight;
    }
    if(MaxBufferX > BufferMaxWidth)
    {
        MaxBufferX = BufferMaxWidth;
    }
    
    s32 MinBitmapY = 0;
    if(MinBufferY < 0)
    {
        MinBitmapY = MinBitmapY - MinBufferY;
        MinBufferY = 0;
    }
    s32 MinBitmapX = 0;
    if(MinBufferX < 0)
    {
        MinBitmapX = MinBitmapX - MinBufferX;
        MinBufferX = 0;
    }
    
    r32 Inverse255 = 1 / 255.0f;
    
    u32 * Dest = (u32*)Buffer->Data + (MinBufferY * Buffer->Width + MinBufferX);
    u32 * Source = (u32*)Bitmap->Data + (MinBitmapY * Bitmap->Width + MinBitmapX);
    
    u32 * RowDest = Dest;
    u32 * RowSource = Source;
    
    for(s32 Y = MinBufferY; 
        Y < MaxBufferY;
        ++Y)
    {
        Dest = RowDest;
        Source = RowSource;
        
        for(s32 X = MinBufferX; 
            X < MaxBufferX;
            ++X)
        {
            //NOTE(Alex): AA RR GG BBB
            
            //TODO(Alex): Pre-multiplied alpha
            //TODO(Alex): Antialiasing
            
            //TODO(Alex) UnMap to 0-1  color space? 
            u32 SColor = *Source;
            u32 DColor = *Dest;
#if 1
            r32 SBlue = (r32)((SColor >> 0) & 0xFF);
            r32 SGreen = (r32)((SColor >> 8) & 0xFF); 
            r32 SRed = (r32)((SColor >> 16) & 0xFF);
            r32 SAlpha = (r32)((SColor >> 24) & 0xFF);
            
            r32 DBlue = (r32)((DColor >> 0) & 0xFF);
            r32 DGreen = (r32)((DColor >> 8) & 0xFF); 
            r32 DRed = (r32)((DColor >> 16) & 0xFF);
            r32 DAlpha = (r32)((DColor >> 24) & 0xFF);
            
            //r32 RAlpha = LinearBlend(DAlpha, 0.5f,  SAlpha) * Inverse255;
            r32 RAlpha = SAlpha * Inverse255;
            r32 RBlue = LinearBlend(DBlue, RAlpha, SBlue) * Blue;
            r32 RGreen = LinearBlend(DGreen,RAlpha, SGreen) * Green;
            r32 RRed = LinearBlend(DRed, RAlpha, SRed) * Red;
            
            u32 RColor = (((RoundR32ToU32(RBlue) & 0xFF) << 0) | 
                          ((RoundR32ToU32(RGreen)  & 0xFF) << 8) |
                          ((RoundR32ToU32(RRed) & 0xFF) << 16) |
                          ((RoundR32ToU32(RAlpha) & 0xFF) << 24));
#else
            u32 RColor = *Source;
#endif
            
            *Dest = RColor;
            ++Dest;
            ++Source;
        }
        
        RowDest += Buffer->Width;
        RowSource += Bitmap->Width;
    }
}

//TODO(Alex): Normalize values on to 0 - 1 space 
internal void
DrawRectangleAt(asset_bitmap * Buffer, r32 x0, r32 y0, r32 x1, r32 y1, r32 SRed = 1.0f, r32 SGreen = 1.0f, r32 SBlue = 1.0f, r32 SAlpha = 1.0f)
{
    //NOTE(Alex): AtX, AtY can be outside of the Buffer 
    //We need to map Screen Coordinates - to bitmap coordinates
    Assert(Buffer && Buffer->Data);
    
    //TODO(Alex): Do we want to un-normalize Color values
    Assert(IsNormalized(SRed) &&
           IsNormalized(SGreen) &&
           IsNormalized(SBlue) &&
           IsNormalized(SAlpha));  
    
    
    Assert(Buffer->Data);
    s32 MinBufferX = RoundR32ToS32(x0);
    s32 MinBufferY = RoundR32ToS32(y0);
    s32 MaxBufferX = RoundR32ToS32(x1);
    s32 MaxBufferY = RoundR32ToS32(y1);
    
    s32 BufferMaxHeight =  (Buffer->Height - 1);
    s32 BufferMaxWidth =  (Buffer->Width - 1);
    
    if(MaxBufferY > BufferMaxHeight)
    {
        MaxBufferY = BufferMaxHeight;
    }
    if(MaxBufferX > BufferMaxWidth)
    {
        MaxBufferX = BufferMaxWidth;
    }
    
    s32 MinBitmapY = 0;
    if(MinBufferY < 0)
    {
        MinBitmapY = MinBitmapY - MinBufferY;
        MinBufferY = 0;
    }
    s32 MinBitmapX = 0;
    if(MinBufferX < 0)
    {
        MinBitmapX = MinBitmapX - MinBufferX;
        MinBufferX = 0;
    }
    
    r32 Inverse255 = 1 / 255.0f;
    u32 * Dest = (u32*)Buffer->Data + (MinBufferY * Buffer->Width + MinBufferX);
    u32 * RowDest = Dest;
    
    for(s32 Y = MinBufferY; 
        Y < MaxBufferY;
        ++Y)
    {
        Dest = RowDest;
        for(s32 X = MinBufferX; 
            X < MaxBufferX;
            ++X)
        {
            //NOTE(Alex): AA RR GG BBB
            
            //TODO(Alex): Pre-multiplied alpha
            //TODO(Alex): Antialiasing
            
            //TODO(Alex) UnMap to 0-1 color space? 
            u32 DColor = *Dest;
            
            r32 DBlue = (r32)((DColor >> 0) & 0xFF) * Inverse255;
            r32 DGreen = (r32)((DColor >> 8) & 0xFF) * Inverse255; 
            r32 DRed = (r32)((DColor >> 16) & 0xFF) * Inverse255;
            r32 DAlpha = (r32)((DColor >> 24) & 0xFF) * Inverse255;
            
            //r32 RAlpha = LinearBlend(DAlpha, 0.5f, SAlpha) * Inverse255;
            r32 RBlue = 255.0f * LinearBlend(DBlue, SAlpha, SBlue);
            r32 RGreen = 255.0f * LinearBlend(DGreen,SAlpha, SGreen);
            r32 RRed = 255.0f * LinearBlend(DRed, SAlpha, SRed);
            
            u32 RColor = (((RoundR32ToU32(RBlue) & 0xFF) << 0) | 
                          ((RoundR32ToU32(RGreen)  & 0xFF) << 8) |
                          ((RoundR32ToU32(RRed) & 0xFF) << 16) |
                          ((RoundR32ToU32(SAlpha) & 0xFF) << 24));
            
            *Dest = RColor;
            ++Dest;
        }
        
        RowDest += Buffer->Width;
    }
}

internal void
DrawRectangleAt(asset_bitmap * Buffer, rectangle_2 Rect, r32 SRed = 1.0f, r32 SGreen = 1.0f, r32 SBlue = 1.0f, r32 SAlpha = 1.0f)
{
    DrawRectangleAt(Buffer, Rect.MinX, Rect.MinY, Rect.MaxX, Rect.MaxY, SRed, SGreen, SBlue, SAlpha);
}

internal void
ClearScreen(asset_bitmap * Buffer, r32 Red, r32 Green, r32 Blue, r32 Alpha)
{
    //NOTE(Alex): AtX, AtY can be outside of the Buffer 
    //We need to map Screen Coordinates - to bitmap coordinates
    //TODO(Alex): Do we want to un-normalized Color values
    Assert(IsNormalized(Red) &&
           IsNormalized(Green) &&
           IsNormalized(Blue) &&
           IsNormalized(Alpha));  
    
    
    u32 Blue255  = RoundR32ToU32(255.0f * Blue) & 0xFF;
    u32 Green255 = RoundR32ToU32(255.0f * Green) & 0xFF;
    u32 Red255 = RoundR32ToU32(255.0f * Red) & 0xFF;
    u32 Alpha255 = RoundR32ToU32(255.0f * Alpha) & 0xFF;
    
    //NOTE(Alex): AA RR GG BBB
    u32 RColor = (u32)((Blue255 << 0) | (Green255 << 8) |(Red255 << 16) | (Alpha255 << 24));
    
    u32 * Dest = (u32*)Buffer->Data;
    u32 * RowDest = Dest;
    for(s32 Y = 0; 
        Y < (Buffer->Height - 1);
        ++Y)
    {
        Dest = RowDest;
        for(s32 X = 0; 
            X < (Buffer->Width - 1);
            ++X)
        {
            *Dest = RColor;
            ++Dest;
        }
        
        RowDest += Buffer->Width;
    }
}

#if 0
internal void
DrawRectangleAt(asset_bitmap * Buffer, 
                r32 x0, r32 y0, r32 x1, r32 y1, 
                r32 Red, r32 Green, r32 Blue, r32 Alpha)
{
    //NOTE(Alex): AtX, AtY can be outside of the Buffer 
    //We need to map Screen Coordinates - to bitmap coordinates
    //TODO(Alex): Do we want to un-normalized Color values
    Assert(IsNormalized(Red) &&
           IsNormalized(Green) &&
           IsNormalized(Blue) &&
           IsNormalized(Alpha));  
    
    u32 Blue255  = RoundR32ToU32(255.0f * Blue) & 0xFF;
    u32 Green255 = RoundR32ToU32(255.0f * Green) & 0xFF;
    u32 Red255 = RoundR32ToU32(255.0f * Red) & 0xFF;
    u32 Alpha255 = RoundR32ToU32(255.0f * Alpha) & 0xFF;
    
    //NOTE(Alex): AA RR GG BBB
    u32 RColor = (u32)((Blue255 << 0) | (Green255 << 8) |(Red255 << 16) | (Alpha255 << 24));
    
    u32 * Dest = (u32*)Buffer->Data;
    u32 * RowDest = Dest;
    for(s32 Y = 0; 
        Y < (Buffer->Height - 1);
        ++Y)
    {
        Dest = RowDest;
        for(s32 X = 0; 
            X < (Buffer->Width - 1);
            ++X)
        {
            *Dest = RColor;
            ++Dest;
        }
        
        RowDest += Buffer->Width;
    }
}
#endif


inline b32 
IsValidCGlyph(codepoint_glyph CGlyph)
{
    b32 Result = (CGlyph.GlyphIndex); 
    return Result;
}

inline b32
IsValidFont(asset_font * Font)
{
    Assert(Font->MaxGlyphIndex >= Font->MinGlyphIndex);
    b32 Result = (Font->MinGlyphIndex && Font->MaxGlyphIndex);
    return Result;
}


internal asset_glyph *
GetGlyphMetrics(ai_state * AIState, u32 CodePoint)
{
    codepoint_glyph CGlyph = AIState->CGlyphTable[CodePoint];
    asset_glyph * Result = 0;
    if(IsValidCGlyph(CGlyph))
    {
        Result = AIState->Glyphs + CGlyph.GlyphIndex;
    }
    
    return Result;
}


internal r32
GetHAdvance(ai_state * AIState, u32 CodePoint)
{
    r32 Result = {};
    asset_glyph * Glyph = GetGlyphMetrics(AIState, CodePoint);
    if(Glyph)
    {
        Result = Glyph->HAdvance;
    }
    
    return Result;
}

//NOTE(Alex): We position each glyph based on the outline!
internal vector_2 
PositionGlyph(ai_state * AIState, u32 CodePoint)
{
    vector_2 Result = {};
    asset_glyph * Glyph = GetGlyphMetrics(AIState ,CodePoint);
    if(Glyph)
    {
        //NOTE(Alex): this is the rectangle that defines the glyph adjust the rendered glyph
        //so it fits in this BBox;
        //NOTE(Alex): You are Here!
        vector_2 MinP = Vector2(AIState->AtX + Glyph->x0, AIState->AtY - Glyph->y1);
        //vector_2 MaxP = Vector2(AIState->AtX + Glyph->x1, AIState->AtY + Glyph->y1);
        Result = MinP;
    }
    
    return Result;
}

internal asset_bitmap *
GetGlyphFromCodePoint(ai_state * AIState, u32 CodePoint)
{
    asset_bitmap * Result = 0;
    asset_glyph * Glyph = GetGlyphMetrics(AIState ,CodePoint);
    if(Glyph)
    {
        Result = &Glyph->Bitmap;
    }
    
    return Result;
}

internal r32  
GetKerningForPair(ai_state * AIState, 
                  u32 CodePoint, 
                  u32 NextCodePoint, 
                  r32 KerningValue = 0.0f)
{
    kerning_pair * PairFound = 0;
    kerning_pair ** FirstPair = (AIState->KerningTable + CodePoint);
    kerning_pair * Iter = *FirstPair;
    while(Iter) 
    {
        if(Iter->NextCode == NextCodePoint)
        {
            PairFound = Iter;
            break;
        }
        
        Iter = Iter->Next;
    }
    
    if(!PairFound)
    {
        PairFound = PushStruct(&AIState->AIArena, kerning_pair); 
        PairFound->Next = *FirstPair;
        PairFound->KerningValue = KerningValue;
        PairFound->NextCode = NextCodePoint;
        *FirstPair = PairFound;
    }
    
    Assert(PairFound);
    return (PairFound->KerningValue);
}


//TODO(Alex): Batch system for rendering glyphs! 
//use codepoint list to modify a specific glyph to be rendered into the batch!   
internal void 
RenderText(ai_state * AIState, asset_bitmap * Dest, char * Text)
{
    if(IsValidFont(&AIState->Fonts[0]))
    {
        asset_font * Font = &AIState->Fonts[0];
        r32 * AtX = &AIState->AtX;  
        r32 * AtY = &AIState->AtY;  
        
        char * At = Text;
        while(*At)
        {
            u32 C0 = (u32)*At;
            u32 C1 = (*(At+1)) ? *(At+1) : 0;
            
            if(C0 == 121)
            {
                int x = 5;
            }
            
            r32 KerningAdvance = AIState->CurrentFontScale * GetKerningForPair(AIState, C0, C1);
            r32 HAdvance = AIState->CurrentFontScale * GetHAdvance(AIState, C0);
            vector_2 P = PositionGlyph(AIState, C0);
            asset_bitmap * Glyph = GetGlyphFromCodePoint(AIState, C0);
            
            if(Glyph)
            {
                DrawBitmapAt(Dest, Glyph,  P.x, P.y);
            }
            
            *AtX += HAdvance + KerningAdvance; 
            ++At;
        }
        
        *AtX = 0;
        *AtY -= Font->YAdvance;
    }
}

#define MapMonoTo4Channel(Value) (((Value) << 24) | ((Value) << 16) | ((Value) << 8) | ((Value) << 0))

//TODO(Alex) Suport unicode
internal asset_glyph *
AddNextGlyphIndex(ai_state * AIState, 
                  asset_font * Font,  
                  u32 CodePoint,
                  s32 HAdvance,
                  s32 LeftSideBearing,
                  s32 x0, s32 y0, s32 x1, s32 y1)
{
    codepoint_glyph * CGlyph = AIState->CGlyphTable + CodePoint;
    CGlyph->CodePoint = CodePoint;
    CGlyph->GlyphIndex = AIState->GlyphCount++;
    Font->MaxGlyphIndex = CGlyph->GlyphIndex;
    
    asset_glyph * Result = AIState->Glyphs + CGlyph->GlyphIndex;
    Result->x0 = x0;
    Result->y0 = y0;
    Result->x1 = x1;
    Result->y1 = y1;
    Result->LeftSideBearing = (r32)LeftSideBearing;
    Result->HAdvance = (r32)HAdvance;
    
    return (Result);
}

internal asset_font *
InitAssetFont(ai_state * AIState)
{
    asset_font * Result = AIState->Fonts + AIState->FontCount++;
    Result->MinGlyphIndex = AIState->GlyphCount;
    Result->MaxGlyphIndex = Result->MinGlyphIndex;
    return Result;
}




















