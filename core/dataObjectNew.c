
#include "../includes/cfrtil.h"

byte *
_CfrTil_NamelessObjectNew ( Namespace * ns, int32 size )
{
    byte * ob = _CfrTil_AllocateNew ( size, OBJECT_MEMORY ) ;
    Word * word = Word_FindInOneNamespace ( ns, ( byte* ) "this" ) ;
    if ( word )
    {
        word->bp_WD_Object = ob ;
    }
    return ob ;
}

// this function is very central and should be worked on
// ?!? i tried to bring alot of things together here but now it needs simplification at least to be understandable ?!?
// Dynamic Object New : Word = Namespace = DObject : have a s_Symbol

void
_DObject_Definition_EvalStore ( Word * word, uint32 value, uint64 ctype, uint64 funcType, byte * function, int arg )
// using a variable that is a type or a function 
{
    // remember : Word = Namespace = DObject : each have an s_Symbol member
    if ( ( funcType != 0 ) || ( function != 0 ) )
    {
        if ( funcType & BLOCK )
        {
            word->CodeStart = ( byte* ) value ;
            if ( word->CType == BLOCK ) word->Definition = ( block ) _OptimizeJumps ( ( byte* ) value ) ; // this comes to play (only(?)) with unoptimized code
            else word->Definition = ( block ) value ;
            if ( ctype & CPRIMITIVE ) word->S_CodeSize = 0 ;
            else if ( ( word->CodeStart < ( byte* ) CompilerMemByteArray->BA_Data ) || ( word->CodeStart > ( byte* ) CompilerMemByteArray->bp_Last ) ) word->S_CodeSize = 0 ; // ?!? not quite accurate
            else word->S_CodeSize = ( ( byte* ) Here ) - ( ( byte* ) word->CodeStart ) ;
            word->bp_WD_Object = 0 ;
            CfrTil_InitBlockSystem ( _Context_->Compiler0 ) ;
        }
        else
        {
            Debugger * debugger = _CfrTil_->Debugger0 ;
            int32 dm = GetState ( _CfrTil_, DEBUG_MODE ) && ( ! GetState ( debugger, DBG_STEPPING ) ) ;
            if ( dm ) _Debugger_PreSetup ( debugger, 0, word ) ;
            if ( ! ( dm && GetState ( debugger, DBG_DONE ) ) )
            {
                ByteArray * scs ;
                word->Value = value ;
                if ( funcType != LITERAL )
                {
                    scs = CompilerMemByteArray ;
                    _Compiler_SetCompilingSpace ( ( byte* ) "ObjectSpace" ) ; // DictionarySpace" ) ; // same problem as namespace ; this can be called in the middle of compiling another word 
                }
                word->Coding = Here ;
                word->CodeStart = Here ;
                word->Definition = ( block ) Here ;
                if ( funcType & ( LITERAL ) )
                {
                    if ( ! ( _LC_ && GetState ( _LC_, LISP_COMPILE_MODE ) ) )
                    {
                        //word->Lo_Value = (Object*) value ;
                        DataObject_Run ( word ) ;
                    }
                    // nb : no RET insn is or should be compiled for literals : cf. below
                }
                else if ( funcType & ( CONSTANT | VARIABLE | LOCAL_VARIABLE | NAMESPACE | CLASS | CLASS_MEMBER_ACCESS | OBJECT | DOBJECT | C_TYPE | C_CLASS | CLASS_CLONE ) )
                {
                    _Compile_CallFunctionWithArg ( ( byte* ) DataObject_Run, ( int32 ) word ) ;
                }
                else if ( arg ) // C startup compiled words
                {
                    if ( arg == - 1 )
                    {
                        ( ( void (* )( ) )( function ) ) ( ) ;
                    }
                    else
                    {
                        ( ( void (* ) ( int ) )( function ) ) ( arg ) ;
                    }
                }
                else if ( ctype & C_PREFIX_RTL_ARGS )
                {
                    _Compile_Stack_Push ( DSP, ( int32 ) word ) ;
                    _Compile_Call ( ( byte* ) function ) ;
                }
                else _Compile_Call ( function ) ; //
                if ( funcType != LITERAL )
                {
                    _Compile_Return ( ) ;
                    word->S_CodeSize = Here - word->CodeStart ; // for use by inline
                    Set_CompilerSpace ( scs ) ;
                }
                else word->S_CodeSize = Here - word->CodeStart ; // for use by inline
                word->PtrObject = & word->bp_WD_Object ;
            }
            if ( dm ) _Debugger_PostShow ( debugger, 0, _Context_->Interpreter0->w_Word ) ; // a literal could have been created and shown by _Word_Run
        }
    }
}

