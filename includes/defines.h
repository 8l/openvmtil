
#define LINUX 1
#define DEBUG 0
#define NEW 0
#if NEW
#define OLD 0
#else 
#define OLD 1
#endif
#define CLANG 0
#define SIGJMP_BUF 0
#if CLANG
#define NO_GLOBAL_REGISTERS 1 
#else
#define NO_GLOBAL_REGISTERS 0
#endif

#if DEBUG 
#define D( x ) x
#define D1( x ) x
#define D0( x ) 
#define d1( x ) x
#define d0( x ) 
#else 
#define D( x ) 
#define D1( x ) 
#define D0( x ) 
#define d1( x ) x
#define d0( x ) 
#endif
// these did work ok
#define SL5 0
#define RETRO 0
#define MARU 0
#define MARU_2_4 0
#define MARU_NILE 0

#if SL5 || MARU || MARU_2_4 || MARU_NILE
#define LISP_IO 1
#else
#define LISP_IO 0
#endif

// these may need some work again to work with the newer IO - LISP_IO
#define JOY 0
#define TINY 0 // femtolisp tiny
#define CHIBI_SCHEME 0
#define PICOLISP 0
#define RACKET 0
#define BOOTSTRAP_SCHEME 0 // needs to add bcfrtil-gdb to Makefile default target
#define NEWLISP_LIB 0

#define CONSTANT_FOLDING 1
// under construction -- not implemented (yet?)
#define BOOTFORTH 0
#define RETURN_STACK 0 // forth style rstack

#define eof (byte) EOF
#define ABI 32
// #define ABI 64
#if ABI == 64
#define INT_FRMT "%ld"
#define INT_FRMT_02 "%02ld"
#define INT_FRMT_9 "%9ld"
#define INT_FRMT_3 "%3ld"
#define UINT_FRMT "%lx" 
#define UINT_FRMT_0x "0x%lx"
#define UINT_FRMT_09 "%09lx"
#define UINT_FRMT_0x09 "0x%09lx"
#define UINT_FRMT_0x020 "0x%020lx"
#else
#define INT_FRMT "%d"
#define INT_FRMT_02 "%02d" 
#define INT_FRMT_9 "%9d"
#define INT_FRMT_9_LEFT "%-9d"
#define INT_FRMT_3 "%3d"
#define UINT_FRMT "0x%x"
#define HEX_UINT_FRMT "0x%x"
#define LISP_HEX_FRMT "#x%x"
#define LISP_DECIMAL_FRMT "#d%d"
#define HEX_INT_FRMT "%x"
#define UINT_FRMT_0x "0x%x"
#define UINT_FRMT_09 "%09x"
#define UINT_FRMT_08 "%08x"
#define UINT_FRMT_0x08 "0x%08x"
#define UINT_FRMT_0x09 "0x%09x"
#define UINT_FRMT_0x020 "0x%020x"
#endif
#define BYTE_FRMT_0x "%x"

#define CELL_SIZE ( sizeof ( int32 ) )
#define SLOT_SIZE CELL_SIZE
#define INT_SIZE ( sizeof (int))
#define CELL CELL_SIZE

#define true 1
#define false 0
#define DONE true
#define NOT_DONE false
#define ESC 27
#define inline
//#define _GCBDW_ 0
#define READLINE 1
#define STACK_CHECK_ERROR 1
#define STACK_EMPTY_POPS_ZERO 1

#define USE_AL_TO_PASS_LOGIC 0 // al x86 register    
#define USE_ZF_TO_PASS_LOGIC 0 // zero flag ; not there yet
#define USE_TOS_TO_PASS_BIGNUM_LOGIC 1 // but is default for interpret mode

#define INIT_FILE_NAME ("./init.f")
#define LABEL_DELIMITER_TOKEN (";")

