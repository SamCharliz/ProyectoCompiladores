#ifndef AST_H
#define AST_H

typedef enum {
    NODE_INT,
    NODE_FLOAT,
    NODE_ID,
    NODE_ASSIGN,
    NODE_PIXEL,     
    NODE_KEY,       
    NODE_IF,        
    NODE_IF_ELSE,   
    NODE_WHILE,     
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
    NODE_AND,       // <--- IMPORTANTE: Debe tener esto
    NODE_OR,        // <--- IMPORTANTE: Y esto
    NODE_BLOCK      
} NodeType;

typedef struct ASTNode {
    NodeType type;
    int intValue;          
    float floatValue;      
    char* idName;          
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *extra; 
    struct ASTNode *cond;  
    struct ASTNode *body;  
    struct ASTNode *elseBody;
} ASTNode;

// Declaraciones
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

#endif