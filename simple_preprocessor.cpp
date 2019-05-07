/*
NOTE(Alex): We are gonna preprocess handmade_simulation_region.h to find all the elements \
in the struct sim_entity and parse it so we could add the same data to debug file  
*/

#include <windows.h>
#include <stdio.h>
#include "math.h"
#include "handmade_platform.h"
#include "simple_preprocessor.h"

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0,MEM_RELEASE);
    }
}


DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{
    debug_file_content DebugFileContent ={};
    
    HANDLE FileHandle = CreateFileA(Filename,GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0,0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        //Handle exists
        LARGE_INTEGER FileSize;
        
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            //TODO: Defines for maximum values UInt32Max
            uint32 FileSize32 = SafeTruncateToUint32(FileSize.QuadPart);
            DebugFileContent.Content = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(DebugFileContent.Content)
            {
                DWORD BytesRead;
                if(ReadFile(FileHandle,DebugFileContent.Content,FileSize32, &BytesRead, 0) && 
                   (FileSize32 == BytesRead))
                {
                    DebugFileContent.ContentSize = FileSize32;
                }
                else
                {
                    DEBUGPlatformFreeFileMemory(DebugFileContent.Content);
                    DebugFileContent.Content = 0;
                }
            }
            else
            {
                //TODO:Logging
            }
        }
        else
        {
            //TODO:Logging
        }
        CloseHandle(FileHandle);
    }
    else
    {
        //TODO:Logging
    }
    return DebugFileContent;
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile)
{
    bool32 Result = false;
    HANDLE FileHandle = CreateFileA(Filename,GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0,0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if(WriteFile(FileHandle, Memory, MemorySize32, &BytesWritten, 0) && (MemorySize32 == BytesWritten))
        {
            Result = true;
        }
        else
        {
            Result = false;
        }
        CloseHandle(FileHandle);
    }
    else
    {
        //TODO:Logging
    }
    return Result;
}

//TODO(Alex): Assert there's a null terminator, on both strings
internal b32 
StringsAreEqual(string * TextA, 
                string * TextB)
{
    b32 Result = false;
    u32 Count = 0;
    
    char * A = TextA->Text;
    char * B = TextB->Text;
    
    if(A && B)
    {
        if(TextA->Count == TextB->Count)
        {
            while(*A == *B)
            {
                ++A; ++B;
                Count++;
            }
            Result = (Count == TextA->Count);
        }
    }
    
    return Result;
}

inline b32
TokenIsIdentifier(token * Token, char * Text)
{
    string OtherString = String(StringLength(Text),Text);
    b32 Result = ((Token->Type == TokenType_Identifier) && 
                  StringsAreEqual(&Token->String, &OtherString));
    return Result;
}


inline b32
IsNewLine(char C)
{
    b32 Result = ((C == '\n') || (C == '\r'));
    return Result;
}

inline b32
CharIsWhiteSpace(char C)
{
    b32 Result = ((C == ' ') || 
                  (C == '\t') || 
                  (C == '\f') || 
                  (C == '\v') || 
                  IsNewLine(C));
    return Result;
}

inline b32
IsNumber(char C)
{
    b32 Result = ((C >= '0') && 
                  (C <= '9'));
    return Result;
}

inline b32
IsAlpha(char C)
{
    b32 Result = (((C >= 'A') && 
                   (C <= 'Z')) || ((C >= 'a') && 
                                   (C <= 'z')));
    return Result;
}


inline void
ParseCStyleComment(tokenizer * Tokenizer)
{
    Assert(Tokenizer->At);
    
    b32 SpecialReturn = false;
    Tokenizer->At += 2;
    while(Tokenizer->At[0])
    {
        if(!SpecialReturn)
        {
            SpecialReturn = ((Tokenizer->At[0] == '\\') && 
                             IsNewLine(Tokenizer->At[1]) && 
                             IsNewLine(Tokenizer->At[2]));
        }
        
        if(!SpecialReturn && 
           IsNewLine(Tokenizer->At[0]))
        {
            break;
        }
        else if(SpecialReturn && !IsNewLine(Tokenizer->At[1]))
        {
            SpecialReturn = false;
        }
        
        ++Tokenizer->At;
    }
}

