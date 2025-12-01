// ast.h - Versión completa con soporte FIS-25
#ifndef AST_H
#define AST_H

typedef enum {
    NODE_INT,
    NODE_FLOAT,
    NODE_ID,
    NODE_ASSIGN,
    NODE_PIXEL,     // PIXEL x y c
    NODE_KEY,       // KEY k dest
    NODE_IF,        // if (cond) { ... }
    NODE_IF_ELSE,   // if (cond) { ... } else { ... }
    NODE_WHILE,     // while (cond) { ... }
    NODE_SEQ,       // Secuencia de instrucciones
    NODE_ADD,       // Suma
    NODE_SUB,       // Resta
    NODE_MUL,       // Multiplicación
    NODE_DIV,       // División
    NODE_MOD,       // Módulo
    NODE_LT,        // Menor que <
    NODE_GT,        // Mayor que >
    NODE_LTE,       // Menor o igual <=
    NODE_GTE,       // Mayor o igual >=
    NODE_EQ,        // Igual ==
    NODE_NEQ,       // Diferente !=
    NODE_BLOCK      // Bloque { ... }
} NodeType;

typedef struct ASTNode {
    NodeType type;
    int intValue;          // Para números enteros
    float floatValue;      // Para números flotantes
    char* idName;          // Para variables
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *extra; // Para tercer argumento
    struct ASTNode *cond;  // Para condiciones en IF/WHILE
    struct ASTNode *body;  // Para cuerpo de IF/WHILE
    struct ASTNode *elseBody; // Para cuerpo de ELSE
} ASTNode;

// Funciones para crear nodos
ASTNode* newInt(int val);
ASTNode* newFloat(float val);
ASTNode* newId(char* name);
ASTNode* newAssign(char* name, ASTNode* val);
ASTNode* newPixel(ASTNode* x, ASTNode* y, ASTNode* c);
ASTNode* newKey(int keyParams, char* destVar);
ASTNode* newSeq(ASTNode* first, ASTNode* second);
ASTNode* newBinOp(NodeType op, ASTNode* left, ASTNode* right);
ASTNode* newIf(ASTNode* cond, ASTNode* body);
ASTNode* newIfElse(ASTNode* cond, ASTNode* body, ASTNode* elseBody);
ASTNode* newWhile(ASTNode* cond, ASTNode* body);
ASTNode* newBlock(ASTNode* stmts);

// Tabla de símbolos simple
typedef struct {
    char* name;
    int value;
} Symbol;

extern Symbol symTable[100];
extern int symCount;

// Declaración de variables
void declareVar(char* name);

// Generador de temporales
char* newTemp();

// Generación de código
void generateCode(ASTNode* node);
char* generateExpr(ASTNode* node);

#endif