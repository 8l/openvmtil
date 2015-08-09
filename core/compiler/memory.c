#include "../../includes/cfrtil.h"

void
Compile_Peek ( Compiler * compiler, int32 stackReg ) // @
{
    int optFlag = CheckOptimizeOperands ( compiler, 2 ) ;
    if ( optFlag == OPTIMIZE_DONE ) return ;
    else // assume arg is tos
    {
        _Compile_Move_Rm_To_Reg ( EAX, stackReg, 0 ) ;
        _Compile_Move_Rm_To_Reg ( EAX, EAX, 0 ) ;
        _Compile_Move_Reg_To_Rm ( stackReg, 0, EAX ) ;
    }
}

// '!' ( value address  -- ) // store value at address - dpans94 - address is on top - value was pushed first, leftmost in rpn, then address

void
Compile_Store ( Compiler * compiler, int32 stackReg ) // !
{
    int optFlag = CheckOptimizeOperands ( compiler, 4 ) ;
    if ( optFlag == OPTIMIZE_DONE ) return ;
    else if ( optFlag )
    {
        // _Compile_MoveImm ( cell direction, cell rm, cell disp, cell imm, cell operandSize )
        if ( compiler->Optimizer->OptimizeFlag & OPTIMIZE_IMM ) _Compile_MoveImm ( compiler->Optimizer->Optimize_Dest_RegOrMem,
                compiler->Optimizer->Optimize_Rm, 0, compiler->Optimizer->Optimize_Disp, compiler->Optimizer->Optimize_Imm, CELL ) ;
        else if ( compiler->Optimizer->OptimizeFlag & OPTIMIZE_REGISTER ) 
        {
            // allow for one of these to be EAX which is 0
            if ( compiler->Optimizer->Optimize_SrcReg || compiler->Optimizer->Optimize_DstReg ) _Compile_Move_Reg_To_Reg ( compiler->Optimizer->Optimize_DstReg, compiler->Optimizer->Optimize_SrcReg ) ;
            //_Compile_Move ( int32 direction, int32 reg, int32 rm, int32 sib, int32 disp )
            else _Compile_Move ( compiler->Optimizer->Optimize_Dest_RegOrMem, compiler->Optimizer->Optimize_Reg, compiler->Optimizer->Optimize_Rm, 0, 0 ) ;
        }
        else _Compile_Move_Reg_To_Rm ( compiler->Optimizer->Optimize_Rm, compiler->Optimizer->Optimize_Disp, compiler->Optimizer->Optimize_Reg ) ;
    }
    else
    {
        _Compile_Move_Rm_To_Reg ( EAX, stackReg, 0 ) ;
        _Compile_Move_Rm_To_Reg ( ECX, stackReg, - CELL_SIZE ) ;
        _Compile_Move_Reg_To_Rm ( EAX, 0, ECX ) ;
        Compile_SUBI ( REG, stackReg, 0, 8, BYTE ) ;
    }
}

// ( address value -- ) store value at address - reverse order of parameters from '!'

void
Compile_Poke ( Compiler * compiler, int32 stackReg ) // =
{
    //if ( CheckOptimizeOperands ( compiler, 3 ) )
    int optFlag = CheckOptimizeOperands ( compiler, 4 ) ;
    if ( optFlag == OPTIMIZE_DONE ) return ;
    else if ( optFlag )
    {
        // _Compile_MoveImm ( cell direction, cell rm, cell disp, cell imm, cell operandSize )
        if ( compiler->Optimizer->OptimizeFlag & OPTIMIZE_IMM ) _Compile_MoveImm ( compiler->Optimizer->Optimize_Dest_RegOrMem,
                compiler->Optimizer->Optimize_Rm, 0, compiler->Optimizer->Optimize_Disp, compiler->Optimizer->Optimize_Imm, CELL ) ;
        else if ( compiler->Optimizer->OptimizeFlag & OPTIMIZE_REGISTER ) 
        {
            // allow for one of these to be EAX which is 0
            if ( compiler->Optimizer->Optimize_SrcReg ||  compiler->Optimizer->Optimize_DstReg ) _Compile_Move_Reg_To_Reg ( compiler->Optimizer->Optimize_DstReg, compiler->Optimizer->Optimize_SrcReg ) ;
            //_Compile_Move ( int32 direction, int32 reg, int32 rm, int32 sib, int32 disp )
            else _Compile_Move ( compiler->Optimizer->Optimize_Dest_RegOrMem, compiler->Optimizer->Optimize_Reg, compiler->Optimizer->Optimize_Rm, 0, 0 ) ;
        }
        else _Compile_Move_Reg_To_Rm ( compiler->Optimizer->Optimize_Rm, compiler->Optimizer->Optimize_Disp, compiler->Optimizer->Optimize_Reg ) ;
    }
    else
    {
        _Compile_Move_Rm_To_Reg ( ECX, stackReg, 0 ) ;
        _Compile_Move_Rm_To_Reg ( EAX, stackReg, - CELL_SIZE ) ;
        _Compile_Move_Reg_To_Rm ( EAX, 0, ECX ) ;
        Compile_SUBI ( REG, stackReg, 0, 8, BYTE ) ;
    }
}

// n = *m
// ( n m -- )
// 
void
Compile_AtEqual ( int32 stackReg ) // !
{
    _Compile_Move_Rm_To_Reg ( EAX, stackReg, 0 ) ;
    _Compile_Move_Rm_To_Reg ( EAX, EAX, 0 ) ;
    _Compile_Move_Rm_To_Reg ( ECX, stackReg, - CELL_SIZE ) ;
    _Compile_Move_Reg_To_Rm ( ECX, 0, EAX ) ;
    Compile_SUBI ( REG, stackReg, 0, CELL_SIZE * 2, BYTE ) ;
}

