// see readme.txt for a text description
typedef unsigned char byte;
typedef byte uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint;
typedef unsigned long uint32;
typedef long long int64;
typedef unsigned long long uint64;

typedef char * CString;
typedef byte CharSet;
typedef int32 Boolean;
typedef void (* VoidFunction) (void);
typedef void (*vFunction_1_Arg) (int32);
typedef void (*vFunction_2_Arg) (int32, int32);
typedef int32(*cFunction_0_Arg) ();
typedef int32(*cFunction_1_Arg) (int32);
typedef int32(*cFunction_2_Arg) (int32, int32);
typedef void * pointer, *ptr;
typedef VoidFunction block;
typedef byte AsciiCharSet [ 256 ];

// typed objects

// 64 bit types from here on down
// design from and also used in sl5.c

typedef struct
{

    union
    {
        int32 o_slots [0];
        int32 * pint32_slots;
    };
} object;

typedef struct ObjectHeader
{
    //int32 State;

    union
    {
        uint64 OH_CType;

        struct
        {
            uint64 _CType : 60;
            uint64 WordType : 4;
        };
    };

    union
    {
        uint64 OH_LType;
        uint64 OH_AType;
    };
    //CType * O_pT_Type ;

    union
    {
        int32 OH_NumberOfSlots;
        int32 OH_NumberOfBytes;
        int32 OH_ChunkSize; // remember MemChunk is prepended at memory allocation time
        int32 OH_Size; // byte size
    };
} ObjectHeader;

typedef struct ObjectData
{

    union
    {
        byte OD_bytes[0];
        struct Object * OD_slots[0];
        //int32 OD_slots[0];
    };
} ObjectData;

typedef struct Object
{

    union
    {
        object O_object;

        struct
        {
            ObjectHeader O_ObjectHeader;
            ObjectData O_ObjectData;
        };
    };
} Object;

typedef Object * (*primop)(Object *);

typedef struct DLNode
{

    union
    {
        ObjectHeader N_ObjectHeader;

        struct
        {
            //uint64 N_CType;

            union
            {
                uint64 N_CType;

                struct
                {
                    uint64 N__CType : 60;
                    uint64 N_WordType : 4;
                };
            };

            union
            {
                uint64 N_LType;
                uint64 N_AType;
            };
            //Type * DLN_pT_Type ;

            union
            {
                int32 N_NumberOfSlots;
                int32 N_NumberOfBytes;
                int32 N_Size;
                int32 N_ChunkSize; // remember MemChunk is prepended at memory allocation time
            };
        };
    };

    struct
    {
        struct DLNode * N_pdln_After; // slots[0]
        struct DLNode * N_pdln_Before; // slots[1]
    };
} DLNode, Node, listNode, List; // 5 total slots
#define After N_pdln_After 
#define Before N_pdln_Before 
#define N_Car After 
#define N_Cdr Before
typedef void ( *MapFunction0) (DLNode *);
typedef void ( *MapFunction1) (DLNode *, int32);
typedef void ( *MapFunction2) (DLNode *, int32, int32);
typedef void ( *MapFunction2_64) (DLNode *, uint64, int32);
typedef void ( *MapFunction3) (DLNode *, int32, int32, int32);
typedef void ( *MapFunction4) (DLNode *, int32, int32, int32, int32);
typedef void ( *MapFunction5) (DLNode *, int32, int32, int32, int32, int32);
typedef Boolean(*MapFunction_1) (DLNode *);

// node with 3 properties

