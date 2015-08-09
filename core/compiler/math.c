#include "../../includes/cfrtil.h"

// X variable op compile for group 5 opCodes - inc/dec - ia32 

void
Compile_Minus ( Compiler * compiler )
{
    Compile_X_Group1 ( compiler, SUB, ZERO, N ) ;
}

void
Compile_Plus ( Compiler * compiler )
{
    Compile_X_Group1 ( compiler, ADD, ZERO, N ) ;
}

#if 0

void
Compile_X_Group3 ( Compiler * compiler, int32 op )
{
    Word *zero = Compiler_WordStack ( compiler, 0 ) ; // refers to this current multiply insn word
    if ( CheckOptimizeOperands ( compiler, 6 ) ) // 6 : especially for the factorial - qexp, bexp 
    {
        // Compile_IMUL ( mod, rm, sib, disp, imm, size )
        // always uses EAX as the first operand and destination of the operation
        if ( compiler->Optimizer->OptimizeFlag & OPTIMIZE_IMM )
        {
            // IMULI : intel insn can't mult to tos in place must use reg ...
            // _Compile_IMUL ( cell mod, cell reg, cell rm, cell sib, cell disp )
            _Compile_Group3 ( op, compiler->Optimizer->Optimize_Mod, EAX, compiler->Optimizer->Optimize_Rm, 0, compiler->Optimizer->Optimize_Disp,
                compiler->Optimizer->Optimize_Imm, 0 ) ;
        }
        else
        {
            //_Compile_IMUL ( cell mod, cell reg, cell rm, cell sib, cell disp )
            _Compile_Group3 ( op, compiler->Optimizer->Optimize_Mod, EAX, compiler->Optimizer->Optimize_Rm, 0,
                compiler->Optimizer->Optimize_Disp ) ;
        }
        zero->StackPushRegisterCode = Here ;
        _Compile_Stack_PushReg ( DSP, EAX ) ;
    }
    else
    {
        Compile_Pop_To_EAX ( DSP ) ;
        //Compile_IMUL ( cell mod, cell reg, cell rm, sib, disp, imm, size )
        _Compile_Group3 ( op, MEM, EAX, DSP, 0, 0 ) ;
        //zero->StackPushRegisterCode = Here ;
        Compile_Move_EAX_To_TOS ( DSP ) ;
    }
}

void
Compile_Multiply ( Compiler * compiler )
{
    Compile_X_Group3 ( compiler, MUL ) ;
}
#endif

void
Compile_IMultiply ( Compiler * compiler )
{
    //if ( CheckOptimizeOperands ( compiler, 6 ) ) // 6 : especially for the factorial - qexp, bexp 
    int optFlag = CheckOptimizeOperands ( compiler, 6 ) ;
    if ( optFlag == OPTIMIZE_DONE ) return ;
    else if ( optFlag )
    {
        // Compile_IMUL ( mod, rm, sib, disp, imm, size )
        // always uses EAX as the first operand and destination of the operation
        if ( ! ( compiler->Optimizer->OptimizeFlag & OPTIMIZE_REGISTER ) ) compiler->Optimizer->Optimize_Reg = EAX ;
        else
        {
            compiler->Optimizer->Optimize_Rm = compiler->Optimizer->Optimize_SrcReg ;
            compiler->Optimizer->Optimize_Reg = compiler->Optimizer->Optimize_DstReg ;
        }
        if ( compiler->Optimizer->OptimizeFlag & OPTIMIZE_IMM )
        {
            // if ( imm == 0 ) skip this ; // TODO 
            // IMULI : intel insn can't mult to tos in place must use reg ...
            //_Compile_IMULI ( int32 mod, int32 reg, int32 rm, int32 sib, int32 disp, int32 imm, int32 size )
            _Compile_IMULI ( compiler->Optimizer->Optimize_Mod, compiler->Optimizer->Optimize_Reg, compiler->Optimizer->Optimize_Rm, 0, compiler->Optimizer->Optimize_Disp,
                compiler->Optimizer->Optimize_Imm, 0 ) ;
        }
        else
        {
            //_Compile_IMUL ( cell mod, cell reg, cell rm, cell sib, cell disp )
            _Compile_IMUL ( compiler->Optimizer->Optimize_Mod, compiler->Optimizer->Optimize_Reg, compiler->Optimizer->Optimize_Rm, 0,
                compiler->Optimizer->Optimize_Disp ) ;
        }
        if ( GetState ( _Context_, C_SYNTAX ) ) _Stack_DropN ( _Context_->Compiler0->WordStack, 2 ) ;
        Word * zero = Compiler_WordStack ( compiler, 0 ) ;
        zero->StackPushRegisterCode = Here ;
        if ( compiler->Optimizer->Optimize_Rm == DSP ) Compile_Move_EAX_To_TOS ( DSP ) ;
        else _Compile_Stack_PushReg ( DSP, EAX ) ;
    }
    else
    {
        Compile_Pop_To_EAX ( DSP ) ;
        //Compile_IMUL ( cell mod, cell reg, cell rm, sib, disp, imm, size )
        _Compile_IMUL ( MEM, EAX, DSP, 0, 0 ) ;
        //zero->StackPushRegisterCode = Here ;
        Compile_Move_EAX_To_TOS ( DSP ) ;
    }
}