#define STACK_ERROR ( 1 << 0 )
#define STACK_UNDERFLOW ( 1 << 1 )
#define STACK_OVERFLOW ( 1 << 2 )
#define MEMORY_ALLOCATION_ERROR ( 1 << 3 )
//#define LABEL_NOT_FOUND_ERROR ( 1 << 4 )
#define COMPILE_TIME_ONLY ( 1 << 5 )
#define SYNTAX_ERROR ( 1 << 6 )
#define LOCALS_UNAVAILABLE ( 1 << 7 )
#define NAMESPACE_ERROR ( 1 << 8 )
#define NESTED_COMPILE_ERROR ( 1 << 9 )
#define SEALED_NAMESPACE_ERROR ( 1 << 10 )
#define COLON_QUOTATION_MIX_WARNING ( 1 << 11 )
#define MACHINE_CODE_ERROR ( 1 << 12 )
#define PARSE_ERROR ( 1 << 13 )
#define DEBUG_ERROR ( 1 << 14 )
#define OBJECT_SIZE_ERROR ( 1 << 15 )
#define VARIABLE_NOT_FOUND_ERROR ( 1 << 16 )
#define BUFFER_OVERFLOW ( 1 << 17 )
#define INLINE_MULTIPLE_RETURN_ERROR ( 1 << 18 )
#define OBJECT_REFERENCE_ERROR ( 1 << 19 )
#define CASE_NOT_LITERAL_ERROR ( 1 << 20 )
#define LABEL_NOT_FOUND_ERROR ( 1 << 21 )
#define ARRAY_DIMENSION_ERROR ( 1 << 22 )
#define FIX_ME_ERROR ( 1 << 23 )

#define KB 1024
#define MB ( KB * KB )
#define INIT_SIZE 1024
#define _STACK_SIZE 16
#define K KB
#define M MB

#define BUFFER_SIZE (1024)
#define DICTIONARY_SIZE ( 10 * MB )
#define HISTORY_SIZE ( 1 * MB )
#define OBJECTS_SIZE ( 20 * MB )
#define CONTEXT_SIZE ( 10 * MB )
#define TEMP_OBJECTS_SIZE ( 10 * MB )
#define COMPILER_TEMP_OBJECTS_SIZE ( 2 * MB )
#define CODE_SIZE ( 10 * MB )
#define SESSION_OBJECTS_SIZE ( 10 * MB )
#define LISP_TEMP_SIZE ( 10 * MB )
#define BUFFER_SPACE_SIZE ( 1 * MB )
//#define QUOTATIONS_SIZE (BUFFER_SIZE * 256)

//#define CODE_BUFFER_SIZE (BUFFER_SIZE * 2 )
#define HISTORY_LIST_SIZE ( 1 * MB )
#define STACK_SIZE ( 1 * MB ) // * 4 bytes per slot

// #define MINOR_STACK_SIZE 64
//#define LOCALS_STACK_SIZE 256
//#define NUMBER_OF_LOCALS 64

// Flags
#define FALSE 0
#define TRUE 1 
#define RVALUE 1
#define LVALUE 2

// PrintStateInfo defines
#define PSI_NEWLINE ( 1 << 15 )
#define PSI_PROMPT  ( 1 << 16 )

// CType - LType -shared in common 
#define CPRIMITIVE ( (uint64) 1 << 0 )
#define CFRTIL_WORD ( (uint64) 1 << 1 )
#define T_RAW_STRING ( (uint64) 1 << 2 )
#define T_STRING ( (uint64) 1 << 3 )
#define T_INT ( (uint64) 1 << 4 )
#define T_FLOAT ( (uint64) 1 << 5 )
#define T_BIG_INT ( (uint64) 1 << 6 )
#define T_BIG_FLOAT ( (uint64) 1 << 7 )
#define T_HEAD ( (uint64) 1 << 8 )
#define T_TAIL ( (uint64) 1 << 9 )
#define LITERAL ( (uint64) 1 << 10 )