typedef struct listObject
{
    DLNode S_dln_Node;
    byte * S_pb_Name;

    union
    {
        DLNode * S_pdln_Node;
        struct listObject * S_plo_listObject;
        int32 S_i32_Data;
        int32 S_i32_Value;
        uint32 S_ui32_Data;
        uint32 S_ui32_Value;
        byte * S_pb_Value;
        byte * S_pb_Data;
        Object * S_po_lo_Slots[1];
        byte * S_Chunk;
    };
} DLList, alistObject, Symbol, MemChunk, HistoryStringNode, Property; // 7 total slots
typedef int32(*cMapFunction_1) (Symbol *);
#define S_Node S_dln_Node
#define S_Car S_Node.N_Car
#define S_Cdr S_Node.N_Cdr
#define Head S_Car
#define Tail S_Cdr
#define S_CurrentNode S_pdln_Node
#define S_AType S_Node.N_AType
#define S_CType S_Node.N_CType
#define S_WType S_Node.N_WordType
#define S__CType S_Node._CType
#define S_LType S_Node.N_LType
#define S_Value S_pb_Value // p[2]
#define S_Size S_Node.N_Size
#define S_ChunkSize S_Node.N_ChunkSize
#define S_Name S_pb_Name // p[3]
#define S_NumberOfSlots S_Size
#define S_Object S_pb_Data
#define S_Pointer S_Object
#define S_String S_Object

struct NamedByteArray;

typedef struct
{
    Symbol s_Symbol;
    struct NamedByteArray * OurNBA;
    byte * StartIndex;
    byte * EndIndex;
    byte * bp_Last;
    byte * BA_Data;
} ByteArray;
#define BA_Size s_Symbol.S_Size
#define BA_CType s_Symbol.S_CType
#define BA_AType s_Symbol.S_AType

typedef struct NamedByteArray
{
    Symbol s_Symbol;
    ByteArray *ba_ByteArray;
    int32 MemInitial;
    int32 MemAllocated;
    int32 MemRemaining;
    int32 NumberOfByteArrays;
    DLList * BA_MemoryList;
} NamedByteArray, NBA;
#define NBA_AType s_Symbol.S_AType
#define NBA_Chunk_Size s_Symbol.S_ChunkSize
#define NBA_Name s_Symbol.S_Name
#define NBA_Size s_Symbol.S_Size

typedef struct
{
    Symbol s_Symbol;
    int32 InUseFlag;
} Buffer;
#define B_CType s_Symbol.S_CType
#define B_Size s_Symbol.S_Size
#define B_ChunkSize s_Symbol.S_ChunkSize
#define B_Data s_Symbol.S_pb_Data

typedef struct
{
    Symbol s_Symbol;
    block CaseBlock;
} CaseNode;
#define CN_CaseValue s_Symbol.S_pb_Data

typedef struct
{
    Symbol s_Symbol;
    byte * pb_LabelName;
    byte * pb_JmpOffsetPointer;
} GotoInfo;
#define GI_CType s_Symbol.S_CType

typedef struct _Namespace
{
    Symbol s_Symbol;
    int32 State;

    union
    {
        DLList * W_dll_SymbolList;
        Object * W_pO_Object;
        byte * W_pb_Object;
        int32 W_i32_Object;
    };

    union // leave this here so we can add a ListObject to a namespace
    {
        struct _Namespace * ContainingNamespace;
        struct _Namespace * ClassFieldNamespace;
        struct _Namespace * ContainingList;
    };
    struct _Namespace * W_pw_CfrTilWord; // doesn't seem necessary for some reason
    struct _WordData * W_pwd_WordData;
} Word, Namespace, Class, DynamicObject, DObject, ListObject;
typedef ListObject* (*ListFunction0)();
typedef ListObject* (*ListFunction)(ListObject*);
typedef ListObject * (*LispFunction2) (ListObject*, ListObject*);
typedef ListObject * (*LispFunction3) (ListObject*, ListObject*, ListObject*);
typedef int32(*MapFunction_Word_PtrInt) (ListObject *, Word *, int32 *);
#define W_WordData W_pwd_WordData

typedef int32(*MapFunction) (Symbol *);
typedef int32(*MapFunction_Cell_1) (Symbol *, int32);
typedef int32(*MapFunction_Cell_2) (Symbol *, int32, int32);
typedef void ( *MapSymbolFunction) (Symbol *);
typedef void ( *MapSymbolFunction2) (Symbol *, int32, int32);