#define DIVIDE 1
#define MOD 2
// ( a b -- a / b ) dividend in edx:eax, quotient in eax, remainder in edx ; immediate divisor in ecx

void
_Compile_Divide ( Compiler * compiler, int32 type )
{
    // dividend in edx:eax, quotient/divisor in eax, remainder in edx
    int optFlag = CheckOptimizeOperands ( compiler, 5 ) ;
    if ( optFlag == OPTIMIZE_DONE ) return ;
    else if ( optFlag )
    {
        _Compile_MoveImm ( REG, EDX, 0, 0, 0, CELL ) ;
        if ( compiler->Optimizer->OptimizeFlag & OPTIMIZE_IMM )
        {
            // for idiv the dividend must be eax:edx, divisor can be reg or rm ; here we use ECX
            // idiv eax by reg or mem
            // Compile_IDIV ( mod, rm, sib, disp, imm, size )
            Compile_IDIV ( REG, ECX, 0, 0, 0, 0 ) ; // ECX is set by optimize.c
        }
            // ?? non immediate optimization code here ??
        else
        {
            // for idiv the dividend must be eax:edx, divisor can be reg or rm ; here we use ECX
            // idiv eax by reg or mem
            // Compile_IDIV ( mod, rm, sib, disp, imm, size )
            Compile_IDIV ( compiler->Optimizer->Optimize_Mod, compiler->Optimizer->Optimize_Rm, 0,
                compiler->Optimizer->Optimize_Disp, 0, 0 ) ;
        }
    }
    else
    {
        // 64 bit dividend EDX:EAX / srcReg
        // EDX holds high order bits
        _Compile_Move_StackN_To_Reg ( EAX, DSP, - 1 ) ;
        _Compile_MoveImm ( REG, EDX, 0, 0, 0, CELL ) ;
        Compile_IDIV ( MEM, DSP, 0, 0, 0, 0 ) ;
        _Compile_Stack_DropN ( DSP, 1 ) ;
        Compile_Move_EAX_To_TOS ( DSP ) ;
        return ;
    }
    if ( GetState ( _Context_, C_SYNTAX ) ) _Stack_DropN ( _Context_->Compiler0->WordStack, 2 ) ;
    Word * zero = Compiler_WordStack ( compiler, 0 ) ;
    zero->StackPushRegisterCode = Here ;
    if ( type == MOD ) _Compile_Move_Reg_To_Reg ( EAX, EDX ) ; // for consistency finally use EAX so optimizer can always count on eax as the pushed reg
    _Compile_Stack_PushReg ( DSP, EAX ) ;
}

void
Compile_Divide ( Compiler * compiler )
{
    _Compile_Divide ( compiler, DIVIDE ) ;
}

// ( a b -- a / b ) quotient in eax, divisor and remainder in edx

void
Compile_Mod ( Compiler * compiler )
{
    _Compile_Divide ( compiler, MOD ) ;
}

// ( x n -- )
// C : "x += n" :: cfrTil : "x n +="

// X variable op compile for group 1 opCodes - ia32 

