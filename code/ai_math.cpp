/*
Alejandro Armenta Ayala
ai_math.cpp
*/


/*NOTE(Alex):
Vector Movement: 

[A][B][C][D][E]

[A]
[B]
[C]
[D]
[E]

Matrix movement: 

[A, B]  
[C, D]
[E, F]
*/


inline u32  
MatrixCount(matrix_2 * Matrix)
{
    u32 Result = Matrix->CountY * Matrix->CountX;
    return Result;
}

internal void 
ZeroData(void * Data, u32 Size)
{
    r64 * At = (r64*)Data;
    u32 Count = 0;
    
    while(Count < Size) 
    {
        *At++ = 0;
        ++Count;
    }
}


#define MatrixX(AIState, X) MatrixXY((AIState), X, 1)
#define MatrixY(AIState, Y) MatrixXY((AIState), 1, Y)

internal matrix_2  
MatrixXY(memory_arena * Arena, u32 SizeX, u32 SizeY)
{
    //NOTE(Alex): Assert I'm pushing on to the same arena on both pushes!
    Assert(Arena);
    matrix_2 Result = {};
    Result.CountX = SizeX;
    Result.CountY = SizeY;
    Result.Data = PushArray(Arena, Result.CountX * Result.CountY, r64);
    
    return Result;
}

internal matrix_2
Ones(u32 CountX, u32 CountY)
{
    matrix_2 Result = MatrixXY(&GetAIState()->TranArena, CountX, CountY);
    u32 Count = MatrixCount(&Result);
    for(u32 Index = 0;
        Index < Count;
        ++Index)
    {
        Result.Data[Index] = 1.0; 
    }
    return Result;
}

internal matrix_2
Zeroes(u32 CountX, u32 CountY)
{
    matrix_2 Result = MatrixXY(&GetAIState()->TranArena, CountX, CountY);
    u32 Count = MatrixCount(&Result);
    for(u32 Index = 0;
        Index < Count;
        ++Index)
    {
        Result.Data[Index] = 0.0; 
    }
    return Result;
}

//TODO(Alex): Do we want other math set functions that don't allocate memory?
internal matrix_2
Zeroes(matrix_2 * out_matrix)
{
    Assert(out_matrix->Data);
    u32 Count = MatrixCount(out_matrix);
    for(u32 Index = 0;
        Index < Count;
        ++Index)
    {
        out_matrix->Data[Index] = 0.0; 
    }
    ;
}


internal matrix_2
Log(matrix_2 A)
{
    matrix_2 Result = MatrixXY(&GetAIState()->TranArena, A.CountX, A.CountY);
    u32 Count = MatrixCount(&Result);
    for(u32 Index = 0;
        Index < Count;
        ++Index)
    {
        Result.Data[Index] = log(A.Data[Index]); 
    }
    
    return Result;
}

//
//NOTE(Alex): Operator Overloading
//

inline matrix_2 
operator-(matrix_2 A, matrix_2 B)
{
    Assert((A.CountX == B.CountX) && 
           (B.CountY == B.CountY));
    
    matrix_2 Result = MatrixXY(&GetAIState()->TranArena, A.CountX, A.CountY);
    u32 Count = MatrixCount(&A);
    for(u32 Index = 0;
        Index < Count;
        ++Index)
    {
        Result.Data[Index] = A.Data[Index] - B.Data[Index]; 
    }
    
    return Result;
}


inline matrix_2 
operator+(matrix_2 A, matrix_2 B)
{
    Assert((A.CountX == B.CountX) && 
           (B.CountY == B.CountY));
    
    matrix_2 Result = MatrixXY(&GetAIState()->TranArena, A.CountX, A.CountY);
    u32 Count = MatrixCount(&A);
    for(u32 Index = 0;
        Index < Count;
        ++Index)
    {
        Result.Data[Index] = A.Data[Index] + B.Data[Index]; 
    }
    
    return Result;
}


inline matrix_2 
operator-(matrix_2 A)
{
    Assert(A.CountX && A.CountY);
    matrix_2 Result = MatrixXY(&GetAIState()->TranArena, A.CountX, A.CountY);
    u32 Count = MatrixCount(&A);
    for(u32 Index = 0;
        Index < Count;
        ++Index)
    {
        Result.Data[Index] = -A.Data[Index]; 
    }
    
    return Result;
}


inline matrix_2 
operator*(matrix_2 A, r64 Scalar)
{
    Assert(A.CountX && A.CountY);
    matrix_2 Result = MatrixXY(&GetAIState()->TranArena, A.CountX, A.CountY);
    u32 Count = MatrixCount(&A);
    for(u32 Index = 0;
        Index < Count;
        ++Index)
    {
        Result.Data[Index] = A.Data[Index] * Scalar; 
    }
    
    return Result;
}


inline matrix_2
operator*(r64 Scalar, matrix_2 A)
{
    matrix_2 Result = A * Scalar;
    return Result;
}


