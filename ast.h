// ast.h
#ifndef AST_H
#define AST_H

typedef enum {
    NODE_INT,
    NODE_ID,
    NODE_ASSIGN,
    NODE_PIXEL, // Instrucción nativa FIS-25
    NODE_KEY,   // Instrucción nativa FIS-25
    NODE_IF,
    NODE_SEQ    // Secuencia de instrucciones
} NodeType;

typedef struct ASTNode {
    NodeType type;
    int intValue;          // Para números
    char* idName;          // Para variables
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *extra; // Para tercer argumento (ej. color en PIXEL)
} ASTNode;

// Funciones para crear nodos
ASTNode* newInt(int val);
ASTNode* newId(char* name);
ASTNode* newAssign(char* name, ASTNode* val);
ASTNode* newPixel(ASTNode* x, ASTNode* y, ASTNode* c);
ASTNode* newKey(int keyParams, char* destVar);
ASTNode* newSeq(ASTNode* first, ASTNode* second);

// Función de generación de código
void generateCode(ASTNode* node);

#endif