inline void
ParseCppStyleComment(tokenizer * Tokenizer)
{
    Assert(Tokenizer->At);
    
    Tokenizer->At += 2;
    for(;;)
    {
        if((Tokenizer->At[0] == '*') &&
           (Tokenizer->At[1] == '/')) 
        {
            Tokenizer->At += 2;
            break;
        }
    }
}

//NOTE(Alex): we dont want to parse comments 
inline void
EliminateWhiteSpace(tokenizer * Tokenizer)
{
    for(;;)
    {
        if(CharIsWhiteSpace(Tokenizer->At[0]))
        {
            ++Tokenizer->At;
        }
        else if((Tokenizer->At[0] == '/') && 
                (Tokenizer->At[1] == '/'))
        {
            ParseCStyleComment(Tokenizer);
        }
        else if((Tokenizer->At[0] == '/') && 
                (Tokenizer->At[1] == '/*'))
        {
            ParseCppStyleComment(Tokenizer);
        }
        else
        {
            break;
        }
    }
}



//NOTE(Alex): Make a hashtable of tokens so we can access them faster?
internal token
GetToken(tokenizer * Tokenizer, b32 CheckToken = false)
{
    Assert(Tokenizer->At < ((char*)Tokenizer->File->Content + Tokenizer->File->ContentSize));
    EliminateWhiteSpace(Tokenizer);
    
    token Result = {};
    Result.String.Text = Tokenizer->At;
    Result.String.Count = 1;
    
    switch (Tokenizer->At[0])
    {
        case '\0':{Result.Type = TokenType_EndOfStream;}break;
        case '(':{Result.Type = TokenType_OpenParen;}break;
        case ')':{Result.Type = TokenType_CloseParen;}break;
        case ':':{Result.Type = TokenType_Colon;}break;
        case ';':{Result.Type = TokenType_SemiColon;}break;
        case '[':{Result.Type = TokenType_OpenBracket;}break;
        case ']':{Result.Type = TokenType_CloseBracket;}break;
        case '{':{Result.Type = TokenType_OpenBraces;}break;
        case '}':{Result.Type = TokenType_CloseBraces;}break;
        case '*':{Result.Type = TokenType_Asterisk;}break;
        case '&':{Result.Type = TokenType_Ampersand;}break;
        case '.':{Result.Type = TokenType_Point;}break;
        case ',':{Result.Type = TokenType_Coma;}break;
        case '\"':
        {
            ++Tokenizer->At;
            Result.String.Text = Tokenizer->At;
            Result.Type = TokenType_String;
            
            while(Tokenizer->At[0] && 
                  (Tokenizer->At[0] != '"'))
            {
                if((Tokenizer->At[0] == '\\') && 
                   Tokenizer->At[1])
                {
                    ++Tokenizer->At;
                }
                
                ++Tokenizer->At;
            }
            
            Result.String.Count = SafeTruncateToUint32(Tokenizer->At - Result.String.Text);
            
            if(Tokenizer->At[0] == '"')
            {
                ++Tokenizer->At;
            }
            
        }break;
        default:
        {
            if(IsAlpha(Tokenizer->At[0]))
            {
                Result.Type = TokenType_Identifier;
                while(IsAlpha(Tokenizer->At[0]) || 
                      IsNumber(Tokenizer->At[0]) ||
                      Tokenizer->At[0] == '_')
                {
                    ++Tokenizer->At;
                }
                
                Result.String.Count = SafeTruncateToUint32(Tokenizer->At - Result.String.Text); 
            }
            else if(IsNumber(Tokenizer->At[0]))
            {
                Result.Type = TokenType_Number;
                //TODO(Alex): Expand this to other numeric types
                while(IsNumber(Tokenizer->At[0]) ||
                      Tokenizer->At[0] == '.' || 
                      Tokenizer->At[0] == 'f')
                {
                    ++Tokenizer->At;
                }
                
                Result.String.Count = SafeTruncateToUint32(Tokenizer->At - Result.String.Text); 
            }
            else
            {
                Result.Type = TokenType_Unknown;
            }
        }break;
    }
    
    if(Result.Type >= TokenType_Unknown)
    {
        ++Tokenizer->At;
    }
    
    if(CheckToken)
    {
        Tokenizer->At = Result.String.Text;
    }
    
    return Result;
}

