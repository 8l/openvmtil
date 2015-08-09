
#include "../includes/cfrtil.h"

#if 1 // SL5

/* A minimal Lisp interpreter
   Copyright 2004 Andru Luvisi

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License , or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, write to the Free Software
   Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

enum
{
    INTEGER, STRING, SYM, CONS, PROC, PRIMOP
} ;
/*** List Structured Memory ***/
#define cons(X, Y)            obMake(CONS, 2, (X), (Y))
#define car(X)                ((X)->po_O_Slots[0])
#define cdr(X)                ((X)->po_O_Slots[1])
#define setcar(X,Y)           (((X)->po_O_Slots[0]) = (Y))
#define setcdr(X,Y)           (((X)->po_O_Slots[1]) = (Y))
#define mkint(X)              obMake(INTEGER, 1, (Object *)(X))
#define intval(X)             ((int)((X)->po_O_Slots[0]))
#define mksym(X)              obMake(SYM, 1, (Object *)(X))
#define symname(X)            ((char *)((X)->po_O_Slots[0]))
#define stringValue(X)        ((char *)((X)->po_O_Slots[0]))
#define mkprimop(X)           obMake(PRIMOP, 1, (Object *)(X))
#define primopval(X)          ((primop)(X)->po_O_Slots[0])
#define mkproc(X,Y,Z)         obMake(PROC, 3, (X), (Y), (Z))
#define procargs(X)           ((X)->po_O_Slots[0])
#define proccode(X)           ((X)->po_O_Slots[1])
#define procenv(X)            ((X)->po_O_Slots[2])
#define isnil(X)              ((X) == nil)
#define extend(ENV, SYM, VAL) (cons(cons((SYM), (VAL)), (ENV)))
#define print( X ) _print ( stdout, X )
Object * assoc ( Object *, Object * ) ;
Object * evlis ( Object *, Object * ) ;
Object * multiple_extend ( Object *, Object *, Object * ) ;
Object * extend_top ( Object *, Object * ) ;
Object * readlist ( ) ;

FILE *ifp ;
int _feof = 0, deleteMemFlag ;
char *token, *lastToken ;
int lastTokenValidFlag = 0 ;
Object *all_symbols, *top_env, *nil, *sl_true, *quote, *s_if, *s_lambda, *s_lambda2, *s_define, *s_setb, *s_begin ;
NamedByteArray * Pnba_SL5 = 0 ;
byte * sl_inputBuffer ;
int sl_inputBufferIndex ;

Object *
obMake ( int64 type, int count, ... )
{
    Object *ob, *arg ;
    va_list args ;
    int i ;
    va_start ( args, count ) ;
    ob = ( Object* ) _Mem_Allocate ( sizeof (Object ) + ( count - 1 ) * sizeof (Object * ), Pnba_SL5 ) ;

    ob->type = type ;
    for ( i = 0 ; i < count ; i ++ )
    {
        ob->po_O_Slots[i] = va_arg ( args, Object * ) ;
    }
    va_end ( args ) ;
    return ob ;
}

Object *
intern ( char *name )
{
    char buffer [ MAXLEN ] ;
    Object *ob = findsym ( name ) ;
    if ( ! isnil ( ob ) ) return car ( ob ) ;
    int len = strlen ( name ) ;
    if ( name [0] == '"' && name [len - 1] == '"' )
    {
        strcpy ( buffer, name ) ;
        name = buffer ;
        name [len - 1] = 0 ;
        ob = mksym ( &name[1] ) ;
    }
    else ob = mksym ( name ) ;
    all_symbols = cons ( ob, all_symbols ) ;
    token = name ;
    return ob ;
}

Object *
assoc ( Object *key, Object * alist )
{
    if ( isnil ( alist ) ) return nil ;
    if ( car ( car ( alist ) ) == key ) return car ( alist ) ;
    return assoc ( key, cdr ( alist ) ) ;
}

Object *
multiple_extend ( Object *env, Object *syms, Object * vals )
{
    return isnil ( syms ) ? env :
        multiple_extend ( extend ( env, car ( syms ), car ( vals ) ), cdr ( syms ), cdr ( vals ) ) ;
}