// CType - C forth types
#define COMBINATOR ( (uint64) 1 << 11 )
#define IMMEDIATE ( (uint64) 1 << 12 )
#define NAMESPACE ( (uint64) 1 << 13 )
#define BLOCK ( (uint64) 1 << 14 )
#define INLINE ( (uint64) 1 << 15 )
#define CLASS ( (uint64) 1 << 16 )
#define VARIABLE ( (uint64) 1 << 17 )
#define STACK_VARIABLE ( (uint64) 1 << 18 )
#define LOCAL_VARIABLE ( (uint64) 1 << 19 )
#define PREFIX ( (uint64) 1 << 20 )
#define INFIXABLE ( (uint64) 1 << 21 )
#define OBJECT ( (uint64) 1 << 22 )
#define CONSTANT ( (uint64) 1 << 23 )
#define CATEGORY_STACK ( (uint64) 1 << 24 )
#define REGISTER_VARIABLE ( (uint64) 1 << 25 )
#define THIS ( (uint64) 1 << 26 )
#define CLASS_MEMBER_ACCESS ( (uint64) 1 << 27 )
#define LIST_FUNCTION ( (uint64) 1 << 28 )
#define LISP_WORD LIST_FUNCTION 
#define CATEGORY_OP_STORE ( (uint64) 1 << 29 )
#define CATEGORY_DUP ( (uint64) 1 << 30 )
#define CATEGORY_OP_ORDERED ( (uint64) 1 << 31 )
#define CATEGORY_OP_LOAD ( (uint64) 1 << 32 )
#define CATEGORY_OP_EQUAL ( (uint64) 1 << 33 )
#define CATEGORY_LOGIC ( (uint64) 1 << 34 )
#define CATEGORY_OP_1_ARG ( (uint64) 1 << 35 )
#define CATEGORY_RECURSIVE ( (uint64) 1 << 36 )
#define CATEGORY_OP_UNORDERED ( (uint64) 1 << 37 )
#define CATEGORY_OP_STACK ( (uint64) 1 << 38 )
#define CATEGORY_OP_DIVIDE ( (uint64) 1 << 39 )
#define C_PREFIX ( (uint64) 1 << 40 )
#define KEYWORD ( (uint64) 1 << 41 )
#define DYNAMIC_OBJECT ( (uint64) 1 << 42 )
#define DOBJECT DYNAMIC_OBJECT
#define C_PREFIX_RTL_ARGS ( (uint64) 1 << 43 )
#define ALIAS ( (uint64) 1 << 44 )
#define TEXT_MACRO ( (uint64) 1 << 45 )
#define STRING_MACRO ( (uint64) 1 << 46 )
#define HISTORY_NODE ( (uint64) 1 << 47 )
#define DEBUG_WORD ( (uint64) 1 << 48 )
#define RT_STACK_OP ( (uint64) 1 << 49 ) // Run Time stack operation
#define WORD_CREATE ( (uint64) 1 << 50 ) // Run Time stack operation
#define INTERPRET_DBG ( (uint64) 1 << 51 ) 
#define INFIX_WORD ( (uint64) 1 << 52 ) 
#define C_RETURN        ( (uint64) 1 << 53 ) 
#define DLSYM_WORD      ( (uint64) 1 << 54 ) 
#define C_TYPE          ( (uint64) 1 << 55 ) 
#define CLASS_CLONE     ( (uint64) 1 << 56 ) 
#define C_CLASS         ( (uint64) 1 << 57 ) 
#define C_TYPEDEF       ( (uint64) 1 << 58 ) 

// _CType for interpreter word types - 4 bits/ 16 possibilities : N_WordType bitfield
#define _PREFIX                 1
#define _INFIXABLE              2
#define _C_PREFIX_RTL_ARGS      3
#define _POSTFIX                4