inline void
TokenAdvance(tokenizer * Tokenizer, token * Token)
{
    Tokenizer->At += Token->String.Count; 
}

inline token
CheckNextToken(tokenizer * Tokenizer)
{
    token Result = GetToken(Tokenizer, true);
    return Result;
}

inline b32
TokenIsOfType(tokenizer * Tokenizer, token_type Type)
{
    b32 Result = false;
    token Token = GetToken(Tokenizer);
    Result = (Token.Type == Type);
    
    return Result;
}

internal u32
CheckIntrospectionType(tokenizer * Tokenizer)
{
    parse_introspect_type Result = IntrospectType_invalid;
    if(TokenIsOfType(Tokenizer, TokenType_OpenParen))
    {
        token TokenCategory = GetToken(Tokenizer);
        char * CategoryString = "category";
        char * StructString = "struct";
        char * DataString = "data";
        if(TokenIsIdentifier(&TokenCategory, CategoryString))
        {
            if(TokenIsOfType(Tokenizer, TokenType_Colon))
            {
                token Token  = GetToken(Tokenizer);
                if(Token.Type == TokenType_String)
                {
                    if(StringsAreEqual(&Token.String, &(String(StringLength(StructString), StructString))))
                    {
                        Result = IntrospectType_struct;
                    }
                    else if(StringsAreEqual(&Token.String, &(String(StringLength(DataString), DataString))))
                    {
                        Result = IntrospectType_data;
                    }
                    //TODO(Alex): More Introspect Types
                }
                
            }
        }
        
        for(;;)
        {
            token Token = GetToken(Tokenizer);
            if((Token.Type == TokenType_CloseParen) || 
               (Token.Type == TokenType_EndOfStream))
            {
                break;
            }
        }
    }
    
    if(Result == IntrospectType_invalid)
    {
        //TODO(Alex): Proper error handling
        printf("ERROR: Invalid Introspection Params");
    }
    
    return Result;
}

inline b32
CheckForKeyword(token * Token)
{
    b32 Result = (StringsAreEqual(&Token->String, &(String(StringLength("static"), "static"))) || 
                  StringsAreEqual(&Token->String, &String(StringLength("volatile"), "volatile")) ||
                  StringsAreEqual(&Token->String, &String(StringLength("public"), "public")) ||
                  StringsAreEqual(&Token->String, &String(StringLength("private"),"private" )) ||
                  StringsAreEqual(&Token->String, &String(StringLength("extern"), "extern"))||
                  StringsAreEqual(&Token->String, &String(StringLength("const"), "const")) ||
                  StringsAreEqual(&Token->String, &String(StringLength("friend"), "friend")) ||
                  StringsAreEqual(&Token->String, &String(StringLength("struct"), "struct")) ||
                  StringsAreEqual(&Token->String, &String(StringLength("virtual"), "virtual")));
    
    return Result;
}

inline void
ConcatFormattedText(output_text * OutputText, u32 TextCount, char * Text)
{
    ConcatStringsA(OutputText->Count, 
                   OutputText->Buffer,
                   TextCount, Text,
                   ArrayCount(OutputText->Buffer), OutputText->Buffer);
    
    OutputText->Count += TextCount;
}

//TODO(Alex): probably we could expand AddFormattedChunkToBuffer to have a more general format expansion 
internal void
AddFormattedChunkToBuffer(output_text * OutputText, char * Format, token * Token = 0)
{
    char Text[MAX_BUFFER_SIZE] = {0};
    u32 CharsWritten = 0;
    if(Token)
    {
        CharsWritten = _snprintf_s(Text, sizeof(Text),
                                   Format, 
                                   Token->String.Count, 
                                   Token->String.Text);
    }
    else
    {
        CharsWritten = _snprintf_s(Text, sizeof(Text), Format); 
    }
    
    ConcatFormattedText(OutputText, CharsWritten, Text);
}