inline matrix_2 
Hadamard(matrix_2 *  A, matrix_2 * B)
{
    Assert((A->CountX == B->CountX) && 
           (B->CountY == B->CountY));
    
    matrix_2 Result = MatrixXY(&GetAIState()->TranArena, A->CountX, A->CountY);
    u32 Count = MatrixCount(A);
    for(u32 Index = 0;
        Index < Count;
        ++Index)
    {
        Result.Data[Index] = A->Data[Index] * B->Data[Index]; 
    }
    
    return Result;
}


internal matrix_2  
AddBiasUnit(memory_arena * Arena, matrix_2 * A)
{
    matrix_2 Result = {};
    Result.CountX = A->CountX;
    Result.CountY = A->CountY + 1;
    Result.Data = PushArray(Arena, Result.CountX * Result.CountY, r64);
    
    u32 MainDataIndex = 0;
    for(u32 X = 0;
        X < Result.CountX;
        ++X)
    {
        for(u32 Y = 0;
            Y < Result.CountY;
            ++Y)
        {
            u32 Index = X * Result.CountY + Y;
            if(Y == 0)
            {
                Result.Data[Index] = 1.0; 
            }
            else
            {
                Result.Data[Index] = A->Data[MainDataIndex++]; 
            }
        }
    }
    
    return Result;
}

internal matrix_2    
InnerProduct(matrix_2 * A, matrix_2 * B)
{
    Assert(A->CountY == B->CountX);
    
    matrix_2 Result = {};
    Result.CountX = A->CountX;
    Result.CountY = B->CountY;
    Result.Data = PushArray(&GetAIState()->TranArena, Result.CountX * Result.CountY, r64);
    
    u32 Count = MatrixCount(&Result);
    for(u32 Index = 0;
        Index < Count;
        ++Index)
    {
        r64 Product = 0;
        for(u32 Y = 0;
            Y < A->CountY;
            ++Y)
        {
            //TODO(Alex): Make a faster linear combination.
            r64 AE = A->Data[(u32)(Index / Result.CountY) * A->CountY + Y];
            r64 BE = B->Data[(Y * B->CountY) + (Index % B->CountY)];
            
            Product += AE * BE;
        }
        
        Result.Data[Index] = Product;
    }
    return Result;
}

internal matrix_2
Sigmoid(matrix_2 * A)
{
    ai_state * AIState = GetAIState();
    matrix_2 Result = MatrixXY(&AIState->TranArena, A->CountX, A->CountY);
    
    u32 Count = MatrixCount(&Result);
    for(u32 Index = 0;
        Index < Count;
        ++Index)
    {
        r64 Value = A->Data[Index];
        Result.Data[Index] = (1 / (1 + exp(-Value))); 
    }
    
    return Result;
}

//NOTE(Alex): This works with matrix row Offsets
//This assumes out_dest is already initialized with the proper row_count y column_count
internal void
CopyMatrix(matrix_2 * out_dest, matrix_2 * in_source, u32 ix = 0)
{
    u32 Count = MatrixCount(out_dest);
    for(u32 Index = 0;
        Index < Count;
        ++Index)
    {
        out_dest->Data[Index] = in_source->Data[ix * in_source->CountY + Index];
    }
}


inline matrix_2
Transpose(matrix_2 * A)
{
    Assert(A->CountX && A->CountY);
    matrix_2 Result = {};
    Result.CountX = A->CountY;
    Result.CountY = A->CountX;
    Result.Data  = PushArray(&GetAIState()->TranArena, Result.CountX * Result.CountY, r64);
    
    u32 Index = 0;
    for(u32 X = 0;
        X < A->CountX;
        ++X)
    {
        for(u32 Y = 0;
            Y < A->CountY;
            ++Y)
        {
            Result.Data[Index++] = A->Data[(Y * A->CountY) + X];
        }
    }
    
    return Result;
}

inline r64 
Summation(matrix_2 * A)
{
    Assert(A->CountX && A->CountY);
    r64 Result = 0;
    
    u32 Count = MatrixCount(A);
    for(u32 Index = 0;
        Index < Count;
        ++Index)
    {
        Result += A->Data[Index]; 
    }
    
    return Result;
}


//
//NOTE(Alex): Single Real Value operations 
//

inline r64
AbsValue(r64 Value)
{
    r64 Result = fabs(Value);  
    return Result;
}

//
//NOTE(Alex): Regression Functions
//

inline b32 
IsValidSet(ai_matrix_set * Set)
{
    b32 Result = (Set->Matrix  && (Set->MaxRowIndex >= Set->MinRowIndex)); 
    return Result;
}

internal matrix_2
GetMatrixFromSet(ai_state * AIState, ai_matrix_set * Set, u32 ix = 0)
{
    matrix_2 Result = {};
    if(IsValidSet(Set))
    {
        u32 cx, cy;
        cx = GET_SET_COUNT(Set);
        cy = Set->Matrix->CountY;
        Result = MatrixXY(&AIState->TranArena, cx, cy);
        CopyMatrix(&Result, Set->Matrix, ix);
    }
    else
    {
        Result = AIState->X;
    }
    
    return Result;
}