void
_DObject_Finish ( Word * word )
{
    uint64 ctype = word->CType ;
    ReadLiner * rl = _Context_->ReadLiner0 ;
    if ( ! ( ctype & CPRIMITIVE ) )
    {
        if ( GetState ( _CfrTil_, OPTIMIZE_ON ) ) word->State |= COMPILED_OPTIMIZED ;
        if ( CfrTil_GetState ( _CfrTil_, INLINE_ON ) ) word->State |= COMPILED_INLINE ;
    }
    if ( GetState ( _Context_, INFIX_MODE ) ) word->CType |= INFIX_WORD ;
    if ( rl->InputStringOriginal )
    {
        word->Filename = rl->bp_Filename ;
        word->LineNumber = rl->i32_LineNumber ;
        word->CursorPosition = rl->i32_CursorPosition ;
    }
    word->NumberOfArgs = _Context_->Compiler0->NumberOfStackVariables ;
    _CfrTil_->LastFinishedWord = word ;
}

Word *
_DObject_Init ( Word * word, uint32 value, uint64 ctype, uint64 ftype, byte * function, int arg, int32 addToInNs, Namespace * addToNs )
{
    // remember : Word = Namespace = DObject : each have an s_Symbol
    _DObject_Definition_EvalStore ( word, value, ctype, ftype, function, arg ) ;
    _Word_Add ( word, addToInNs, addToNs ) ;
    _DObject_Finish ( word ) ;
    return word ;
}

// DObject : dynamic object

Word *
_DObject_New ( byte * name, uint32 value, uint64 ctype, uint64 ltype, uint64 ftype, byte * function, int arg, int32 addToInNs, Namespace * addToNs, int32 allocType )
{
    // remember : Word = Namespace = DObject has a s_Symbol
    Word * word = _Word_New ( name, ctype, ltype, allocType ) ;
    word->RunType = ftype ;
    _DObject_Init ( word, value, ctype, ftype, function, arg, addToInNs, addToNs ) ;
    return word ;
}

Word *
_Class_Object_New ( byte * name, uint64 category )
{
    int32 size = 0 ;
    byte * object ;
    Namespace * ns = _CfrTil_Namespace_InNamespaceGet ( ) ;
    if ( strcmp ( "this", ( char* ) name ) != 0 ) // not with "this" : Class_New auto creates an object named "this"
    {
        // assuming we are in the correct namespace
        size = _Namespace_VariableValueGet ( ns, ( byte* ) "size" ) ;
        if ( ! size )
        {
            CfrTil_Exception ( OBJECT_SIZE_ERROR, QUIT ) ;
        }
        object = _CfrTil_NamelessObjectNew ( ns, size ) ; // size : not for "this"
    }
    else object = 0 ;

    Word * word = _DObject_New ( name, ( int32 ) object, ( OBJECT | IMMEDIATE | category ), 0, OBJECT, ( byte* ) DataObject_Run, - 1, 1, 0, DICTIONARY ) ;
    word->Size = size ;
    Class_Object_Init ( object, word, ns ) ;
    return word ;
}

Namespace *
_Class_New ( byte * name, uint64 type, int32 cloneFlag )
{
    Namespace * ns = _Namespace_Find ( name, 0, 0 ), * sns ;
    int32 size = 0 ;
    if ( ! ns )
    {
        sns = _CfrTil_Namespace_InNamespaceGet ( ) ;
        if ( cloneFlag )
        {
            size = _Namespace_VariableValueGet ( sns, ( byte* ) "size" ) ;
        }
        ns = _DObject_New ( name, 0, CPRIMITIVE | CLASS | IMMEDIATE | type, 0, type, ( byte* ) DataObject_Run, - 1, 0, sns, DICTIONARY ) ;
        _Namespace_DoNamespace ( ns ) ; // before "size", "this"
        _CfrTil_Variable ( ( byte* ) "size", size ) ; // start with size of the prototype for clone
        _Class_Object_New ( ( byte* ) "this", VARIABLE | THIS ) ;
    }
    else
    {
        Printf ( ( byte* ) "\nNamespace Error ? : class \"%s\" already exists!\n", ns->Name ) ;
        _Namespace_DoNamespace ( ns ) ;
    }
    return ns ;
}

void
_CfrTil_MachineCodePrimitive_NewAdd ( const char * name, uint64 cType, block * callHook, byte * function, int32 functionArg, const char *nameSpace, const char * superNamespace )
{
    Word * word = _DObject_New ( ( byte* ) name, 0, cType, 0, 0, function, functionArg, 0, 0, DICTIONARY ) ;
    if ( callHook ) *callHook = word->Definition ;
    _CfrTil_InitialAddWordToNamespace ( word, ( byte* ) nameSpace, ( byte* ) superNamespace ) ;
}