internal void
GetHashedToken(token * Other, u32 Flags)
{
    //TODO(Alex): Better Hash Function
    u32 Index = ((13 * Other->Type) + (7 * (u32)Other->String.Text[0])) & (ArrayCount(SeenTokens.Hash) - 1);
    
    hashed_token * Temp = SeenTokens.Hash[Index];
    for(;
        Temp;
        Temp = Temp->NextInHash)
    {
        if(StringsAreEqual(&Temp->Token.String, &Other->String))
        {
            break;
        }
    }
    
    if(!Temp)
    {
        //TODO(Alex): Change this to Memory Buffer from VirtualAlloc if necessary 
        Temp = (hashed_token*)malloc(sizeof(hashed_token));
        
        Temp->Flags = Flags;
        Temp->Token = *Other;
        Temp->NextInHash = SeenTokens.Hash[Index];
        SeenTokens.Hash[Index] = Temp; 
    }
}

inline b32 
CheckForType(tokenizer * Tokenizer, output_text * OutputText, token Token)
{
    b32 Result = false;
    if(CheckForKeyword(&Token))
    {
    }
    else
    {
        Result = true;
        AddFormattedChunkToBuffer(OutputText,"{MetaType_%.*s", &Token);
        
        GetHashedToken(&Token, TokenFlag_MemberType);
        TokenAdvance(Tokenizer, &Token);
    }
    
    return Result;
}


#if 0        
char TextBuffer[256] = {0};
u32 CharsWritten = _snprintf_s(TextBuffer, sizeof(TextBuffer),
                               "MetaType_%.*s", 
                               Token.LetterCount, 
                               Token.Text);

meta_member_type * Temp  = (meta_member_type*)malloc(sizeof(*FirstMetaMemberType));
Temp->Name = (char*)malloc(CharsWritten + 1);
Copy(CharsWritten, TextBuffer, Temp->Name);

Temp->Name[CharsWritten] = 0;

Temp->Next = FirstMetaMemberType;
FirstMetaMemberType = Temp;
#endif


inline void
CheckForPointerReference(tokenizer * Tokenizer, output_text * OutputText)
{
    token Token = CheckNextToken(Tokenizer);
    if(Token.Type == TokenType_Asterisk)
    {
        ConcatFormattedText(OutputText, 5, "Ptr, ");
        TokenAdvance(Tokenizer, &Token);
    }
    else
    {
        if(Token.Type == TokenType_Ampersand)
        {
            TokenAdvance(Tokenizer, &Token);
        }
        ConcatFormattedText(OutputText, 5, ",    ");
    }
}


inline void
CheckForName(tokenizer * Tokenizer, output_text * OutputText, token * StructName)
{
    token Token = GetToken(Tokenizer);
    
    AddFormattedChunkToBuffer(OutputText,"\"%.*s\", ", &Token);
    
    AddFormattedChunkToBuffer(OutputText, "(memory_index)&(((%.*s *)(0))->", StructName);
    AddFormattedChunkToBuffer(OutputText, "%.*s), ", &Token);
    AddFormattedChunkToBuffer(OutputText,"sizeof(((%.*s *)0)->", StructName);
    AddFormattedChunkToBuffer(OutputText, "%.*s), ", &Token);
}

inline void
CheckForArray(tokenizer * Tokenizer, output_text * OutputText)
{
    token ArrayValue = {};
    ArrayValue.Type = TokenType_Number;
    ArrayValue.String.Count = 1;
    ArrayValue.String.Text = "1";
    
    token Token = CheckNextToken(Tokenizer);
    if(Token.Type == TokenType_OpenBracket)
    {
        TokenAdvance(Tokenizer, &Token);
        
        Token = CheckNextToken(Tokenizer);
        if(Token.Type == TokenType_Identifier || 
           Token.Type == TokenType_Number)
        {
            ArrayValue = Token;
            TokenAdvance(Tokenizer, &Token);
            
            Token = CheckNextToken(Tokenizer);
        }
        
        TokenAdvance(Tokenizer, &Token);
    }
    
    AddFormattedChunkToBuffer(OutputText, "%.*s", &ArrayValue);
}