Object *
extend_top ( Object *sym, Object * val )
{
    setcdr ( top_env, cons ( cons ( sym, val ), cdr ( top_env ) ) ) ;
    return val ;
}

Object *
findsym ( char *name )
{
    Object *symlist ;
    for ( symlist = all_symbols ; ! isnil ( symlist ) ; symlist = cdr ( symlist ) )
        if ( String_Equal ( name, symname ( car ( symlist ) ) ) )
            return symlist ;
    return nil ;
}

Object *
eval ( Object *exp, Object * env )
{
    Object *tmp, *proc, *vals ;
eval_start:
    if ( exp == nil ) return nil ;
    switch ( ( int ) exp->type )
    {
        case INTEGER: return exp ;
        case SYM:
        {
            tmp = assoc ( exp, env ) ;
            if ( ( ! tmp ) || ( tmp == nil ) ) sl5_exception ( exp, ( byte* ) "sl5 : Unbound symbol", 0 ) ;
            return cdr ( tmp ) ;
        }
        case CONS:
        {
            if ( car ( exp ) == s_if )
            {
                if ( eval ( car ( cdr ( exp ) ), env ) != nil ) exp = car ( cdr ( cdr ( exp ) ) ) ;
                else exp = car ( cdr ( cdr ( cdr ( exp ) ) ) ) ;
                goto eval_start ;
            }
            if ( ( car ( exp ) == s_lambda ) || ( car ( exp ) == s_lambda2 ) )
                return mkproc ( car ( cdr ( exp ) ), cdr ( cdr ( exp ) ), env ) ;
            if ( car ( exp ) == quote ) return car ( cdr ( exp ) ) ;
            if ( car ( exp ) == s_define ) return extend_top ( car ( cdr ( exp ) ), eval ( car ( cdr ( cdr ( exp ) ) ), env ) ) ;
            if ( car ( exp ) == s_setb )
            {
                Object *pair = assoc ( car ( cdr ( exp ) ), env ) ;
                Object *newval = eval ( car ( cdr ( cdr ( exp ) ) ), env ) ;
                setcdr ( pair, newval ) ;
                return newval ;
            }
            if ( car ( exp ) == s_begin )
#if 0               
            {
                exp = cdr ( exp ) ;
                if ( exp == nil ) return nil ;
                while ( 1 )
                {
                    if ( cdr ( exp ) == nil )
                    {
                        exp = car ( exp ) ;
                        goto eval_start ;
                    }
                    eval ( car ( exp ), env ) ;
                    exp = cdr ( exp ) ;
                }
            }
#elif 0
                {
                    ListObject * lexp, * lnext ;
                    // 'begin' is first node ; skip it.
                    for ( lexp = List_Next ( l0 ) ; lexp && ( lnext = List_Next ( lexp ) ) ; lexp = lnext )
                    {
                        _List_Print ( _List_Eval ( lexp ) ) ;
                    }
                    return lexp ;
                }
#else            
                {
                    Object * nxt ;
                    for ( exp = cdr ( exp ) ; exp != nil ; exp = nxt )
                    {
                        if ( ( nxt = cdr ( exp ) ) != nil ) eval ( car ( exp ), env ) ;
                        else
                        {
                            exp = car ( exp ) ;
                            goto eval_start ;
                        }
                    }
                    return exp ;
                }
#endif            
            proc = eval ( car ( exp ), env ) ;
            vals = evlis ( cdr ( exp ), env ) ;
            if ( proc->type == PRIMOP ) return exp ; //(*primopval ( proc ) )( vals ) ;
            if ( proc->type == PROC )
            {
                /* For dynamic scope, use env instead of procenv(proc) */
                // env = multiple_extend ( env, procargs ( proc ), vals ) ;
                env = multiple_extend ( procenv ( proc ), procargs ( proc ), vals ) ;
                exp = cons ( s_begin, proccode ( proc ) ) ;
                goto eval_start ;
            }
            sl5_exception ( proc, ( byte* ) "sl5 : error : Bad PROC type", CONTINUE ) ;
            return exp ; // allow to print the list
        }
        case PRIMOP: return exp ;
        case PROC: return exp ;
    }
    /* Not reached */
    return exp ;
}

