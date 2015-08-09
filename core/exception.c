
#include "../includes/cfrtil.h"

// ?? this file could be worked on ??

void
_OpenVmTil_ShowExceptionInfo ( )
{
    AlertColors ;
    if ( _Q_->Verbosity )
    {
        if ( _Q_->SignalExceptionsHandled < 2 )
        {
            if ( _CfrTil_ && _CfrTil_->Debugger0 )
            {
                Debugger_ShowInfo ( _CfrTil_->Debugger0, _Q_->ExceptionMessage, _Q_->Signal ) ;
                Word * word = _CfrTil_->Debugger0->w_Word ;
                if ( ! word ) word = _CfrTil_->CurrentRunWord ;
                if ( word )
                {
                    _CfrTil_Source ( word, 0 ) ;
                    if ( ! CompileMode && ( ! ( word->CType & CPRIMITIVE ) ) ) _CfrTil_Word_Disassemble ( word ) ;
                }
                //_Printf ( ( byte* ) "\n" ) ;
            }
            _DisplaySignal ( _Q_->Signal ) ;
        }
    }
    //if ( _Q_->SignalExceptionsHandled < 2 )
    _Q_->Signal = 0 ;
    OpenVmTil_Pause ( ) ;
    DefaultColors ;
    SetBuffersUnused ;
}

void
_OpenVmTil_Pause ( byte * prompt )
{
    int key ;
start:
    DebugColors ;
    //ConserveNewlines ;
    _Printf ( "%s", prompt ) ;
    key = Key ( ) ;
    _ReadLine_PrintfClearTerminalLine ( ) ;
    if ( key == 'd' ) CfrTil_Debug ( ) ; //_Debugger_Interpret ( _CfrTil_->Debugger0, 1 ) ;
    else if ( key == '\\' )
    {
        SetState ( _CfrTil_, DEBUG_MODE, false ) ;
        SetState ( _CfrTil_->Debugger0, DBG_COMMAND_LINE, true ) ;
        Debugger_InterpretLine ( ) ;
        goto start ;
    }
    DefaultColors ;
}

void
OpenVmTil_Pause ( )
{
    _OpenVmTil_Pause ( ( byte* ) "\nPause : Any <key> to continue... :: 'd' for debugger, '\\' for a command prompt and turn off this debug mode ..." ) ;
}

void
_OpenVmTil_Throw ( jmp_buf * sjb, byte * excptMessage, int32 restartCondition ) 
{
    _Q_->ExceptionMessage = excptMessage ;
    _Q_->RestartCondition = restartCondition ;
    _Q_->Thrown = restartCondition ;
    _OpenVmTil_ShowExceptionInfo ( ) ;
    longjmp ( *sjb, - 1 ) ;
}

void
OpenVmTil_Throw ( byte * excptMessage, int32 restartCondition ) 
{
    _OpenVmTil_Throw ( &_CfrTil_->JmpBuf0, excptMessage, restartCondition ) ;
}

void
_OpenVmTil_SigLongJmp_WithMsg ( int32 restartCondition, byte * msg )
{
    OpenVmTil_Throw ( msg, restartCondition ) ;
}

void
OpenVmTil_SignalAction ( int signal, siginfo_t * si, void * uc )
{
    _Q_->SigAddress = 0 ;
    if ( signal != 17 ) // 17 : "CHILD TERMINATED" : ignore; its just back from a shell fork
    {
        _Q_->Signal = signal ;
        _Q_->SigAddress = si->si_addr ;
        _Q_->SigLocation = ( byte* ) c_dd ( Context_Location ( ) ) ;
        OpenVmTil_Throw ( 0, 0 ) ;
    }
}

#if 0
void
_OVT_ClearExceptionStack ( )
{
    Stack_Clear ( _Q_->ExceptionStack ) ;
}

void
_OVT_PopExceptionStack ( )
{
    _Stack_Pop ( _Q_->ExceptionStack ) ;
}
#endif