inline token
CheckForEnd(tokenizer * Tokenizer, output_text * OutputText)
{
    token Result = {};
    for(;;)
    {
        Result = CheckNextToken(Tokenizer);
        if(Result.Type == TokenType_SemiColon)
        {
            //TokenAdvance(Tokenizer, &Token);
            
            char Text[] = "},\n";
            ConcatFormattedText(OutputText, ArrayCount(Text) - 1, Text);
            break;
        }
    }
    
    return Result;
}


internal void
ParseStructMembers(tokenizer * Tokenizer, token * NameStruct)
{
    output_text OutputText = {};
    b32 EndOfStruct = false;
    for(;
        !EndOfStruct;
        )
    {
        token Token = CheckNextToken(Tokenizer);
        switch(Token.Type)
        {
            case TokenType_CloseBraces:
            {
                --Tokenizer->StackLevel;
                if(Tokenizer->StackLevel == 0)
                {
                    EndOfStruct = true;
                }
            }break;
            case TokenType_OpenBraces:
            {
                ++Tokenizer->StackLevel;
            }break;
            case TokenType_Identifier:
            {
                //TODO(Alex): CheckForValue(Tokenizer);?
                if(CheckForType(Tokenizer, &OutputText, Token))
                {
                    CheckForPointerReference(Tokenizer, &OutputText);
                    CheckForName(Tokenizer, &OutputText, NameStruct);
                    CheckForArray(Tokenizer, &OutputText);
                    Token = CheckForEnd(Tokenizer, &OutputText);
                }
            };
            default:
            {
            }break;
        }
        
        TokenAdvance(Tokenizer, &Token);
    }
    
    printf(OutputText.Buffer);
}


internal void
ParseStruct(tokenizer * Tokenizer)
{
    token StructName = GetToken(Tokenizer);
    if(TokenIsOfType(Tokenizer, TokenType_OpenBraces))
    {
        ++Tokenizer->StackLevel;
        
        GetHashedToken(&StructName, TokenFlag_IntrospectedStruct);
        printf("member_definition DefinitionOf_%.*s [] =\r\n{\r\n", 
               StructName.String.Count, 
               StructName.String.Text);
        
        ParseStructMembers(Tokenizer, &StructName);
        printf("};\n");
    }
    else
    {
        printf("ERROR: Not definition of struct");
    }
}

/*First i'll need to get to the decimal point to know  
if there's decimal point you know where to start and if theres no decimal point we can deliver an integer
.9
.09
.009
.0009
9.0
90.0
900.0
9000.0
90000.0
yeah because the user could cast the number to its value
9.92f  floating point float
9.4 floating point double
9 u32 integer value
*/

internal r64
GetR64FromString(string * String)
{
    r64 Result = 0;
    char * Char = String->Text;
    
    r64 Base = 10;
    r64 Exponent = 0;
    
    for(u32 Index = 0;
        Index < (String->Count - 1);
        ++Index)
    {
        if(*Char == '.')
        {
            break;
        }
        
        ++Exponent;
        ++Char;
    }
    
    //TODO(Alex): Faster way of abstracting floating point numbers
    Char = String->Text;
    for(u32 Index = 0;
        Index < String->Count;
        ++Index)
    {
        switch(*Char)
        {
            case '0':
            {
                Result += (0.0 * pow(Base, Exponent--));
            }break;
            case '1':
            {
                Result += (1.0 * pow(Base, Exponent--));
            }break;
            case '2':
            {
                Result += (2.0 * pow(Base, Exponent--));
            }break;
            case '3':
            {
                Result += (3.0 * pow(Base, Exponent--));
            }break;
            case '4':
            {
                Result += (4.0 * pow(Base, Exponent--));
            }break;
            case '5':
            {
                Result += (5.0 * pow(Base, Exponent--));
            }break;
            case '6':
            {
                Result += (6.0 * pow(Base, Exponent--));
            }break;
            case '7':
            {
                Result += (7.0 * pow(Base, Exponent--));
            }break;
            case '8':
            {
                Result += (8.0 * pow(Base, Exponent--));
            }break;
            case '9':
            {
                Result += (9.0 * pow(Base, Exponent--));
            }break;
        }
        ++Char;
    }
    
    return Result;
}

