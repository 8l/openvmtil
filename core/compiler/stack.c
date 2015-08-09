
#include "../../includes/cfrtil.h"

// keep this stack code correlated with core/cstack.c

void
_Compile_Stack_Drop ( int32 stackReg )
{
    Compile_SUBI ( REG, stackReg, 0, sizeof (int32 ), BYTE ) ; // 3 bytes long
}

void
_Compile_Stack_DropN ( int32 stackReg, int n )
{
    Compile_SUBI ( REG, stackReg, 0, n * sizeof ( int32 ), 0 ) ;
}

// RDrop : drop the sytem return, ESP based, stack
// system stack is backward to cfrTil - grows downward

void
_Compile_DropN_ESP ( int n )
{
    Compile_ADDI ( REG, ESP, 0, n * sizeof ( int32 ), 0 ) ;
}

void
_Compile_SetStackN_WithObject ( int32 stackReg, int32 n, int32 obj ) // runtime
{
    //_Compile_MoveImm ( int32 direction, int32 rm, int32 sib, int32 disp, int32 imm, int32 operandSize )
    _Compile_MoveImm ( MEM, stackReg, 0, n * CELL, obj, CELL ) ;
}

void
_Compile_Stack_Push ( int32 stackReg, int32 obj ) // runtime
{
    Compile_ADDI ( REG, stackReg, 0, sizeof (int32 ), BYTE ) ;
    //_Compile_MoveImm ( MEM, stackReg, 0, 0, obj, CELL ) ;
    _Compile_SetStackN_WithObject ( DSP, 0, obj ) ;
}

void
_Compile_Move_StackN_To_Reg ( int32 reg, int32 stackReg, int32 index )
{
    _Compile_Move_Rm_To_Reg ( reg, stackReg, index * CELL ) ;
}

void
_Compile_Move_Reg_To_StackN ( int32 stackReg, int32 index, int32 reg )
{
    _Compile_Move_Reg_To_Rm ( stackReg, index * CELL, reg ) ;
}

void
_Compile_Move_StackNRm_To_Reg ( int32 reg, int32 stackReg, int32 index )
{
    _Compile_Move_StackN_To_Reg ( reg, stackReg, index ) ;
    _Compile_Move_Rm_To_Reg ( reg, reg, 0 ) ; // *x
}

//  ( reg sreg n ) mov_reg_to_stacknMemAddr

void
_Compile_Move_Reg_To_StackNRm_UsingReg ( int32 stackReg, int32 index, int32 reg, int32 ureg )
{
    _Compile_Move_StackN_To_Reg ( ureg, stackReg, index ) ;
    _Compile_Move_Reg_To_Rm ( ureg, 0, reg ) ;
}

// remember to use a negative number to access an existing stack item

void
_Compile_Stack_PushReg ( int32 stackReg, int32 reg )
{
    Compile_ADDI ( REG, stackReg, 0, sizeof (int32 ), BYTE ) ;
    _Compile_Move_Reg_To_StackN ( stackReg, 0, reg ) ;
}

void
_Compile_Stack_PopToReg ( int32 stackReg, int32 reg )
{
    _Compile_Move_StackN_To_Reg ( reg, stackReg, 0 ) ;
    Compile_SUBI ( REG, stackReg, 0, sizeof (int32 ), BYTE ) ;
}

void
Compile_Stack_PushEAX ( int32 stackReg )
{
    _Compile_Stack_PushReg ( stackReg, EAX ) ;
}

void
Compile_Move_TOS_To_EAX ( int32 stackReg )
{
    _Compile_Move_StackN_To_Reg ( EAX, stackReg, 0 ) ;
}

void
Compile_Move_EAX_To_TOS ( int32 stackReg )
{
    _Compile_Move_Reg_To_StackN ( stackReg, 0, EAX ) ;
}

void
Compile_Pop_To_EAX ( int32 stackReg )
{
    _Compile_Stack_PopToReg ( stackReg, EAX ) ;
}

void
Compile_Pop_ToEAX_AndCall ( int32 stackReg )
{
    _Compile_Stack_PopToReg ( stackReg, EAX ) ;
    _Compile_CallEAX ( ) ;
}

void
Compile_MoveImm_To_TOS ( int32 stackReg, int32 imm, int32 size )
{
    _Compile_MoveImm ( MEM, stackReg, 0, 0, imm, size ) ;
}

#if 0
// for n < 64

void
_Compile_Stack_NDup ( int32 stackReg )
{
    _Compile_Move_Rm_To_Reg ( EAX, stackReg, 0 ) ;
    Compile_ADDI ( REG, stackReg, 0, sizeof (int32 ), BYTE ) ; // 3 bytes long
    _Compile_Move_Reg_To_Rm ( stackReg, 0, EAX ) ;
}
#endif

void
_Compile_Stack_Dup ( int32 stackReg )
{
    Compiler * compiler = _Context_->Compiler0 ;
    int optFlag = CheckOptimizeOperands ( compiler, 3 ) ;
    if ( optFlag == OPTIMIZE_DONE ) return ;
    else
    {
        _Compile_Move_Rm_To_Reg ( EAX, stackReg, 0 ) ;
        Word *zero = ( Word* ) Compiler_WordStack ( compiler, 0 ) ;
        zero->StackPushRegisterCode = Here ;
        Compile_ADDI ( REG, stackReg, 0, sizeof (int32 ), BYTE ) ; // 3 bytes long
        _Compile_Move_Reg_To_Rm ( stackReg, 0, EAX ) ;
    }
}

// pick is from stack below top index

void
_Compile_Stack_Pick ( int32 stackReg ) // pick
{
    _Compile_Move_Rm_To_Reg ( EDX, stackReg, 0 ) ;
    Compile_NOT ( REG, EDX, 0, 0 ) ;
    _Compile_Move ( REG, EDX, stackReg, _CalculateSib ( SCALE_CELL, EDX, ESI ), 0 ) ; // n
    _Compile_Move_Reg_To_Rm ( stackReg, 0, EDX ) ;
}

void
_Compile_Stack_Swap ( int32 stackReg )
{
    _Compile_Move_Rm_To_Reg ( ECX, stackReg, 0 ) ;
    _Compile_Move_Rm_To_Reg ( EDX, stackReg, - CELL ) ;
    _Compile_Move_Reg_To_Rm ( stackReg, - CELL, ECX ) ;
    _Compile_Move_Reg_To_Rm ( stackReg, 0, EDX ) ;
}

void
Compile_DataStack_PushEAX ( )
{
    Compile_Stack_PushEAX ( DSP ) ;
}

void
_Compile_Esp_Push ( int32 value )
{
    _Compile_MoveImm_To_Reg ( EAX, value, CELL ) ;
    _Compile_PushReg ( EAX ) ;
}

void
Compile_DspPop_EspPush ( )
{
    _Compile_Stack_PopToReg ( DSP, EAX ) ;
    // _Compile_Stack_Push_Reg ( ESP, ECX ) ; // no such op
    _Compile_PushReg ( EAX ) ;
}

#if 0
void
Compile_EspPop_DspPush ( )
{
    _Compile_PopToReg ( EAX ) ; // intel pop is pop esp
    Compile_DataStack_PushEAX ( ) ; // no such op
}
#endif