void
Compile_Group1_X_OpEqual ( Compiler * compiler, int32 op ) // += , operationCode
{
    if ( CheckOptimizeOperands ( compiler, 5 ) )
    {
        if ( compiler->Optimizer->OptimizeFlag & OPTIMIZE_IMM )
        {
            //_Compile_Group1_Immediate ( ADD, mod, operandReg, offset, immediateData, size ) ;
            _Compile_Group1_Immediate ( op, compiler->Optimizer->Optimize_Mod,
                compiler->Optimizer->Optimize_Reg, compiler->Optimizer->Optimize_Disp, compiler->Optimizer->Optimize_Imm, CELL ) ;
        }
        else
        {
            //_Compile_Group1 ( ADD, toRegOrMem, mod, reg, rm, sib, disp, isize )
            _Compile_Group1 ( op, compiler->Optimizer->Optimize_Dest_RegOrMem, compiler->Optimizer->Optimize_Mod,
                compiler->Optimizer->Optimize_Reg, compiler->Optimizer->Optimize_Rm, 0, compiler->Optimizer->Optimize_Disp, CELL ) ;
        }
    }
    else
    {
        // next :
        // EBX is used by compiler as register variable in some combinators
        _Compile_Move_StackN_To_Reg ( EAX, DSP, 0 ) ; // n
        _Compile_Move_StackN_To_Reg ( ECX, DSP, - 1 ) ; // x
        Compile_SUBI ( REG, ESI, 0, 2 * CELL_SIZE, BYTE ) ;
        //Compile_ADD ( MEM, MEM, EAX, ECX, 0, 0, CELL ) ;
        //_Compile_Group1 ( ADD, toRegOrMem, mod, reg, rm, sib, disp, isize )
        _Compile_Group1 ( op, MEM, MEM, EAX, ECX, 0, 0, CELL ) ;
    }
}

void
Compile_MultiplyEqual ( Compiler * compiler )
{
    if ( CheckOptimizeOperands ( compiler, 5 ) )
    {
        // address is in EAX
        // Compile_IMUL ( mod, rm, sib, disp, imm, size )
        //_Compile_IMULI ( cell mod, cell reg, cell rm, cell sib, cell disp, cell imm, cell size )
        if ( compiler->Optimizer->OptimizeFlag & OPTIMIZE_IMM )
        {
            _Compile_IMULI ( MEM, ECX, compiler->Optimizer->Optimize_Rm, 0, compiler->Optimizer->Optimize_Disp,
                compiler->Optimizer->Optimize_Imm, CELL ) ;
            _Compile_Move_Reg_To_Rm ( EAX, 0, ECX ) ;
        }
        else
        {
            // address is in EAX
            //_Compile_IMUL_Reg ( cell mod, cell reg, cell rm, cell sib, cell disp )
            _Compile_IMUL ( MEM, ECX, compiler->Optimizer->Optimize_Rm, 0, compiler->Optimizer->Optimize_Disp ) ;
            _Compile_Move_Reg_To_Rm ( EAX, 0, ECX ) ;
        }
    }
    else
    {
        _Compile_Move_StackNRm_To_Reg ( EAX, DSP, - 1 ) ;
        _Compile_IMUL ( MEM, EAX, ESI, 0, 0 ) ;
        _Compile_Stack_Drop ( DSP ) ;
        _Compile_Move_Reg_To_StackNRm_UsingReg ( DSP, 0, EAX, EBX ) ;
    }
}

void
Compile_DivideEqual ( Compiler * compiler )
{
    // for idiv the dividend must be eax:edx, divisor can be reg or rm
    // idiv eax by reg or mem
    if ( CheckOptimizeOperands ( compiler, 5 ) )
    {
        // assumes destination address is in EAX
        _Compile_Move_Reg_To_Reg ( EBX, EAX ) ; // save the destination address 
        if ( compiler->Optimizer->OptimizeFlag & OPTIMIZE_IMM )
        {
            //_Compile_MoveImm ( cell direction, cell rm, cell disp, cell imm, cell operandSize )
            _Compile_MoveImm ( REG, ECX, 0, 0, compiler->Optimizer->Optimize_Imm, CELL ) ;
        }
        _Compile_Move_Rm_To_Reg ( EAX, EAX, 0 ) ;
        _Compile_MoveImm ( REG, EDX, 0, 0, 0, CELL ) ;
        // Compile_IDIV( mod, rm, sib, disp, imm, size )
        Compile_IDIV ( REG, ECX, 0, 0, 0, 0 ) ;
        _Compile_Move_Reg_To_Rm ( EBX, 0, EAX ) ; // move result to destination
    }
    else
    {
        _Compile_Move_StackNRm_To_Reg ( EAX, DSP, - 1 ) ; // address of dividend is second on stack
        //_Compile_Move_Reg_To_Reg ( EDX, EAX ) ;
        //Compile_SAR ( REG, EDX, 0, 0, 31 ) ;
        _Compile_MoveImm ( REG, EDX, 0, 0, 0, CELL ) ;
        Compile_IDIV ( MEM, DSP, 0, 0, 0, 0 ) ; // divisor is tos
        _Compile_Stack_Drop ( DSP ) ;
        _Compile_Move_Reg_To_StackNRm_UsingReg ( DSP, 0, EAX, EBX ) ;
    }
}

