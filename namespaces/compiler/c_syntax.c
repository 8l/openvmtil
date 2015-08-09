5 
// maybe we're shooting for a "smart syntactic language" that will parse "poetic" syntactic arrangements
System Interpreter
// : #! ' // xt@ compile ; // for script files with first line as "#! cfrtil -s"
// : token { nextChar ' ' !=  } {  } 
#if 1
C_Syntax 
"typedef" ' type alias
C_Syntax 
type int ;
C_Syntax 
type void ;
C_Syntax 
type char ;

C_Combinators Infix C
infixOn prefixOn

void "printf" "/lib32/libc.so.6" dlsymWord c_prefix
int "strcmp" "/lib32/libc.so.6" dlsymWord c_return c_prefix
' strcmp wdiss

void
chi ( | a b c )
{
    c = 1 + 2 ;
    b = c + 2 ;
    a = b + c ;
    p ( c ) ;
    p ( b ) ;
    p ( a ) ;
}
' chi wdiss
chi

void
chey ( | a )
{
    a = 1 + 2 ;
    p ( a ) ;
}
' chey wdiss
chey 

void
interpret2 ( | atoken aword ) 
{
    while ( 1 )
    {   
        nl () ; tab () ; ps ( "interpret2 :> " );
        atoken = token () ;
        d: if ( ! ( strcmp ( atoken, ";;" ) ) ) 
        { 
            return ; 
        }
        else
        {
            if ( atoken @ != 0 )
            {       
                printf ( "\ttoken = %s", atoken ) ; 
                aword = find ( atoken ) ;
                if ( aword @ != 0 )
                { 
                    setupWordEval ( aword ) ;
                    if ( ( ?compileMode () not ) || ( ?immediate (aword ) ) )
                    { 
                        // printf ( "\ntoken = %s\n", atoken ) ; 
                        wordEval ( aword ) ; 
                    } 
                    else 
                    { 
                        _compileWord ( aword ) ;
                    }
                } 
                else 
                { 
                    literalInterpret (atoken ) ;
                } 
            }       
        }
    }
}
' interpret2 wdiss
"C_Syntax" notUsing
"C_Combinators" notUsing 
"C" notUsing 
infixOff d: using
void interpret2 words using : hi 1 2 + nl p ; : hey hi hi ; using hey 
#endif
2 + 7 assertStkChk // stack checking