typedef struct _WordData
{
    uint64 RunType ;
    block Definition;

    union
    {
        byte * WD_bp_Object;
        uint32 Value;
    };
    Namespace * TypeNamespace;
    Namespace ** FunctionTypesArray;
    byte * CodeStart; // set at Word allocation 
    byte * Coding; // nb : !! this field is set by the Interpreter and modified by the Compiler in some cases so we also need (!) CodeStart both are needed !!  
    byte * ObjectCode; // used by objects/class words
    byte * StackPushRegisterCode; // used by the optimizer

    union
    {
        int32 Offset; // used by ClassField
        int32 NumberOfArgs;
        int32 RegToUse; // reg code : ECX, EBX, EDX, EAX, (1, 3, 2, 0) : in this order, cf. machineCode.h
    };

    union
    {
        ListObject * LambdaBody;
        int32 AccumulatedOffset; // used by Do_Object 
    };

    union
    {
        ListObject * LambdaArgs;
        int32 Index; // used by Variable and LocalWord
    };
    byte ** PtrObject; // necessary for compiling class words and variables -- might as well be used" otherwise
    int32 NestedObjects;

    union
    {
        int32 * ArrayDimensions ;
        Word * AliasOf ;
    };

    union
    {
        ListObject * ListProc;
        ListObject * ListFirst;
        int32 Slots; // number of slots in Object
    };
    byte *SourceCode;
    byte * Filename; // ?? should be made a part of a accumulated string table ??
    int32 LineNumber;
    int32 CursorPosition;
} WordData;

#define Size s_Symbol.S_Size 
#define Name s_Symbol.S_Name
#define CType s_Symbol.S_CType
#define LType s_Symbol.S_LType
#define WType s_Symbol.S_WType
#define Data s_Symbol.S_pb_Data
#define S_CodeSize Size // used by Debugger, Finder
#define S_MacroLength Size // used by Debugger, Finder
#define Lo_CType CType
#define Lo_LType LType
#define Lo_Name Name
#define Lo_Car s_Symbol.S_Car
#define Lo_Cdr s_Symbol.S_Cdr
#define Lo_Size Size
#define Lo_Head Lo_Car
#define Lo_Tail Lo_Cdr
#define Lo_NumberOfSlots Size
#define Lo_CfrTilWord W_pw_CfrTilWord

#define Lo_Object W_pO_Object 
#define Lo_List W_dll_SymbolList

//#define Lo_ObjectValue Lo_Object->O_Value
#define Lo_Value Lo_Object
#define Lo_ObjectValue Lo_Value
#define Lo_ObjectData Lo_Value

#define Lo_Procedure Lo_Value
#define Lo_SymbolString Lo_ObjectValue
#define Lo_UInteger Lo_ObjectData
#define Lo_Integer Lo_ObjectData
#define Lo_String Lo_ObjectData
#define Lo_LambdaFunctionParameters W_WordData->LambdaArgs
#define Lo_LambdaFunctionBody W_WordData->LambdaBody
//#define Lo_LambdaVals W_WordData->LambdaVals

// to keep using existing code without rewriting ...
#define Definition W_WordData->Definition 
//#define po_WD_Object W_WordData->WD_po_Object
#define bp_WD_Object W_WordData->WD_bp_Object
//#define plo_WD_ListObject W_WordData->WD_plo_ListObject
#define CodeStart W_WordData->CodeStart // set at Word allocation 
#define Coding W_WordData->Coding // nb : !! this field is set by the Interpreter and modified by the Compiler in some cases so we also need (!) CodeStart both are needed !!  
#define Offset W_WordData->Offset // used by ClassField
#define NumberOfArgs W_WordData->NumberOfArgs 
#define TtnReference W_WordData->TtnReference // used by Logic Words
#define Slots W_WordData->Slots // number of slots in Object
#define RunType W_WordData->RunType // number of slots in Object
#define Value W_WordData->Value
#define PtrObject W_WordData->PtrObject // necessary for compiling class words and variables -- might as well be used" otherwise
#define AccumulatedOffset W_WordData->AccumulatedOffset // used by Do_Object
#define Index W_WordData->Index // used by Variable and LocalWord
#define NestedObjects W_WordData->NestedObjects // used by Variable and LocalWord
#define ObjectCode W_WordData->ObjectCode // used by objects/class words
#define StackPushRegisterCode W_WordData->StackPushRegisterCode // used by Optimize
#define SourceCode W_WordData->SourceCode 
#define Filename W_WordData->Filename // ?? should be made a part of a accumulated string table ??
#define LineNumber W_WordData->LineNumber 
#define CursorPosition W_WordData->CursorPosition 
#define W_FunctionTypesArray W_WordData->FunctionTypesArray
//#define RegFlag s_W_WordData->RegFlag
#define RegToUse W_WordData->RegToUse
#define ArrayDimensions W_WordData->ArrayDimensions
#define AliasOf W_WordData->AliasOf
#define TypeNamespace W_WordData->TypeNamespace 
#define Lo_ListProc W_WordData->ListProc
#define Lo_ListFirst W_WordData->ListFirst

