#include "../includes/cfrtil.h"

#if 0
// example from : http://www.kernel.org/doc/man-pages/online/pages/man3/dlsym.3.html

Load the math library, and _print the cosine of 2.0 :

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int
main ( int argc, char **argv )
{
    void *handle ;
    double (*cosine )( double ) ;
    char *error ;

    handle = dlopen ( "libm.so", RTLD_LAZY ) ;
    if ( ! handle )
    {
        fprintf ( stderr, "%s\n", dlerror ( ) ) ;
        exit ( EXIT_FAILURE ) ;
    }

    dlerror ( ) ; /* Clear any existing error */

    /* Writing: cosine = (double (*)(double)) dlsym(handle, "cos");
       would seem more natural, but the C99 standard leaves
       casting from "void *" to a function pointer undefined.
       The assignment used below is the POSIX.1-2003 (Technical
       Corrigendum 1) workaround; see the Rationale for the
       POSIX specification of dlsym(). */

    *( void ** ) ( &cosine ) = dlsym ( handle, "cos" ) ;

    if ( ( error = dlerror ( ) ) != NULL )
    {
        fprintf ( stderr, "%s\n", error ) ;
        exit ( EXIT_FAILURE ) ;
    }

    printf ( "%f\n", ( *cosine )( 2.0 ) ) ;
    dlclose ( handle ) ;
    exit ( EXIT_SUCCESS ) ;
}
/*
       If this program were in a file named "foo.c", you would build the program with
       the following command:

           gcc -rdynamic -o foo foo.c -ldl

       Libraries exporting _init() and _fini() will want to be compiled as follows,
       using bar.c as the example name:

           gcc -shared -nostartfiles -o bar bar.c
 */

#endif

// lib : full library path

#define RTLD_DEFAULT	((void *) 0)

void *
_dlsym ( char * sym, char * lib )
{
    void *fp, * hLibrary = dlopen ( lib, RTLD_GLOBAL | RTLD_LAZY ) ;
    if ( hLibrary )
    {
        //fp = ( void* ) dlsym ( hLibrary, ( char* ) sym ) ;
        fp = ( void* ) dlsym ( RTLD_DEFAULT, ( char* ) sym ) ; // either work after dlopen
    }
    if ( ( ! hLibrary ) || ( ! fp ) )
    {
        Printf ( ( byte* ) c_ad ( "\ndlsym : dlerror = %s\n" ), dlerror () ) ;
        return 0 ;
    }
    return fp ;
}

// lib sym | addr

