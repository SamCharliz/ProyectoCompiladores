#ifndef AST_H
#define AST_H

typedef enum {
    NODE_INT, NODE_FLOAT, NODE_BOOL, NODE_ID,
    NODE_ASSIGN, NODE_PIXEL, NODE_KEY,
    NODE_IF, NODE_IF_ELSE, NODE_WHILE, NODE_SEQ, NODE_BLOCK,
    NODE_ADD, NODE_SUB, NODE_MUL, NODE_DIV, NODE_MOD,
    NODE_LT, NODE_GT, NODE_LTE, NODE_GTE, NODE_EQ, NODE_NEQ,
    NODE_AND, NODE_OR
} NodeType;

typedef struct ASTNode {
    NodeType type;
    int intValue;          // Para ints y bools y flags
    float floatValue;      
    char* idName;          
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *extra; // Para 3er argumento (color en pixel, else en if)
    struct ASTNode *cond;  
    struct ASTNode *body;  
    struct ASTNode *elseBody;
} ASTNode;

ASTNode* newInt(int val);
ASTNode* newId(char* name);
// isDecl: 1 si es "int x = ...", 0 si es "x = ..."
ASTNode* newAssign(char* name, ASTNode* val, int isDecl); 
ASTNode* newPixel(ASTNode* x, ASTNode* y, ASTNode* c);
ASTNode* newKey(int keyVal, char* dest);
ASTNode* newSeq(ASTNode* first, ASTNode* second);
ASTNode* newBinOp(NodeType op, ASTNode* left, ASTNode* right);
ASTNode* newIf(ASTNode* cond, ASTNode* body);
ASTNode* newIfElse(ASTNode* cond, ASTNode* body, ASTNode* elseBody);
ASTNode* newWhile(ASTNode* cond, ASTNode* body);
ASTNode* newBlock(ASTNode* stmts);

#endif