typedef struct
{
    Symbol s_Symbol;
    int32 State;
    byte *FrameStart;
    byte * AfterEspSave;
    byte *Start;
    byte *bp_First;
    byte *bp_Last;
    byte *JumpOffset;
    byte *LogicCode;
    byte *CombinatorStartsAt;
    byte *ActualCodeStart;
    int32 Ttt;
    int32 NegFlag;
    Word * LogicCodeWord, *LiteralWord;
    Namespace * LocalsNamespace;
} BlockInfo;

typedef struct
{
    int32 State;
    int32 OutputLineCharacterNumber;
} PrintStateInfo;

typedef struct
{

    union
    {

        struct
        {
            int32 Eax;
            int32 Ecx;
            int32 Edx;
            int32 Ebx;
            int32 Esp;
            int32 Ebp;
            int32 Esi;
            int32 Edi;
            int32 EFlags;
            int32 Eip;
        };
        int32 Registers [ 10 ];
    };
} CpuState;

typedef struct
{
    int32 StackSize;
    int32 *StackPointer;
    int32 *StackMin;
    int32 *StackMax;
    int32 *InitialTosPointer;
    int32 StackData [];
} Stack;

typedef struct TCI
{
    int32 State;
    int32 TokenFirstChar, TokenLastChar, EndDottedFlag, DotSeparator, TokenLength;
    byte *SearchToken, * PreviousIdentifier, *Identifier, *LastUpMove, *LastMove;
    Word * TrialWord, * OriginalWord, *RunWord, *OriginalRunWord, *LastNamespace, *MarkWord;
    Namespace * OriginalContainingNamespace, * MarkNamespace;
} TabCompletionInfo, TCI;

struct ReadLiner;
typedef byte(*ReadLiner_KeyFunction) (struct ReadLiner *);

typedef struct ReadLiner
{
    int32 State;
    ReadLiner_KeyFunction Key; //byte(*Key)( struct ReadLiner * );
    FILE *InputFile;
    FILE *OutputFile;
    byte *bp_Filename;

    byte InputKeyedCharacter;
    byte LastCheckedInputKeyedCharacter;
    int32 FileCharacterNumber;
    int32 i32_LineNumber;
    int32 InputLineCharacterNumber; // set by _CfrTil_Key
    int32 OutputLineCharacterNumber; // set by _CfrTil_Key
    int32 ReadIndex;
    int32 EndPosition; // index where the next input character is put
    int32 MaxEndPosition; // index where the next input character is put
    int32 i32_CursorPosition; //
    int32 EscapeModeFlag;
    byte * DebugPrompt;
    byte * NormalPrompt;
    byte * AltPrompt;
    byte * Prompt;
    HistoryStringNode * HistoryNode;
    TabCompletionInfo * TabCompletionInfo0;
    byte * InputLine;
    byte * InputStringOriginal;
    byte * InputStringCurrent;
    int32 InputStringIndex;
    int32 LineStartFileIndex;
    Stack * TciNamespaceStack;
} ReadLiner;
typedef void ( * ReadLineFunction) (ReadLiner *);

typedef struct
{
    int32 FinderFlags;
    Word *w_Word;
    Word *FoundWord;
    Word *LastWord;
    Namespace * FoundWordNamespace;
    Namespace * QualifyingNamespace;
} Finder;

struct Interpreter;

