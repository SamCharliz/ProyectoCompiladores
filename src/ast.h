/* ast.h - Árbol de Sintaxis Abstracta */
#ifndef AST_H
#define AST_H

// Tipos de variables
typedef enum {
    TYPE_INT_T,
    TYPE_FLOAT_T,
    TYPE_BOOL_T,
    TYPE_STRING_T,
    TYPE_VOID_T
} VarType;

// Tipos de nodos del AST
typedef enum {
    NODE_INT,
    NODE_FLOAT,
    NODE_BOOL,
    NODE_STRING,
    NODE_ID,
    NODE_ASSIGN,
    NODE_ARRAY_DECL,
    NODE_ARRAY_ACCESS,
    NODE_ARRAY_LENGTH,
    NODE_PIXEL,
    NODE_KEY,
    NODE_INPUT,
    NODE_PRINT,
    NODE_IF,
    NODE_IF_ELSE,
    NODE_WHILE,
    NODE_FOR,
    NODE_FUNCTION,
    NODE_CALL,
    NODE_RETURN,
    NODE_SEQ,
    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_DIV,
    NODE_MOD,
    NODE_LT,
    NODE_GT,
    NODE_LTE,
    NODE_GTE,
    NODE_EQ,
    NODE_NEQ,
    NODE_AND,
    NODE_OR,
    NODE_NOT,
    NODE_BLOCK
} NodeType;

// Nodo del AST
typedef struct ASTNode {
    NodeType type;
    VarType varType;
    
    // Valores literales
    int intValue;
    float floatValue;
    char* idName;
    char* stringValue;
    
    // Expresiones binarias
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *extra;
    
    // Control de flujo
    struct ASTNode *cond;
    struct ASTNode *body;
    struct ASTNode *elseBody;
    
    // For loop
    struct ASTNode *init;
    struct ASTNode *increment;
    
    // Funciones
    struct ASTNode *params;
    struct ASTNode *args;
    struct ASTNode *next;
    
    // Arreglos
    struct ASTNode *index;
    int arraySize;
} ASTNode;

// Variable global para el AST
extern ASTNode* root;

// Funciones de creación de nodos
ASTNode* newInt(int val);
ASTNode* newFloat(float val);
ASTNode* newBool(int val);
ASTNode* newString(char* str);
ASTNode* newId(char* name);
ASTNode* newAssign(char* name, ASTNode* val);
ASTNode* newArrayDecl(VarType type, char* name, ASTNode* size);
ASTNode* newArrayAccess(char* name, ASTNode* index);
ASTNode* newArrayLength(char* name);
ASTNode* newPixel(ASTNode* x, ASTNode* y, ASTNode* c);
ASTNode* newKey(int keyNum, char* dest);
ASTNode* newInput(char* dest);
ASTNode* newPrint(ASTNode* expr);
ASTNode* newSeq(ASTNode* first, ASTNode* second);
ASTNode* newBinOp(NodeType op, ASTNode* left, ASTNode* right);
ASTNode* newUnaryOp(NodeType op, ASTNode* operand);
ASTNode* newIf(ASTNode* cond, ASTNode* body);
ASTNode* newIfElse(ASTNode* cond, ASTNode* body, ASTNode* elseBody);
ASTNode* newWhile(ASTNode* cond, ASTNode* body);
ASTNode* newFor(ASTNode* init, ASTNode* cond, ASTNode* inc, ASTNode* body);
ASTNode* newFunction(VarType retType, char* name, ASTNode* params, ASTNode* body);
ASTNode* newCall(char* name, ASTNode* args);
ASTNode* newReturn(ASTNode* expr);
ASTNode* newBlock(ASTNode* stmts);

// Utilidades
void printAST(ASTNode* node, int indent);
void freeAST(ASTNode* node);

#endif