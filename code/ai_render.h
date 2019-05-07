/*
Alejandro Armenta Ayala
ai_render.h
*/

#define Min_Codepoint_Extract 32 
#define Max_Codepoint_Extract 126

struct asset_font
{
    r32 YAdvance;
    u32 MinGlyphIndex;
    u32 MaxGlyphIndex;
};

//TODO(Alex): Allow Sparse codepoint_glyph table for unicode 
struct codepoint_glyph
{
    u32 CodePoint;
    u32 GlyphIndex;
    
#if 0    
    u32 SizeIndex;
    asset_bitmap Bitmap;
    codepoint_glyph * Next;
#endif
    
};

//NOTE(Alex): Glyph metrics are unscaled
struct kerning_pair
{
    u32 NextCode;
    r32 KerningValue; 
    kerning_pair * Next;
};

struct asset_glyph
{
    asset_bitmap Bitmap;
    
    r32 HAdvance;
    r32 LeftSideBearing;
    
#if 0    
    r32 YAdvance;
    r32 Ascent;
    r32 Descent;
    r32 LineGap;
    
    //NOTE(ALex): This Offsets are in pixel space
    r32 XOffset;
    r32 YOffset;
#endif
    
    //NOTE(Alex): BBox relative to (CurrentPoint, BaseLine)
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;
    
};