typedef struct Lexer
{
    int32 State;
    byte *OriginalToken;
    int32 Literal;
    uint64 TokenType;
    Word * TokenWord;
    byte TokenInputCharacter;
    byte CurrentTokenDelimiter;
    int32 TokenStart_ReadLineIndex;
    int32 TokenEnd_ReadLineIndex;
    byte * TokenDelimiters;
    byte * DelimiterCharSet;
    byte * BasicTokenDelimiters;
    byte * BasicDelimiterCharSet;
    byte * TokenDelimitersAndDot;
    byte * DelimiterOrDotCharSet;
    int32 CurrentReadIndex, TokenWriteIndex;
    struct Interpreter * OurInterpreter;
    ReadLiner * ReadLiner;
    byte(*NextChar) (ReadLiner * rl);
    byte * TokenBuffer;
} Lexer;

typedef struct
{
    int32 OptimizeFlag;
    int32 Optimize_Dest_RegOrMem;
    int32 Optimize_Mod;
    int32 Optimize_Reg;
    int32 Optimize_Rm;
    int32 Optimize_Disp;
    int32 Optimize_Imm;
    int32 Optimize_SrcReg;
    int32 Optimize_DstReg;
} CompileOptimizer;

typedef struct
{
    int32 State;
    byte *IfZElseOffset;
    byte *ContinuePoint; // used by 'continue'
    byte * BreakPoint;
    byte * StartPoint;
    int32 NumberOfLocals;
    int32 NumberOfStackVariables;
    int32 NumberOfRegisterVariables;
    int32 LocalsFrameSize;
    int32 SaveCompileMode;
    int32 LispParenLevel;
    int32 ParenLevel;
    int32 GlobalParenLevel;
    int32 BlockLevel;
    int32 ArrayEnds;
    //int32 ArrayIndex, PreviousArrayIndex ;
    //int32 ArrayIndexWithNonInteger;
    byte * InitHere;
    int32 * AccumulatedOptimizeOffsetPointer;
    int32 * AccumulatedOffsetPointer;
    int32 * AddOffsetCodePointer;
    int32 * FrameSizeCellOffset;
    byte * EspSaveOffset;
    byte * EspRestoreOffset;
    Word * ReturnVariableWord;
    Word * RecursiveWord;
    Word * CurrentWord;
    Word * CurrentCreatedWord;
    Word * LHS_Word;
    Namespace ** FunctionTypesArray, *C_BackgroundNamespace;
    DLList * GotoList;
    DLList * CurrentSwitchList;
    CompileOptimizer * Optimizer;
    Stack * CombinatorInfoStack;
    Stack * ObjectStack;
    Stack * PointerToOffset;
    Stack * WordStack;
    Stack * LocalNamespaces;
    Stack * CombinatorBlockInfoStack;
    Stack * BlockStack;
    Stack * NamespacesStack;
    Stack * InfixOperatorStack;
} Compiler;

typedef struct Interpreter
{
    int32 State;
    ReadLiner * ReadLiner;
    Finder * Finder;
    Lexer * Lexer;
    Compiler * Compiler;
    Word *w_Word;
    Word * BaseObject, *QidObject ;
    Word *ObjectField;
    Word *CurrentPrefixWord;
    Symbol * s_List;
    int32 InterpretStringBufferIndex;
    byte * Token; //InterpretStringBuffer;
    int32 * PrefixWord_SaveSP, ParenLevel;
} Interpreter;

struct _Debugger;
typedef void (* DebuggerFunction) (struct _Debugger *);

typedef struct _Debugger
{
    int32 State;
    int32 * SaveDsp;
    int32 * WordDsp;
    int32 SaveTOS;
    int32 SaveStackDepth;
    int32 Key;
    int32 SaveKey;
    Word * w_Word, *LastShowWord, *LastPreWord, *SteppedWord;
    byte * Token;
    block SaveCpuState;
    block RestoreCpuState;
    block GetEIP, GetESP, RestoreEbpEsp;
    int32 TokenEnd_FileCharNumber;
    byte * OptimizedCodeAffected;
    byte * PreHere;
    Stack *DebugStack;
    CpuState * cs_CpuState;
    CpuState * cs_CpuState_Entry;
    byte* DebugAddress;
    int32 * DebugESP;
    Stack *AddressAfterJmpCallStack;
    ByteArray * StepInstructionBA;
    byte CharacterTable [ 128 ];
    DebuggerFunction CharacterFunctionTable [ 30 ];
    ud_t * Udis;
    Namespace * Locals;
} Debugger;