void
CfrTil_Exception ( int32 signal, int32 restartCondition )
{
    //Buffer * buffer = Buffer_New ( BUFFER_SIZE ) ;
    //byte * b = Buffer_Data ( buffer ) ;
    char * b = ( char* ) Buffer_Data ( _CfrTil_->HistoryExceptionB ) ;
    AlertColors ;
    switch ( signal )
    {
        case CASE_NOT_LITERAL_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Syntax Error : \"case\" only takes a literal/constant as its parameter after the block", restartCondition ) ;
            break ;
        }
        case DEBUG_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Debug Error : User is not in debug mode", restartCondition ) ;
            break ;
        }
        case OBJECT_REFERENCE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Object Reference Error", restartCondition ) ;
            break ;
        }
        case OBJECT_SIZE_ERROR:
        {
            sprintf ( ( char* ) b, "Warning : Class object size is 0. Did you declare 'size' for %s? ",
                _Context_->Interpreter0->w_Word->ContainingNamespace->Name ) ;
            OpenVmTil_Throw ( b, restartCondition ) ;
            break ;
        }
        case STACK_OVERFLOW:
        {
            OpenVmTil_Throw ( ( byte* ) "Stack Overflow", restartCondition ) ;
            break ;
        }
        case STACK_UNDERFLOW:
        {
            OpenVmTil_Throw ( ( byte* ) "Stack Underflow", restartCondition ) ;
            break ;
        }
        case STACK_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Stack Error", restartCondition ) ;
            break ;
        }
        case SEALED_NAMESPACE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "New words can not be added to sealed namespaces", restartCondition ) ;
            break ;
        }
        case NAMESPACE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Namespace (Not Found?) Error", restartCondition ) ;
            break ;
        }
        case SYNTAX_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Syntax Error", restartCondition ) ;
            break ;
        }
        case NESTED_COMPILE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Nested Compile Error", restartCondition ) ;
            break ;
        }
        case COMPILE_TIME_ONLY:
        {
            OpenVmTil_Throw ( ( byte* ) "Compile Time Use Only", restartCondition ) ;
            break ;
        }
        case BUFFER_OVERFLOW:
        {
            OpenVmTil_Throw ( ( byte* ) "Buffer Overflow", restartCondition ) ;
            break ;
        }
        case MEMORY_ALLOCATION_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Memory Allocation Error", restartCondition ) ;
            break ;
        }
        case NOT_A_KNOWN_OBJECT:
        case LABEL_NOT_FOUND_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Word not found. Misssing namespace qualifier? ", QUIT ) ;
            break ;
        }
        case ARRAY_DIMENSION_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Array has no dimensions!? ", QUIT ) ;
            break ;
        }
        case INLINE_MULTIPLE_RETURN_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Multiple return points in a inlined function", restartCondition ) ;
            break ;
        }
        case MACHINE_CODE_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Error in machine coding", restartCondition ) ;
            break ;
        }
        case VARIABLE_NOT_FOUND_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Variable not found error", restartCondition ) ;
            break ;
        }
        case FIX_ME_ERROR:
        {
            OpenVmTil_Throw ( ( byte* ) "Fix Me", restartCondition ) ;
            break ;
        }
        default:
        {
            OpenVmTil_Throw ( 0, restartCondition ) ;
            break ;
        }
    }
    //Buffer_SetAsUnused ( buffer ) ;
    //OpenVmTil_Throw ( 0, restartCondition ) ;
    return ;
}

// ?? the logic of exceptions could be reworked ??

void
Error3 ( byte * format, byte * one, byte * two, int three )
{
    char buffer [ 128 ] ;
    sprintf ( ( char* ) buffer, ( char* ) format, one, two ) ;
    Error ( ( byte* ) buffer, three ) ;
}

void
Error2 ( byte * format, byte * one, int two )
{
    char buffer [ 128 ] ;
    sprintf ( ( char* ) buffer, ( char* ) format, one ) ;
    Error ( ( byte* ) buffer, two ) ;
}