void
_CfrTil_ClassField_New ( byte * token, Class * aclass, int32 size, int32 offset )
{
    Word * word = _DObject_New ( token, 0, IMMEDIATE | CLASS_MEMBER_ACCESS, 0, CLASS_MEMBER_ACCESS, ( byte* ) DataObject_Run, - 1, 1, 0, DICTIONARY ) ;
    word->ClassFieldNamespace = aclass ;
    word->Size = size ;
    word->Offset = offset ;
}

void
CfrTil_Class_New ( void )
{
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    //_Class_New ( name, CLASS, 0, ( byte* ) _Namespace_DoNamespace ) ;
    _DataObject_New ( CLASS, name, 0, 0, 0, 0 ) ;
}

void
CfrTil_Class_Clone ( void )
{
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    //_Class_New ( name, CLASS_CLONE, 1, ( byte* ) _Namespace_DoNamespace ) ;
    _DataObject_New ( CLASS_CLONE, name, 0, 0, 0, 0 ) ;
}
// ( <name> value -- )

// remember : this word runs at compile/interpret time ; nothing is compiled yet

void
Class_Object_New ( byte * name )
{
    //_Class_Object_New ( name, 0 ) ;
    _DataObject_New ( OBJECT, name, 0, 0, 0, 0 ) ;
}

void
CfrTil_Class_Object_New ( )
{
    //Class_Object_New ( ( byte* ) _DataStack_Pop ( ) ) ;
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    _DataObject_New ( OBJECT, name, 0, 0, 0, 0 ) ;
}

DObject *
DObject_Sub_New ( DObject * proto, byte * name, uint64 category )
{
    DObject * dobject = _DObject_New ( name, 0, ( category | DOBJECT ), 0, DOBJECT, ( byte* ) DataObject_Run, - 1, 1, 0, DICTIONARY ) ;
    DObject_SubObjectInit ( dobject, proto ) ;
    return dobject ;
}

DObject *
_DObject_NewSlot ( DObject * proto, byte * name, int32 value )
{
    DObject * dobject = DObject_Sub_New ( proto, name, DOBJECT ) ;
    dobject->bp_WD_Object = ( byte* ) value ;
    proto->Slots ++ ;
    return dobject ;
}

void
_DObject_NewClone ( DObject * proto, byte * name )
{
    DObject_Sub_New ( proto, name, DOBJECT ) ;
}

void
DObject_NewClone ( DObject * proto )
{
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    _DObject_NewClone ( proto, name ) ;
}

void
CfrTil_DObject_Clone ( )
{
    DObject * proto = ( DObject * ) _DataStack_Pop ( ), * one = WordStack ( - 1 ) ; //remember : _CfrTil_Do_DObject pushes &dobject->bp_WD_Object
    byte * name = ( byte * ) _DataStack_Pop ( ) ;
    if ( ! ( one->CType & DOBJECT ) ) Error2 ( ( byte* ) "Cloning Error : %s is not an object.", one->Name, 1 ) ;
    _DObject_NewClone ( proto, name ) ;
}

void
DObject_New ( )
{
    DObject * proto = Namespace_Find ( ( byte* ) "DObject" ) ;
    DObject_NewClone ( proto ) ;
}

void
CfrTil_DObject_New ( )
{
    // clone DObject -- create an object with DObject as it's prototype (but not necessarily as it's namespace)
    //DObject_New ( ) ;
    _DataObject_New ( DOBJECT, 0, 0, 0, 0, 0 ) ;

}

Word *
_CfrTil_Variable ( byte * name, int32 value )
{
    Namespace * addToNamespace ;
    Word * word ;
    if ( CompileMode )
    {
        BlockInfo * bi = ( BlockInfo * ) _Stack_Top ( _Context_->Compiler0->BlockStack ) ;
        if ( bi->LocalsNamespace ) addToNamespace = bi->LocalsNamespace ;
        else
        {
            addToNamespace = Namespace_FindOrNew_Local ( ) ;
        }
        word = _DObject_New ( name, value, ( LOCAL_VARIABLE | IMMEDIATE ), 0, LOCAL_VARIABLE, ( byte* ) DataObject_Run, - 1, ( ( int32 ) addToNamespace ) ? 0 : 1, addToNamespace, SESSION ) ;
        word->Index = _Context_->Compiler0->NumberOfLocals ++ ;
    }
        //else word = _DObject_New ( name, value, VARIABLE | IMMEDIATE, 0, VARIABLE, ( byte* ) Do_VariableOrLiteral, 0, 1, 0, DICTIONARY ) ;
    else word = _DObject_New ( name, value, VARIABLE | IMMEDIATE, 0, VARIABLE, ( byte* ) DataObject_Run, 0, 1, 0, DICTIONARY ) ;
    return word ;
}