// LType - lisp types
#define T_LAMBDA ( (uint64) 1 << 11 )
#define T_CONS ( (uint64) 1 << 12 )
#define LIST ( (uint64) 1 << 13 )
#define LISP_C_RTL_ARGS ( (uint64) 1 << 14 )
#define LIST_OBJECT ( (uint64) 1 << 15 )
#define LIST_QUOTATION ( (uint64) 1 << 16 )
#define T_LISP_IF ( (uint64) 1 << 17 )
#define T_LISP_SET ( (uint64) 1 << 18 )
#define LIST_NODE ( (uint64) 1 << 19 )
#define T_LISP_SPECIAL ( (uint64) 1 << 20 )
//#define T_LISP_SYMBOL ( (uint64) 1 << 20 )
#define T_LISP_BEGIN ( (uint64) 1 << 21 )
#define T_LISP_TOKEN ( (uint64) 1 << 22 )
#define T_LISP_DEFINE ( (uint64) 1 << 23 )
#define T_LISP_MACRO ( (uint64) 1 << 24 )
#define LISP_VOID_RETURN ( (uint64) 1 << 26 )
#define T_NIL ( (uint64) 1 << 27 )
#define LIST_FUNCTION ( (uint64) 1 << 28 )
#define LISP_WORD LIST_FUNCTION 
#define T_LISP_COMPILED_WORD ( (uint64) 1 << 29 )
#define T_LISP_COMPILE ( (uint64) 1 << 30 )
#define T_LISP_LET ( (uint64) 1 << 31 )
#define T_LISP_TERMINATING_MACRO ( (uint64) 1 << 32 )
#define T_LISP_READ_MACRO ( (uint64) 1 << 33 )
#define T_LISP_LOCALS_ARG ( (uint64) 1 << 35 )
//#define T_LISP_QUASIQUOTE ( (uint64) 1 << 34 )
//#define T_LISP_UNQUOTE ( (uint64) 1 << 35 )
#define T_LISP_UNQUOTE_SPLICING ( (uint64) 1 << 36 )
#define T_LISP__DEFINE ( (uint64) 1 << 37 )
#define T_LISP_VALUE ( (uint64) 1 << 38 )
#define T_LISP_WORD ( (uint64) 1 << 39 )
//#define T_LISP_DEFMACRO ( (uint64) 1 << 37 )
#define T_LISP_SYMBOL ( (uint64) 1 << 44 )

//#define   ( (uint64) 1 <<  )
#define NEW_RUN_COMPOSITE 0
#define EXIT ( 1 << 30 )
#define PAUSE ( 1 << 11)
#define INITIAL_START ( 1 << 10)
#define FULL_RESTART ( 1 << 9 )
#define RESTART ( 1 << 8 )
#define RESET_ALL ( 1 << 7 )
#define ABORT ( 1 << 6 )
#define QUIT ( 1 << 5 )
// don't use 4 because it is also SIGILL ??
#define STOP ( 1 << 3 )
#define BREAK ( 1 << 2 )
#define CONTINUE ( 1 << 0 )
#define ON ( 1 )
#define OFF ( 0 )
#define PROMPT "-: "
#define STARTUP_TIME_IT 8 // random number not 0 or 1

// GotoInfo Types
#define GI_BREAK ( 1 << 0 )
#define GI_RETURN ( 1 << 1 )
#define GI_CONTINUE ( 1 << 2 )
#define GI_GOTO ( 1 << 3 )
#define GI_RECURSE ( 1 << 4 )
#define GI_TAIL_CALL ( 1 << 5 )
#define GI_CALL_LABEL ( 1 << 6 )