//NOTE(Alex): This will be wrong for test set so that means that we want to do some stubs
internal r64 
CostOfTheta_(ai_state * AIState, matrix_2 * X, matrix_2 * y, r64 RegParam)
{
    r64 Result = 0;
    
    matrix_2 z = InnerProduct(X, &AIState->Theta);
    matrix_2 Hyp = Sigmoid(&z);
    
    matrix_2 LogY1 = Log(Hyp);
    matrix_2 ProductY1 = Hadamard(y, &LogY1);
    
    matrix_2 One = Ones(Hyp.CountX, Hyp.CountY);
    matrix_2 LogY0 = Log(One - Hyp);
    matrix_2 OneMinusy = One - *y;
    matrix_2 ProductY0 = Hadamard(&OneMinusy, &LogY0);
    matrix_2 Sum = -ProductY1 - ProductY0;
    
    r64 SumTheta = 0;
    u32 Count = MatrixCount(&AIState->Theta);
    for(u32 Index = 1;
        Index < Count;
        ++Index)
    {
        SumTheta += (AIState->Theta.Data[Index] * AIState->Theta.Data[Index]); 
    }
    
    r64 RegExp = 2.0 * AIState->RegParam * AIState->InverseTrainingSetSize * SumTheta;
    
    Result = ((AIState->InverseTrainingSetSize * Summation(&Sum)) + RegExp);
    return Result;
}

internal r64
CostOfTheta(ai_state * AIState, ai_matrix_set * XSet, ai_matrix_set * ySet, r64 RegParam, u32 ix = 0)
{
    r64 Result;
    matrix_2 X = GetMatrixFromSet(AIState, XSet, ix);
    matrix_2 y = GetMatrixFromSet(AIState, ySet, ix);
    Result = CostOfTheta_(AIState, &X, &y, RegParam);
    return Result;
}

//NOTE(Alex): Get gradient is only for training set so we keep set/matrix transformation inline
internal matrix_2
GetGradient(ai_state * AIState, ai_matrix_set * XSet, ai_matrix_set * ySet, r64 RegParam)
{
    matrix_2 SubX = GetMatrixFromSet(AIState, XSet);
    matrix_2 Suby = GetMatrixFromSet(AIState, ySet);
    //NOTE(Alex) z = m x 1;
    matrix_2 z = InnerProduct(&SubX, &AIState->Theta);
    //NOTE(Alex) Hyp = m x 1;
    matrix_2 Hyp = Sigmoid(&z);
    //NOTE(Alex) z = m x 1;
    matrix_2 Diff = Hyp - Suby;
    matrix_2 DiffT = Transpose(&Diff);
    matrix_2 UnscaledR = InnerProduct(&DiffT, &SubX);   
    //NOTE(Alex): Result = n x 1 
    matrix_2 Result = AIState->InverseTrainingSetSize * UnscaledR;
    Result = Transpose(&Result); 
    
    //NOTE(Alex): Regularization Parameter
    Result = (Result + (RegParam * AIState->InverseTrainingSetSize * AIState->Theta));
    return Result; 
}



#if 0
//TODO(Alex): Element Selector? 
inline matrix_2 
GetVecFromMatrix(matrix_2 A, u32 VecIndex, b32 ByColumns)
{
    Assert(A.CountX && A.CountY);
    matrix_2 Result = ByColumns ? MatrixY(&GetAIState()->TranArena, A.CountY) : MatrixX(&GetAIState()->TranArena, A.CountX);
    
    for(u32 Index = 0;
        Index < MatrixCount(&A);
        ++Index)
    {
        (ByColumns)
        {
            Result.Data[Index] = A.Data[A.CountY * Index]; 
        }
        else
        {
            Result.Data[Index] = A.Data[A.CountY * VecIndex + Index]; 
        }
    }
    
    return Result;
}
#endif


#if 0
internal matrix_2
NNHypothesis(matrix_2 * A0, matrix_2 * Theta0, matrix_2 * Theta1)
{
    matrix_2 A0 = MatrixX(InputLayerSize);
    matrix_2 Theta0 = MatrixXY(HiddenLayerSize, InputLayerSize + 1);
    matrix_2 Theta1 = MatrixXY(OutputLayerSize, HiddenLayerSize + 1);
    
    matrix_2 A0Bias = AddBiasUnit(&A0);
    matrix_2 Z0 = InnerProduct(&Theta0, &A0Bias);
    matrix_2 A1 = Sigmoid(&Z0);
    
    matrix_2 A1Bias = AddBiasUnit(&A1);
    matrix_2 Z1 = InnerProduct(&Theta1, &A1Bias);
    matrix_2 A2 = Sigmoid(&Z1);
    
    return A2;
}
#endif