internal u32
GetU32FromString(string * String)
{
    r64 R64 = GetR64FromString(String);
    u32 Result = SafeTruncateToUint32((u64)R64); 
    return Result;
}

struct matrix_dim
{
    u32 Cx;
    u32 Cy;
};

internal matrix_dim 
GetMatrixDim(tokenizer * Tokenizer, char * ID)
{
    matrix_dim Result = {};
    token Matrix  = GetToken(Tokenizer);
    if(TokenIsIdentifier(&Matrix, ID))
    {
        if(TokenIsOfType(Tokenizer
                         , TokenType_OpenParen))
        {
            for(;;)
            {
                token Token = GetToken(Tokenizer);
                if(Token.Type == TokenType_Number)
                {
                    if(!Result.Cx)
                    {
                        Result.Cx = GetU32FromString(&Token.String);
                    }
                    else if(!Result.Cy)
                    {
                        Result.Cy = GetU32FromString(&Token.String);
                    }
                }
                
                if((Token.Type == TokenType_CloseParen) || 
                   (Token.Type == TokenType_EndOfStream))
                {
                    break;
                }
            }
        }
    }
    return Result;
}

internal void 
METADefMatrix(tokenizer * Tokenizer, matrix_dim Dim, char * ID)
{
    printf("#define META_MATRIX_%s(Name, Arena) matrix_2 Name = MatrixXY(Arena, %i, %i);{", 
           ID, 
           Dim.Cx, Dim.Cy);
    
    for(u32 Index = 0;
        Index < (Dim.Cx * Dim.Cy);
        )
    {
        token Token = GetToken(Tokenizer);
        if(Token.Type == TokenType_Number)
        {
            printf("Name.Data[%i] = %.*s;", Index, Token.String.Count, Token.String.Text);
            ++Index;
        }
        else if((Token.Type == TokenType_CloseBraces) || 
                (Token.Type == TokenType_EndOfStream))
        {
            break;
        }
    }
    printf("}\n");
}


//TODO(Alex): Dump data directly to a .h file
internal void
ParseData(tokenizer * Tokenizer)
{
    if(TokenIsOfType(Tokenizer, TokenType_OpenBraces))
    {
        char * XID = "X";
        char * YID = "y";
        
        matrix_dim XDim = GetMatrixDim(Tokenizer, XID);
        matrix_dim yDim = GetMatrixDim(Tokenizer, YID);
        
        METADefMatrix(Tokenizer, XDim, XID);
        METADefMatrix(Tokenizer, yDim, YID);
    }
    else
    {
        printf("ERROR: Not definition of struct");
    }
}


internal void
ParseIntrospection(tokenizer * Tokenizer)
{
    switch(CheckIntrospectionType(Tokenizer))
    {
        case IntrospectType_struct:
        {
            token Token = GetToken(Tokenizer);
            if(TokenIsIdentifier(&Token, "struct") ||
               TokenIsIdentifier(&Token, "union"))
            {
                ParseStruct(Tokenizer);
            }
        }break;
        case IntrospectType_data:
        {
            ParseData(Tokenizer);
        }break;
        //TODO(Alex): More introspection types
    }
}


internal void 
LexFile(char * FileName)
{
    debug_file_content File = DEBUGPlatformReadEntireFile(FileName);
    
    tokenizer Tokenizer = {};
    Tokenizer.Tokenizing = false;
    Tokenizer.File = &File;
    Tokenizer.At = (char*)File.Content;
    
    for(;
        Tokenizer.At < ((char*)File.Content + File.ContentSize);
        )
    {
        token Token = GetToken(&Tokenizer);
        switch(Token.Type)
        {
            default:
            {
                
            }break;
            case TokenType_Identifier:
            {
                if(TokenIsIdentifier(&Token, "Introspection"))
                {
                    ParseIntrospection(&Tokenizer);
                }
            }break;
            case TokenType_EndOfStream:
            {
                break;
            }break;
        }
    }
}