// MemChunk / Memory Types
#define OPENVMTIL ( 1 << 0 )
#define CFRTIL ( 1 << 1 )
#define PERMANENT ( 1 << 2 )
#define TEMPORARY ( 1 << 3 )
#define TEMP_OBJECT_MEMORY TEMPORARY
#define DATA_STACK ( 1 << 4 )
#define SESSION ( 1 << 5 )
#define DICTIONARY ( 1 << 6 )
#define EXISTING ( 1 << 7 )
// can't use these for chunk types
//#define T_HEAD ( (uint64) 1 << 8 )
//#define T_TAIL ( (uint64) 1 << 9 )
#define CONTEXT ( 1 << 10 )
#define BUFFER ( 1 << 12 )
#define LISP_TEMP ( 1 << 13 )
#define OBJECT_MEMORY ( 1 << 14 )
#define SL5_MEM ( 1 << 14 )
#define LISP ( 1 << 15 )
#define HISTORY ( 1 << 16 )
#define CODE ( 1 << 17 )
#define COMPILER_TEMP_OBJECT_MEMORY ( 1 << 18 )
#define COMPILER_TEMP COMPILER_TEMP_OBJECT_MEMORY 
#define LARGEST_MEMORY_TYPE COMPILER_TEMP_OBJECT_MEMORY
//#define TINY ( 1 << 14 )
//#define DEBUG ( 1 << 12 )
//#define ALL      ( INTERNAL | EXTERNAL | CFRTIL | TEMPORARY | DATA_STACK | SESSION )

#define OPTIMIZE_RM ( 1 << 0 )
#define OPTIMIZE_IMM ( 1 << 1 )
#define OPTIMIZE_REG_REG ( 1 << 2 )
#define OPTIMIZE_REG_MEM ( 1 << 3 )
#define OPTIMIZE_MEM_REG ( 1 << 4 )
#define OPTIMIZE_EAX_ECX ( 1 << 5 )
#define OPTIMIZE_ECX_EAX ( 1 << 6 )
#define OPTIMIZE_EAX_MEM ( 1 << 7 )
#define OPTIMIZE_MEM_EAX ( 1 << 8 )
#define OPTIMIZE_MEM_ECX ( 1 << 9 )
#define OPTIMIZE_ECX_MEM ( 1 << 10 )
#define OPTIMIZE_TO_MEM  ( 1 << 11 )
#define OPTIMIZE_TO_REG  ( 1 << 12 )
#define OPTIMIZE_DONE  ( 1 << 13 )
#define OPTIMIZE_REGISTER  ( 1 << 14 )
//#define OPTIMIZE_TOS ( 1 << 11 )
//#define OPTIMIZE_NOS ( 1 << 12 ) // Next On Stack = Dsp [ -1 ]

#define OP_LC 1 // literal/constant
#define OP_VAR 2 
#define OP_FETCH 3 
#define OP_STORE 4 
#define OP_CPRIMITIVE 4  
#define OP_ORDERED 5 
#define OP_UNORDERED 6 
#define OP_LOGIC 7 
#define OP_1_ARG 8 
#define OP_RECURSE 9 
#define OP_EQUAL 10 
#define OP_DUP 11  // stack or local var
#define OP_DIVIDE 12
#define OP_CLASS_ACCESS_WORD 13  
#define OP_STACK 14  
#define OP_OBJECT 15
#define OP_C_RETURN 16
// #define O_BITS 4 // bits needed to encode category codes
#define O_BITS 5 // experiment with 64 bit encoding 

#define DONT_PUSH_EAX (( byte* ) 1 )
#define PUSH_EAX (( byte* ) 2 )