typedef struct
{
    int32 State;
    int32 NumberBase;
    int32 BigNumPrecision;
    int32 BigNumWidth;
    int32 ExceptionFlag;
    int32 IncludeFileStackNumber;
    struct timespec Timers [ 8 ];
} System;

typedef struct
{
    int32 State;
    int32 MemoryType;
    ReadLiner *ReadLiner0;
    Lexer *Lexer0;
    Finder * Finder0;
    Interpreter * Interpreter0;
    Compiler *Compiler0;
    System * System0;
    Stack * ContextDataStack;
    byte * Location;
#if SIGJMP_BUF    
    sigjmp_buf JmpBuf0;
#else
    jmp_buf JmpBuf0;
#endif    
} Context;
typedef void (* ContextFunction_1) (Context * cntx, byte* arg);
typedef void (* ContextFunction) (Context * cntx);
typedef void (* LexerFunction) (Lexer *);

typedef struct _CombinatorInfo
{

    union
    {
        int32 CI_i32_Info;

        struct
        {
            unsigned BlockLevel : 16;
            unsigned ParenLevel : 16;
        };
    };
} CombinatorInfo;
struct _CfrTil;

typedef struct LambdaCalculus
{
    int32 State, DontCopyFlag, Loop, *SaveStackPtr;
    Namespace * LispTemporariesNamespace, *LispNamespace;
    ListObject * Nil, *True, *CurrentList, *CurrentFunction, *ListFirst;
    ByteArray * SavedCodeSpace;
    uint32 ItemQuoteState, QuoteState;
    struct _CfrTil * OurCfrTil;
    Stack * QuoteStateStack;
} LambdaCalculus;

typedef struct
{

    union
    {

        struct
        {
            unsigned CharFunctionTableIndex : 16;
            unsigned CharType : 16;
        };
        int32 CharInfo;
    };
} CharacterType;

typedef struct _StringTokenInfo
{
    int32 State;
    int32 StartIndex, EndIndex;
    byte * In, *Out, *Delimiters, *SMNamespace;
    CharSet * CharSet;
} StringTokenInfo, StrTokInfo;
// StrTokInfo State constants
#define STI_INITIALIZED     ( 1 << 0 )

typedef struct _CfrTil
{
    int32 State;
    Stack * DataStack;
#if RETURN_STACK
    Stack * ReturnStack;
#endif    
    Namespace * Namespaces;
    Context * Context0;
    Stack * ContextStack;
    Debugger * Debugger0;
    Stack * ObjectStack;
    Namespace * InNamespace, *LispNamespace;
    LambdaCalculus * LC;
    FILE * LogFILE;
    int LogFlag;
    int32 * SaveDsp;
    CpuState * cs_CpuState;
    block SaveCpuState;
    Word * LastFinishedWord, *StoreWord, *PokeWord;
    int32 DebuggerVerbosity;
#if SIGJMP_BUF    
    sigjmp_buf JmpBuf0;
#else
    jmp_buf JmpBuf0;
#endif    

    byte ReadLine_CharacterTable [ 256 ];
    ReadLineFunction ReadLine_FunctionTable [ 23 ];
    CharacterType LexerCharacterTypeTable [ 256 ];
    LexerFunction LexerCharacterFunctionTable [ 24 ];
    Buffer *StringB, * TokenB, *OriginalInputLineB, *InputLineB, *SourceCodeSPB, *StringInsertB, *StringInsertB2, *StringInsertB3;
    Buffer *TabCompletionB, * LambdaCalculusPB, *PrintfB, *DebugB, *DebugB2, *HistoryExceptionB, *StringMacroB; // token buffer, tab completion backup, source code scratch pad, 
    StrTokInfo Sti;
    byte * OriginalInputLine;
    byte * TokenBuffer;
    byte * SourceCodeScratchPad; // nb : keep this here -- if we add this field to Lexer it just makes the lexer bigger and we want the smallest lexer possible
    int32 SC_ScratchPadIndex;
    byte * LispPrintBuffer; // nb : keep this here -- if we add this field to Lexer it just makes the lexer bigger and we want the smallest lexer possible
    DLList * TokenList, *PeekTokenList;
    Word * CurrentRunWord ;
} CfrTil;