void
CfrTil_Constant ( )
{
    int32 value = _DataStack_Pop ( ) ;
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    _DObject_New ( name, value, CONSTANT | IMMEDIATE, 0, CONSTANT, ( byte* ) DataObject_Run, 0, 1, 0, DICTIONARY ) ;
}

void
_CfrTil_Label ( byte * lname )
{
    Namespace * ns = Namespace_FindOrNew_SetUsing ( ( byte* ) "__labels__", _CfrTil_->Namespaces, 1 ) ;
    _DObject_New ( lname, ( int32 ) Here, CONSTANT | IMMEDIATE, 0, CONSTANT, ( byte* ) DataObject_Run, 0, 0, ns, DICTIONARY ) ;
}

Word *
_CfrTil_LocalWord ( byte * name, int32 index, int64 ctype, uint64 ltype ) // svf : flag - whether stack variables are in the frame
{
    Word * word = _DObject_New ( name, 0, ( ctype | VARIABLE | IMMEDIATE ), ltype, LOCAL_VARIABLE, ( byte* ) DataObject_Run, - 1, 1, 0, SESSION ) ;
    word->Index = index ;
    return word ;
}

Word *
ConstantOrLiteral_New ( Interpreter * interp, uint32 uliteral )
{
    // nb.! : remember the compiler optimizes with the WordStack so everything has to be converted to a Word
    // _DObject_New : calls _Do_Literal which pushes the literal on the data stack or compiles a push ...
    Word * word ;
    byte _name [ 128 ], *name ;
    if ( ! Lexer_GetState ( interp->Lexer, KNOWN_OBJECT ) )
    {
        sprintf ( _name, "literal : %x", ( uint ) uliteral ) ;
        name = _name ;
    }
    else name = ( byte* ) interp->Lexer->OriginalToken ;
    word = _DObject_New ( name, uliteral, LITERAL | CONSTANT, 0, LITERAL, ( byte* ) DataObject_Run, 0, 0, 0, ( CompileMode ? DICTIONARY : SESSION ) ) ;
    return word ;
}

Namespace *
_Namespace_New ( byte * name, Namespace * containingNs )
{
    Namespace * ns = _DObject_New ( name, 0, ( CPRIMITIVE | NAMESPACE | IMMEDIATE ), 0, NAMESPACE, ( byte* ) DataObject_Run, - 1, 0, containingNs, DICTIONARY ) ;
    return ns ;
}

Namespace *
CfrTil_Type_New ( )
{
    CfrTil_Token ( ) ;
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    return _DataObject_New ( C_TYPE, name, 0, 0, 0, 0 ) ;
}

void
CfrTil_Typedef ( )
{
    _DataObject_New ( C_TYPEDEF, 0, 0, 0, 0, 0 ) ;
}

Word *
_DataObject_New ( uint64 type, byte * name, uint64 ctype, uint64 ltype, int32 index, int32 value )
{
    Word * word = 0 ;
    switch ( type )
    {
        case NAMESPACE:
        {
            word = _Namespace_New ( name, ( Namespace * ) value ) ; // containingNs ) ;
            break ;
        }
        case VARIABLE:
        {
            word = _CfrTil_Variable ( name, value ) ;
            break ;
        }
        case OBJECT:
        {
            word = _Class_Object_New ( name, type ) ;
            break ;
        }
        case DOBJECT:
        {
            DObject_New ( ) ;
            break ;
        }
        case CLASS:
        {
            word = _Class_New ( name, CLASS, 0 ) ;
            break ;
        }
        case CLASS_CLONE:
        {
            word = _Class_New ( name, CLASS_CLONE, 1 ) ;
            break ;
        }
        case C_CLASS:
        {
            word = _Class_New ( name, C_CLASS, 0 ) ;
            break ;
        }
        case C_TYPE:
        {
            word = _Class_New ( name, C_TYPE, 0 ) ;
            _Type_Create ( ) ;
            break ;
        }
        case C_TYPEDEF:
        {
            _CfrTil_Typedef ( ) ;
            break ;
        }
        case STACK_VARIABLE:
        case LOCAL_VARIABLE:
        case T_LISP_SYMBOL | STACK_VARIABLE:
        case T_LISP_SYMBOL | LOCAL_VARIABLE:
        default: // REGISTER_VARIABLE combinations with others in this case
        {
            word = _CfrTil_LocalWord ( name, index, ctype, ltype ) ; // svf : flag - whether stack variables are in the frame
            break ;
        }
    }
    return word ;
}