// Debugger Flags
#define DBG_DONE ( 1 << 0 ) 
#define DBG_MENU ( 1 << 1 ) 
#define DBG_STEPPING ( 1 << 2 ) 
#define DGB_SAVE_DSP ( 1 << 3 ) 
#define DBG_RESTORE_REGS ( 1 << 4 ) 
#define DBG_CAN_STEP ( 1 << 5 ) 
#define DBG_NEWLINE ( 1 << 6 )
#define DBG_PROMPT ( 1 << 7 )
#define DBG_INFO ( 1 << 8 )
#define DBG_ACTIVE ( 1 << 9 )
#define DBG_AUTO_MODE ( 1 << 10 )
#define DBG_INTERNAL ( 1 << 11 ) 
#define DBG_STEPPING_THRU_C_IMMEDIATE ( 1 << 12 ) 
#define DBG_ESCAPED ( 1 << 13 )
#define DBG_COMMAND_LINE ( 1 << 14 )
#define DBG_CONTINUE ( 1 << 15 )
#define DBG_RETURN ( 1 << 16 )
#define DBG_STEPPED ( 1 << 17 )
#define DBG_PRE_DONE ( 1 << 18 )
#define DBG_STACK_CHANGE ( 1 << 19 )
#define DBG_BRK_INIT ( 1 << 20 )
#define DBG_RUNTIME ( 1 << 21 )

// TODO : nb. flags need to be edited !!!!! for right category, overlap, use/non-use, etc.
// CfrTil state flags added to System flags
#define CFRTIL_RUN ( 1 << 0 )
#define SOURCE_CODE_INITIALIZED  ( 1 << 1 )
#define STRING_MACROS_ON ( 1 << 9 )
#define DEBUG_MODE ( 1 << 7 )

// State Flags --
// ? sytem flags ?? -- not all -- todo here
#define ADD_READLINE_TO_HISTORY ( 1 << 8 )
//#define SYSTEM_CALLS ( 1 << 9 )
#define VERBOSE ( 1 << 10 )
#define END_OF_FILE ( 1 << 12 )
#define GET_NEWLINE ( 1 << 13 )
#define PRINTED ( 1 << 14 )
#define DO_PROMPT ( 1 << 18 )
#define AUTO_VAR ( 1 << 19 )
#define DO_OK ( 1 << 20 )
#define ALWAYS_ABORT_ON_EXCEPION ( 1 << 21 )  // for debugging
#define DOT_WORD_COMPLETION ( 1 << 22 )
#define DEBUG_ON ( 1 << 23 )
#define REGISTERS_MODE_ON ( 1 << 24 )
#define OPTIMIZE_ON ( 1 << 25 )
#define INLINE_ON ( 1 << 26 )
#define READLINE_ECHO_ON ( 1 << 27 )

// interpreter flags
#define INTERPRETER_DONE ( 1 << 28 )
#define INTERPRETER_DBG_ACTIVE ( 1 << 29 )

// lexer flags
#define LEXER_DONE   ( 1 << 0)
#define SINGLE_ESCAPE ( 1 << 1 )
#define MULTIPLE_ESCAPE ( 1 << 2 )
#define ADD_TOKEN_TO_SOURCE ( 1 << 3 ) 
#define NOT_A_KNOWN_OBJECT ( 1 << 4 )  
#define KNOWN_OBJECT ( 1 << 5 )  
#define APPEND_NEWLINE ( 1 << 6 ) 
#define PARSING_STRING ( 1 << 7 ) 
#define LEXER_END_OF_STRING ( 1 << 8 )
#define LEXER_LISP_MODE ( 1 << 9 )
#define ADD_CHAR_TO_SOURCE ( 1 << 10 ) 
#define LEXER_RETURN_NULL_TOKEN ( 1 << 11 ) 

// readline flags and defines
#define PROMPT_LENGTH ( strlen ( (char*) rl->Prompt ) ) 
#define END_OF_LINE ( 1 << 3 )
#define END_OF_INPUT ( 1 << 4 )
#define TAB_WORD_COMPLETION ( 1 << 5 )
#define CHAR_ECHO ( 1 << 6 )
#define ANSI_ESCAPE ( 1 << 7 )
#define READLINER_DONE   ( 1 << 8)
#define END_OF_STRING ( 1 << 9 )
#define STRING_MODE ( 1 << 10 )
#define ADD_TO_HISTORY ( 1 << 11 )