int main(int ArgCount, char ** Args)
{
    LexFile("ex2data1.txt");
}


#if 0
int 
main(int ArgCount, char **Args)
{
    //LexFile("handmade_simulation_region.h");
    //LexFile("handmade_math.h");
    //LexFile("handmade_platform.h");
    
    //TODO(Alex): Implement this?
    //GetHashedTokenWithFlag(TokenFlag_IntrospectedStruct);
    //AddFormattedChunkToBuffer(&OutputText, "#define Introspected_Struct_List()\\\nenum introspected_struct_list\\\n{\\\n");
    printf("#define DEBUG_SWITCH_MEMBER_DEFS(DepthLevel)\\\n");
    
    for(u32 BucketIndex = 0;
        BucketIndex < ArrayCount(SeenTokens.Hash);
        ++BucketIndex)
    {
        hashed_token * Hashed = SeenTokens.Hash[BucketIndex];
        for(;
            Hashed;
            Hashed = Hashed->NextInHash)
        {
            if(Hashed->Flags & TokenFlag_IntrospectedStruct)
            {
                //NOTE(Alex): Debug switch members
                printf("case MetaType_%.*s:{u32 BytesWritten = _snprintf_s(At, BufferSize, BufferSize, \"%s:\", Def->VarName); DEBUGTextLine(TextBuffer); At += BytesWritten; DumpStructDataOut(ArrayCount(DefinitionOf_%.*s), DefinitionOf_%.*s, MemberOffset, DepthLevel);}break;",
                       Hashed->Token.LetterCount, Hashed->Token.Text,
                       "%s",
                       Hashed->Token.LetterCount, Hashed->Token.Text,
                       Hashed->Token.LetterCount, Hashed->Token.Text);
                
                printf("case MetaType_%.*sPtr:{u32 BytesWritten = _snprintf_s(At, BufferSize, BufferSize, \"%s:\", Def->VarName); DEBUGTextLine(TextBuffer); At += BytesWritten; DumpStructDataOut(ArrayCount(DefinitionOf_%.*s), DefinitionOf_%.*s, (void*)(*(u64*)MemberOffset), DepthLevel);}break;", 
                       Hashed->Token.LetterCount, Hashed->Token.Text,
                       "%s",
                       Hashed->Token.LetterCount, Hashed->Token.Text,
                       Hashed->Token.LetterCount, Hashed->Token.Text);
                
                //AddFormattedChunkToBuffer(&OutputText, "IntroStruct_%.*s,\\\n", &Hashed->Token);
            }
        }
    }
    //AddFormattedChunkToBuffer(&OutputText, "IntroStruct_Count,\\\n};\n\n");
    
    //NOTE(Alex): meta:type_generated.cpp
    output_text OutputText = {};
    AddFormattedChunkToBuffer(&OutputText, "#define META_MEMBER_TYPE_ENUM()\\\nenum member_definition_type\\\n{\\\n");
    for(u32 BucketIndex = 0;
        BucketIndex < ArrayCount(SeenTokens.Hash);
        ++BucketIndex)
    {
        hashed_token * Hashed = SeenTokens.Hash[BucketIndex];
        for(;
            Hashed;
            Hashed = Hashed->NextInHash)
        {
            AddFormattedChunkToBuffer(&OutputText, "MetaType_%.*s,\\\n", &Hashed->Token);
            AddFormattedChunkToBuffer(&OutputText, "MetaType_%.*sPtr,\\\n", &Hashed->Token);
        }
    }
    AddFormattedChunkToBuffer(&OutputText, "};\n");
    DEBUGPlatformWriteEntireFile("handmade_meta_type_generated.h", OutputText.Count, OutputText.Buffer);
    printf("\0");
    
    return 0;
}

#endif




