Object *
evlis ( Object *exps, Object * env )
{
    if ( exps == nil ) return nil ;
    return cons ( eval ( car ( exps ), env ), evlis ( cdr ( exps ), env ) ) ;
}

void
sl5_ReadEvalPrint ( )
{
    print ( eval ( sl_read ( ), top_env ) ) ;
}

/*** Main Driver ***/
// repl

void
sl5 ( void )
{
    Printf ( ( byte* ) "sl5\n" ) ;
    _List_Repl ( sl5_ReadEvalPrint ) ;
}

#if 0
// interpret a string from stack and return a string on the stack

void
_sl5 ( void )
{
    ReadLiner *rl = _Context_->ReadLiner0 ;
    byte * b = Buffer * buffer = Buffer_New ( BUFFER_SIZE ) ;  ;
    init_sl5 ( ) ;
    strcpy ( ( char* ) b, ( char* ) &rl->InputLine [rl->ReadIndex] ) ; //save spot after '_sl5' token which brought us here
    strcpy ( ( char* ) rl->InputLine, ( char* ) _DataStack_Pop ( ) ) ;
    strcat ( ( char* ) rl->InputLine, ( char* ) "\n" ) ;
    rl->ReadIndex = 0 ;

    _try {
        print ( eval ( sl_read ( ), top_env ) ) ;
    }
    _catchAll ;
    _DataStack_Push ( ( int32 ) SessionString_New ( ( byte* ) "" ) ) ;
    strcpy ( ( char* ) rl->InputLine, ( char* ) b ) ; // setup to continue where we left off
    rl->ReadIndex = 0 ;
    PrintStateInfo_SetState ( _Q_->PrintState, PSI_NEWLINE, false ) ;
    Buffers_SetAllUnused ;
}

// reads a list and evaluates it

void
isl5 ( void )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;
    Lexer * lexer = _Context_->Lexer0 ;
    int si = rl->ReadIndex = lexer->TokenEnd_ReadLineIndex - strlen ( lexer->OriginalToken ) - 1 ;

    init_sl5 ( ) ;
    _try {
        print ( eval ( sl_read ( ), top_env ) ) ;
    }
    _catchAll ;
    //String_InsertStringIntoSlot ( ( char* ) rl->InputLine, si, rl->ReadIndex, "" ) ; //String_RemoveFinalNewline ( sl_outputBuffer ) ) ;
    PrintStateInfo_SetState ( _Q_->PrintState, PSI_NEWLINE, false ) ;
    Buffers_SetAllUnused ;
}
#endif

void
putback_token ( char *token )
{
    lastToken = token ;
    lastTokenValidFlag = 1 ;
}

char *
gettoken ( )
{
    //return (char*) Lexer_NextToken ( _Context_->Lexer0 ) ;
    int ch ;

    sl_inputBufferIndex = 0 ;
    if ( lastTokenValidFlag )
    {
        lastTokenValidFlag = 0 ;
        return lastToken ;
    }
    do
    {
        if ( ( ch = Getc ( ifp ) ) == EOF )
        {
            if ( ifp != stdin ) _feof = 1 ;
            else sl5_exception ( 0, ( byte* ) "sl5: gettoken EOF input error", 0 ) ;
        }
    }
    while ( isspace ( ch ) ) ;
    add_to_buf ( ch ) ;
    if ( strchr ( "()\'", ch ) ) return buf2str ( ) ;
    while ( 1 )
    {
        if ( ( ch = Getc ( ifp ) ) == EOF )
        {
            if ( ifp != stdin ) _feof = 1 ;
            else sl5_exception ( 0, ( byte* ) "sl5: gettoken EOF input error", 0 ) ;
        }
        if ( strchr ( "()\'", ch ) || isspace ( ch ) )
        {
            UnGetc ( ch, ifp ) ;
            return buf2str ( ) ;
        }
        add_to_buf ( ch ) ;
    }
}