// compiler flags
#define COMPILE_MODE ( 1 << 0 )
#define BLOCK_MODE COMPILE_MODE
#define ADD_FRAME ( 1 << 1 )
#define LEFT_BRACKET ( 1 << 2 )
#define SAVE_ESP ( 1 << 3 ) 
#define LISP_COMPILE_MODE ( 1 << 4 )
#define RETURN_NONE ( 1 << 5 ) 
#define RETURN_TOS ( 1 << 6 )
#define RETURN_EAX ( 1 << 7 ) 
#define LISP_COMBINATOR_MODE ( 1 << 8 )
#define LISP_MODE ( 1 << 9 )
#define INFIX_MODE ( 1 << 10 )
#define RETURN_VAR ( 1 << 11 )
#define VARIABLE_FRAME ( 1 << 12 )
#define DONT_REMOVE_STACK_VARIABLES ( 1 << 13 )
#define C_COMBINATOR_LPAREN ( 1 << 14 )
#define LISP_LAMBDA_MODE ( 1 << 15 )
#define LC_ARG_PARSING ( 1 << 16 )
#define INTERPRET_NBLOCKS ( 1 << 17 )
#define LC_PREFIX_ARG_PARSING ( 1 << 18 )
#define LC_C_RTL_ARG_PARSING ( 1 << 19 )
#define PARSING_QUALIFIED_ID ( 1 << 20 ) 
//#define INFIX_MODE_ONE_OFF ( 1 << 23 )

// Context flags
#define C_SYNTAX ( 1 << 20 )
#define PREFIX_MODE ( 1 << 22 )
#define C_LHS ( 1 << 23 )
#define C_RHS ( 1 << 24 )
#define CONTEXT_PARSING_QUALIFIED_ID ( 1 << 25 ) 
#define ADDRESS_OF_MODE ( 1 << 27 ) 

#define NON_INLINABLE ( 1 << 0 )
#define DONE true

// block flag values
#define FORCE_COMPILE 1
#define FORCE_RUN 2

// Word - Namespace State flags
#define ANY (NOT_USING|USING)
#define NOT_USING ( 1 << 0 )
#define USING ( 1 << 1 )
#define TREED ( 1 << 2 )
#define NOT_COMPILED ( 1 << 3 )
#define COMPILED_INLINE ( (uint64) 1 << 4 )
#define COMPILED_OPTIMIZED ( (uint64) 1 << 5 )
#define UNQUOTED ( 1 << 6 )
#define LISP_EVALLIST_ARG ( 1 << 7 )
#define UNQUOTE_SPLICE ( 1 << 8 )
#define QUOTED ( 1 << 9 )
#define QUASIQUOTED ( 1 << 10 )
#define QUALIFIED_ID ( 1 << 11 )
//#define TC_START ( 1 << 12 )
//#define TC_VISITED ( 1 << 11 )

// Lisp State flags 
#define PRINT_VALUE ( 1 << 0 )
#define LC_DEFINE_MODE ( 1 << 1 )
// LambdaBody states
#define USED ( 1<< 0 )
// ListObject states
#define NEW_COPY ( 1 << 6 )
#define MULTI_USE ( 1 << 7 )

// CharSet types
#define CHAR_DELIMITER  ( 1 << 0 )
#define CHAR_BLANK      ( 1 << 1 )
#define CHAR_ALPHA      ( 1 << 2 )
#define CHAR_DIGIT10	( 1 << 3 )
#define CHAR_DIGIT16	( 1 << 4 )
#define CHAR_LETTER     ( 1 << 5 )
#define CHAR_PRINT      ( 1 << 6 )

#define _CharSet_IsDelimiter( cset, ch ) cset [ ch ]

// OpenVmTil State defines
#define OVT_IN_USEFUL_DIRECTORY ( 1 << 0 )

// tree node states
// #define GONE_DEPTH 
#define TREE_VISITED ( 1 << 0 )