typedef struct
{
    // static buffers
    // short term memory
    NamedByteArray * SessionObjectsSpace; // until reset
    NamedByteArray * TempObjectSpace; // last for one line
    NamedByteArray * CompilerTempObjectSpace; // last for compile of one word
    // long term memory
    NamedByteArray * CodeSpace;
    NamedByteArray * ObjectSpace;
    NamedByteArray * DictionarySpace;
    NamedByteArray * BufferSpace;
    NamedByteArray * ContextSpace;
    NamedByteArray * HistorySpace;
    NamedByteArray * CfrTilInternalSpace;
    NamedByteArray * OpenVmTilSpace;
    NamedByteArray * LispTempSpace;
    DLList * NBAs;
    DLList * BufferList;
} MemorySpace;

typedef struct
{
    int Red, Green, Blue;
} RgbColor;

typedef struct
{
    RgbColor rgbc_Fg;
    RgbColor rgbc_Bg;
} RgbColors;

typedef struct
{
    int Fg;
    int Bg;
} IntColors;

typedef struct
{

    union
    {
        RgbColors rgbcs_RgbColors;
        IntColors ics_IntColors;
    };
} Colors;

typedef struct
{
    int32 State;
    CfrTil * CfrTil0;
    ByteArray * CodeByteArray; // a variable

    PrintStateInfo *psi_PrintStateInfo;
    int32 SignalExceptionsHandled;

    byte *InitString;
    byte *StartupString;
    byte *StartupFilename;
    byte *ErrorFilename;
    byte *VersionString;
    byte *ExceptionMessage;
    int32 RestartCondition;
    int32 Signal;

#if SIGJMP_BUF    
    sigjmp_buf JmpBuf0;
#else
    jmp_buf JmpBuf0;
#endif    
    int Thrown;

    int32 Argc;
    char ** Argv;
    void * SigAddress;
    byte * SigLocation;
    Colors *Current, Default, Alert, Debug, Notice, User;
    int32 Console;
#if NO_GLOBAL_REGISTERS  // NGR NO_GLOBAL_REGISTERS
    block Dsp_To_ESI;
    block ESI_To_Dsp;
#endif    

    DLList * PermanentMemList;
    MemorySpace * MemorySpace0;
    int32 MemAllocated, MemRemaining;
    int32 MmapMemoryAllocated, NumberOfByteArrays;

    // variables accessible from cfrTil
    int32 Verbosity;
    int32 StartIncludeTries;
    int32 StartedTimes;
    int32 DictionarySize;
    int32 LispTempSize;
    int32 MachineCodeSize;
    int32 ObjectsSize;
    int32 ContextSize;
    int32 TempObjectsSize;
    int32 CompilerTempObjectsSize;
    int32 SessionObjectsSize;
    int32 DataStackSize;
    int32 HistorySize;
} OpenVmTil;

typedef struct
{
    NamedByteArray * HistorySpaceNBA;
    DLNode _StringList_HeadNode, _StringList_TailNode;
    DLList _StringList, * StringList;
} HistorySpace;

// note : this puts these namespaces on the search list such that last, in the above list, will be searched first

typedef struct
{
    const char * ccp_Name;
    block blk_Definition;
    uint64 ui64_CType;
    uint64 ui64_LType;
    const char *NameSpace;
    const char * SuperNamespace;
} CPrimitive;

// ( byte * name, int32 value, uint64 ctype, uint64 ltype, uint64 ftype, byte * function, int arg, int32 addToInNs, Namespace * addToNs, int32 allocType )
// ( const char * name, block b, uint64 ctype, uint64 ltype, const char *nameSpace, const char * superNamespace )

typedef struct
{
    const char * ccp_Name;
    uint64 ui64_CType;
    block blk_CallHook;
    byte * Function;
    int32 i32_FunctionArg;
    const char *NameSpace;
    const char * SuperNamespace;
} MachineCodePrimitive;