Object *
sl_read ( )
{
    int ch ;
start:
    token = gettoken ( ) ;
    if ( token [ 0 ] == ';' ) // == comment to eol
    {
        while ( 1 )
        {
            if ( ( ch = Getc ( ifp ) ) == EOF ) sl5_exception ( 0, ( byte* ) "sl5: lread input error", 0 ) ;
            if ( ch == '\n' ) break ;
        }
        goto start ;
    }
    if ( String_Equal ( token, "(" ) ) return readlist ( ) ;
    if ( String_Equal ( token, "\'" ) ) return cons ( quote, cons ( sl_read ( ), nil ) ) ;
    if ( token[strspn ( token, "0123456789" )] == '\0' ) return mkint ( atoi ( token ) ) ;
    return intern ( token ) ;
}

Object *
readlist ( )
{
    char *token = gettoken ( ) ;
    Object *tmp ;
    if ( String_Equal ( token, ")" ) ) return nil ;
    if ( String_Equal ( token, "." ) )
    {
        tmp = sl_read ( ) ;
        if ( strcmp ( gettoken ( ), ")" ) ) sl5_exception ( 0, ( byte* ) "sl5: readlist input error", 0 ) ;
        return tmp ;
    }
    putback_token ( token ) ;
    tmp = sl_read ( ) ; /* Must force evaluation order */
    return cons ( tmp, readlist ( ) ) ;
}

void //byte *
_print ( FILE *ofp, Object * op )
{
    if ( op )
    {
        switch ( ( int ) op->type )
        {
            case INTEGER: _vprintf ( ofp, "%d", intval ( op ) ) ;
                break ;
            case CONS:
                _vprintf ( ofp, "(" ) ;
                while ( 1 )
                {
                    _print ( ofp, car ( op ) ) ;
                    if ( isnil ( cdr ( op ) ) )
                    {
                        _vprintf ( ofp, ")" ) ;
                        break ;
                    }
                    op = cdr ( op ) ;
                    if ( op->type != CONS )
                    {
                        _vprintf ( ofp, " . " ) ;
                        _print ( ofp, op ) ;
                        _vprintf ( ofp, ")" ) ;
                        break ;
                    }
                    _vprintf ( ofp, " " ) ;
                }
                break ;
            case SYM:
                if ( isnil ( op ) ) _vprintf ( ofp, "()" ) ;
                else _vprintf ( ofp, "%s", symname ( op ) ) ;
                break ;
            case PRIMOP: _vprintf ( ofp, "#<PRIMOP>" ) ;
                break ;
            case PROC: _vprintf ( ofp, "#<PROC>" ) ;
                break ;
            default: sl5_exception ( 0, ( byte* ) "sl5: writeobj error", 0 ) ;
        }
    }
}

/*** Primitives ***/
Object *
prim_sum ( Object * args )
{
    int sum ;
    for ( sum = 0 ; ! isnil ( args ) ; sum += intval ( car ( args ) ), args = cdr ( args ) ) ;
    return mkint ( sum ) ;
}

Object *
prim_sub ( Object * args )
{
    int sum ;
    for ( sum = intval ( car ( args ) ), args = cdr ( args ) ;
        ! isnil ( args ) ;
        sum -= intval ( car ( args ) ), args = cdr ( args ) ) ;
    return mkint ( sum ) ;
}

Object *
prim_prod ( Object * args )
{
    int prod = 1 ;
    for ( prod = 1 ; ! isnil ( args ) ; prod *= intval ( car ( args ) ), args = cdr ( args ) ) ;
    return mkint ( prod ) ;
}

Object *
prim_div ( Object * args )
{
    int prod = 1 ;
    for ( prod = 1 ; ! isnil ( args ) ; prod /= intval ( car ( args ) ), args = cdr ( args ) ) ;
    return mkint ( prod ) ;
}

Object *
prim_numeq ( Object * args )
{
    return intval ( car ( args ) ) == intval ( car ( cdr ( args ) ) ) ? sl_true : nil ;
}

Object *
prim_cons ( Object * args )
{
    return cons ( car ( args ), car ( cdr ( args ) ) ) ;
}

Object *
prim_car ( Object * args )
{
    return car ( car ( args ) ) ;
}

Object *
prim_cdr ( Object * args )
{
    return cdr ( car ( args ) ) ;
}

#if 1 //daj