void
CfrTil_dlsym ( )
{
    char * sym = ( char* ) _DataStack_Pop ( ) ;
    char * lib = ( char* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( ( int ) _dlsym ( lib, sym ) ) ;
}

// callNumber | errno

void
CfrTil_system0 ( )
{
    _Compile_Stack_PopToReg ( DSP, EAX ) ;
    _Compile_INT80 ( ) ;
    _Compile_Stack_PushReg ( DSP, EAX ) ;
}

void
CfrTil_system1 ( )
{
    _Compile_Stack_PopToReg ( DSP, EAX ) ;
    _Compile_Stack_PopToReg ( DSP, EBX ) ;
    _Compile_INT80 ( ) ;
    _Compile_Stack_PushReg ( DSP, EAX ) ;
}

void
CfrTil_system2 ( )
{
    _Compile_Stack_PopToReg ( DSP, EAX ) ;
    _Compile_Stack_PopToReg ( DSP, EBX ) ;
    _Compile_Stack_PopToReg ( DSP, ECX ) ;
    _Compile_INT80 ( ) ;
    _Compile_Stack_PushReg ( DSP, EAX ) ;
}

void
CfrTil_system3 ( )
{
    _Compile_Stack_PopToReg ( DSP, EAX ) ;
    _Compile_Stack_PopToReg ( DSP, EBX ) ;
    _Compile_Stack_PopToReg ( DSP, ECX ) ;
    _Compile_Stack_PopToReg ( DSP, EDX ) ;
    _Compile_INT80 ( ) ;
    _Compile_Stack_PushReg ( DSP, EAX ) ;
}

#if 0

void *
dlOpen_dlSym ( char * lib, char * sym )
{
    void * hLibrary, *fp ;
    char * error, buffer [1024] ;

    sprintf ( buffer, "./%s.so", lib ) ;
    hLibrary = dlopen ( buffer, RTLD_GLOBAL | RTLD_LAZY ) ;
    if ( ! hLibrary )
    {
        sprintf ( buffer, "/usr/lib32/%s.so", lib ) ;
        hLibrary = dlopen ( buffer, RTLD_GLOBAL | RTLD_LAZY ) ;
    }
    if ( ! hLibrary )
    {
        sprintf ( buffer, "/usr/local/lib/%s.so", lib ) ;
        hLibrary = dlopen ( buffer, RTLD_GLOBAL | RTLD_LAZY ) ;
    }
    if ( ! hLibrary )
    {
        sprintf ( buffer, "/usr/lib/%s.so", lib ) ;
        hLibrary = dlopen ( buffer, RTLD_GLOBAL | RTLD_LAZY ) ;
    }
    if ( ! hLibrary )
    {
        Printf ( ( byte* ) "\nCannot open %s - cannot import library\n", buffer ) ;
        return 0 ;
    }
    fp = ( void* ) dlsym ( RTLD_DEFAULT /*hLibrary*/, ( char* ) sym ) ;
    //if ( ( error = dlerror ( ) ) != NULL )
    if ( ( ! fp ) || ( ( error = dlerror ( ) ) != NULL ) )
    {
        Printf ( ( byte* ) "dlOpen_dlSym : dlerror: %s\n", error ) ;
        return 0 ;
    }

    //void * hLibrary = dlopen ( lib, RTLD_DEFAULT |RTLD_GLOBAL | RTLD_LAZY ) ;
    void * fp = _dlsym ( lib, sym ) ;

    return fp ;
}
#endif

char *
_CfrTil_GetSystemState_String0 ( byte * buf )
{
    strcpy ( buf, "\noptimization is " ) ;
    if ( GetState ( _CfrTil_, OPTIMIZE_ON ) ) strcat ( buf, "on, " ) ;
    else strcat ( buf, "off, " ) ;
    strcat ( buf, "inlining is " ) ;
    if ( CfrTil_GetState ( _CfrTil_, INLINE_ON ) ) strcat ( buf, "on, " ) ;
    else strcat ( buf, "off, " ) ;
    strcat ( buf, "infixMode is " ) ;
    if ( GetState ( _Context_, INFIX_MODE ) ) strcat ( buf, "on, " ) ;
    else strcat ( buf, "off, " ) ;
    strcat ( buf, "prefixMode is " ) ;
    if ( GetState ( _Context_, PREFIX_MODE ) ) strcat ( buf, "on, " ) ;
    else strcat ( buf, "off, " ) ;
    strcat ( buf, "c_syntax is " ) ;
    if ( GetState ( _Context_, C_SYNTAX ) ) strcat ( buf, "on, " ) ;
    else strcat ( buf, "off, " ) ;
    strcat ( buf, "LHS is " ) ;
    if ( GetState ( _Context_, C_LHS ) ) strcat ( buf, "on, " ) ;
    else strcat ( buf, "off, " ) ;
    strcat ( buf, "RHS is " ) ;
    if ( GetState ( _Context_, C_RHS ) ) strcat ( buf, "on. " ) ;
    else strcat ( buf, "off. " ) ;
    return buf ;
}
char *
_CfrTil_GetSystemState_String1 ( char *buf )
{
    strcat ( buf, "\nReadLine echo is " ) ;
    if ( CfrTil_GetState ( _CfrTil_, READLINE_ECHO_ON ) ) strcat ( buf, "on. " ) ;
    else strcat ( buf, "off. " ) ;
    strcpy ( buf, "\nDebug is " ) ;
    if ( GetState ( _Q_, DEBUG_ON ) ) strcat ( buf, "on. " ) ;
    else strcat ( buf, "off. " ) ;
    sprintf ( &buf[strlen ( buf )], "Verbosity = %d. ", _Q_->Verbosity ) ;
    sprintf ( &buf[strlen ( buf )], "Console = %d.\n", _Q_->Console ) ;
    return buf ;
}

void
_CfrTil_SystemState_Print ( int32 pflag )
{
    Buffer * buffer = Buffer_New ( BUFFER_SIZE ) ;
    char * buf = ( char* ) Buffer_Data ( buffer ) ;
    buf = _CfrTil_GetSystemState_String0 ( buf ) ;
    Printf ( ( byte* ) buf ) ;
    buf = _CfrTil_GetSystemState_String1 ( buf ) ;
    Printf ( ( byte* ) buf ) ;
    if ( pflag ) OpenVmTil_Print_DataSizeofInfo ( pflag ) ;
    Buffer_SetAsUnused ( buffer ) ;
}

void
__CfrTil_Dump ( int32 address, int32 number, int32 dumpMod )
{
    byte * nformat ;
    int32 i, n ;
    if ( _Context_->System0->NumberBase == 16 ) nformat = ( byte* ) "\nDump : Address = " UINT_FRMT_0x08 " : Number = " UINT_FRMT " - (little endian)" ;
    else nformat = ( byte* ) "\nDump : Address = " UINT_FRMT_0x08 " : Number = " INT_FRMT " - (little endian)" ;
    Printf ( nformat, ( int32 ) address, number ) ;
    for ( i = 0 ; i < number ; )
    {
        Printf ( ( byte* ) "\n" UINT_FRMT_0x08 " : ", address + i ) ;
        if ( ! ( i % dumpMod ) )
        {
            for ( n = 0 ; n < dumpMod ; n += CELL_SIZE )
            {
                Printf ( ( byte* ) UINT_FRMT_08 " ", *( int32* ) ( address + i + n ) ) ;
            }
            Printf ( ( byte* ) " " ) ;
            for ( n = 0 ; n < dumpMod ; n += CELL_SIZE )
            {
                CfrTil_NByteDump ( ( byte* ) ( address + i + n ), CELL_SIZE ) ;
            }
            for ( n = 0 ; n < dumpMod ; n += CELL_SIZE )
            {
                CfrTil_CharacterDump ( ( byte* ) ( address + i + n ), CELL_SIZE ) ;
            }
            i += dumpMod ;
        }

        else i ++ ;
    }
}

void
_CfrTil_Source ( Word *word, int32 addToHistoryFlag )
{
    if ( word )
    {
        char * name = c_dd ( word->Name ) ;
        uint64 category = word->CType ;
        if ( word->ContainingNamespace ) Printf ( ( byte* ) "\n%s.", word->ContainingNamespace->Name ) ;
        if ( category & OBJECT )
        {
            Printf ( ( byte* ) "%s <:> %s", name, "object" ) ;
        }
        else if ( category & NAMESPACE )
        {
            Printf ( ( byte* ) "%s <:> %s", name, "namespace" ) ;
        }
        else if ( category & TEXT_MACRO )
        {
            Printf ( ( byte* ) "%s <:> %s", name, "macro" ) ;
        }
        else if ( category & LOCAL_VARIABLE )
        {
            Printf ( ( byte* ) "%s <:> %s", name, "local variable" ) ;
        }
        else if ( category & STACK_VARIABLE )
        {
            Printf ( ( byte* ) "%s <:> %s", name, "stack variable" ) ;
        }
        else if ( category & VARIABLE )
        {
            Printf ( ( byte* ) "%s <:> %s", name, "variable" ) ;
        }
        else if ( category & CONSTANT )
        {
            Printf ( ( byte* ) "%s <:> %s", name, "constant" ) ;
        }
        else if ( category & ALIAS )
        {
            Word * aword = Word_GetFromCodeAddress_NoAlias ( ( byte* ) ( block ) word->Definition ) ;
            if ( aword )
            {
                Printf ( ( byte* ) "%s alias for %s", name, ( char* ) c_dd ( aword->Name ) ) ;
                word = aword ;
            }
        }
        else if ( category & CPRIMITIVE )
        {
            Printf ( ( byte* ) "%s <:> %s", name, "primitive" ) ;
        }
        else if ( word->LType & T_LISP_COMPILED_WORD )
        {
            Printf ( ( byte* ) "%s <:> %s", name, "lambdaCalculus compiled word" ) ;
        }
        else if ( category & CFRTIL_WORD )
        {
            Printf ( ( byte* ) "%s <:> %s", name, "cfrTil compiled word" ) ;
        }
        else if ( word->LType & T_LISP_DEFINE )
        {
            Printf ( ( byte* ) "%s <:> %s", name, "lambdaCalculus defined word" ) ;
        }
        else if ( category & BLOCK )
        {
            Printf ( ( byte* ) "%s <:> %s", name, "cfrTil compiled code block" ) ;
        }
        // else CfrTil_Exception ( 0, QUIT ) ;
        if ( category & INLINE ) Printf ( ( byte* ) ", %s", "inline" ) ;
        if ( category & IMMEDIATE ) Printf ( ( byte* ) ", %s", "immediate" ) ;
        if ( category & PREFIX ) Printf ( ( byte* ) ", %s", "prefix" ) ;
        if ( category & C_PREFIX ) Printf ( ( byte* ) ", %s", "c_prefix" ) ;
        if ( category & C_RETURN ) Printf ( ( byte* ) ", %s", "c_return" ) ;
        if ( category & INFIXABLE ) Printf ( ( byte* ) ", %s", "infixable" ) ;
        if ( category & INFIXABLE ) Printf ( ( byte* ) ", %s", "infixable" ) ;
        if ( word->W_pwd_WordData )
        {
            __Word_ShowSourceCode ( word ) ; // source code has newlines for multiline history
            if ( addToHistoryFlag ) _OpenVmTil_AddStringToHistoryList ( word->SourceCode ) ;
            if ( word->Filename ) Printf ( ( byte* ) "\nSource code file location of %s : \"%s\" at %d.%d", name, word->Filename, word->LineNumber, word->CursorPosition ) ;
            if ( ! ( category & CPRIMITIVE ) ) Printf ( ( byte* ) "\nCompiled with : %s%s%s", GetState ( word, COMPILED_OPTIMIZED ) ? "optimizeOn" : "optimizeOff", GetState ( word, COMPILED_INLINE ) ? ", inlineOn" : ", inlineOff",  
                GetState (_Context_, C_SYNTAX )? ", c_syntaxOn" : "", GetState (_Context_, INFIX_MODE )? ", infixOn" : "" ) ;
            if ( word->S_CodeSize ) Printf ( ( byte* ) " -- starting at address : 0x%x -- code size = %d bytes", word->Definition, word->S_CodeSize ) ;
            else Printf ( ( byte* ) " -- starting at address : 0x%x", word->Definition ) ;
        }
        Printf ( ( byte* ) "\n" ) ;
    }
}

void
_CfrTil_Dump ( int32 dumpMod )
{
    int32 number = _DataStack_Pop ( ) ;
    int32 address = _DataStack_Pop ( ) ;
    __CfrTil_Dump ( address, number, dumpMod ) ;
}