Object *
prim_load ( Object * args )
{
    Object * v ;
    for ( ; ! isnil ( args ) ; args = cdr ( args ) )
    {
        Object * arg = car ( args ) ;
        char * name = symname ( arg ) ;
        FILE *f = fopen ( name, "r" ) ;
        if ( f == NULL )
        {
            sl5_exception ( arg, ( byte* ) "sl5 : error : file not found", 0 ) ;
        }
        _try {
            setinput ( f ) ;
            for ( _feof = 0 ; ! _feof ; v = eval ( sl_read ( ), top_env ) ) ;
        }
        _catchAll ;
        fclose ( f ) ;
    }
    setinput ( stdin ) ;
    return v ;
}
#endif

byte *
_vprintf ( FILE * f, char *format, ... )
{
    va_list args ;
    va_start ( args, ( char* ) format ) ;
    __Printf ( format, args ) ;
}

unsigned int
Getc ( FILE * f )
{
    if ( f == stdin )
        return ( int ) ReadLine_NextChar ( _Context_->ReadLiner0 ) ;
    else return fgetc ( f ) ;
}

void
UnGetc ( int c, FILE * f )
{
    if ( f == stdin )
        ReadLine_UnGetChar ( _Context_->ReadLiner0 ) ;
    else ungetc ( c, f ) ;
}

void
sl5_exception ( Object * exp, byte * msg, int restart )
{
    Warning2 ( ( byte* ) msg, exp ? ( byte* ) exp->po_O_Slots[0] : 0 ) ;
    if ( restart != CONTINUE ) _Throw ( restart ) ;
}

void
setinput ( FILE * fp )
{
    ifp = fp ;
}

void
add_to_buf ( char ch )
{
    if ( sl_inputBufferIndex < MAXLEN - 1 ) sl_inputBuffer[sl_inputBufferIndex ++] = ch ;
}

char *
buf2str ( )
{
    sl_inputBuffer [ sl_inputBufferIndex ++ ] = '\0' ;
    return strdup ( ( char* ) sl_inputBuffer ) ;
}

/*** Initialization ***/

void
init_sl5 ( )
{
    if ( ! Pnba_SL5 )
    {
        Buffer * slBuffer ;
        Pnba_SL5 = MemorySpace_NBA_New ( _Q_->MemorySpace0, ( byte* ) "SL5", 2 * MB, SL5_MEM ) ;
        sl_inputBuffer = Buffer_NewPermanent ( &slBuffer, MAXLEN ) ;
        setinput ( stdin ) ;
        nil = mksym ( "nil" ) ;
        all_symbols = cons ( nil, nil ) ;
        top_env = cons ( cons ( nil, nil ), nil ) ;
        sl_true = intern ( "t" ) ;
        extend_top ( sl_true, sl_true ) ;
        quote = intern ( "quote" ) ;
        s_if = intern ( "if" ) ;
        s_lambda = intern ( "lambda" ) ;
        s_lambda2 = intern ( "\\" ) ;
        s_define = intern ( "define" ) ;
        s_setb = intern ( "set!" ) ;
        s_begin = intern ( "begin" ) ;
        extend_top ( intern ( "+" ), mkprimop ( prim_sum ) ) ;
        extend_top ( intern ( "-" ), mkprimop ( prim_sub ) ) ;
        extend_top ( intern ( "*" ), mkprimop ( prim_prod ) ) ;
        extend_top ( intern ( "/" ), mkprimop ( prim_div ) ) ;
        extend_top ( intern ( "=" ), mkprimop ( prim_numeq ) ) ;
        extend_top ( intern ( "cons" ), mkprimop ( prim_cons ) ) ;
        extend_top ( intern ( "car" ), mkprimop ( prim_car ) ) ;
        extend_top ( intern ( "cdr" ), mkprimop ( prim_cdr ) ) ;
        extend_top ( intern ( "load" ), mkprimop ( prim_load ) ) ;
    }
}

#if 0

int
main ( )
{
    init_sl5 ( ) ;
    setinput ( stdin ) ;
    while ( 1 )
    {
        _print ( stdout, eval ( sl_read ( ), top_env ) ) ;
        printf ( "\n" ) ;
    }
    return 0 ;
}
#endif

#endif // SL5
