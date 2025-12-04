/* ast.c - Implementación del Árbol de Sintaxis Abstracta */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

// Variable global para la raíz del AST
//ASTNode* root = NULL;

/* ============================================================
   FUNCIONES AUXILIARES
   ============================================================ */

// Inicializa un nodo con valores por defecto
static ASTNode* initNode() {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Error: No se pudo asignar memoria para nodo AST\n");
        exit(1);
    }
    
    node->type = NODE_INT;
    node->varType = TYPE_VOID_T;  // ← CAMBIAR DE TYPE_INT_T A TYPE_VOID_T
    node->intValue = 0;
    node->floatValue = 0.0;
    node->idName = NULL;
    node->stringValue = NULL;
    node->left = NULL;
    node->right = NULL;
    node->extra = NULL;
    node->cond = NULL;
    node->body = NULL;
    node->elseBody = NULL;
    node->init = NULL;
    node->increment = NULL;
    node->params = NULL;
    node->args = NULL;
    node->next = NULL;
    node->index = NULL;
    node->arraySize = 0;
    
    return node;
}

/* ============================================================
   NODOS LITERALES
   ============================================================ */

ASTNode* newInt(int val) {
    ASTNode* node = initNode();
    node->type = NODE_INT;
    node->varType = TYPE_INT_T;
    node->intValue = val;
    return node;
}

ASTNode* newFloat(float val) {
    ASTNode* node = initNode();
    node->type = NODE_FLOAT;
    node->varType = TYPE_FLOAT_T;
    node->floatValue = val;
    return node;
}

ASTNode* newBool(int val) {
    ASTNode* node = initNode();
    node->type = NODE_BOOL;
    node->varType = TYPE_BOOL_T;
    node->intValue = val; // 0 o 1
    return node;
}

ASTNode* newString(char* str) {
    ASTNode* node = initNode();
    node->type = NODE_STRING;
    node->varType = TYPE_STRING_T;
    node->stringValue = strdup(str);
    return node;
}

ASTNode* newId(char* name) {
    ASTNode* node = initNode();
    node->type = NODE_ID;
    node->idName = strdup(name);
    return node;
}

/* ============================================================
   ASIGNACIONES Y DECLARACIONES
   ============================================================ */

ASTNode* newAssign(char* name, ASTNode* val) {
    ASTNode* node = initNode();
    node->type = NODE_ASSIGN;
    node->idName = strdup(name);
    node->left = val;
    return node;
}

ASTNode* newArrayDecl(VarType type, char* name, ASTNode* size) {
    ASTNode* node = initNode();
    node->type = NODE_ARRAY_DECL;
    node->varType = type;
    node->idName = strdup(name);
    node->left = size;
    
    // Si el tamaño es un literal, guardarlo
    if (size && size->type == NODE_INT) {
        node->arraySize = size->intValue;
    }
    
    return node;
}

ASTNode* newArrayAccess(char* name, ASTNode* index) {
    ASTNode* node = initNode();
    node->type = NODE_ARRAY_ACCESS;
    node->idName = strdup(name);
    node->index = index;
    return node;
}

ASTNode* newArrayLength(char* name) {
    ASTNode* node = initNode();
    node->type = NODE_ARRAY_LENGTH;
    node->idName = strdup(name);
    node->varType = TYPE_INT_T;
    return node;
}

/* ============================================================
   INSTRUCCIONES FIS-25
   ============================================================ */

ASTNode* newPixel(ASTNode* x, ASTNode* y, ASTNode* c) {
    ASTNode* node = initNode();
    node->type = NODE_PIXEL;
    node->left = x;
    node->right = y;
    node->extra = c;
    return node;
}

ASTNode* newKey(int keyNum, char* dest) {
    ASTNode* node = initNode();
    node->type = NODE_KEY;
    node->intValue = keyNum;
    node->idName = strdup(dest);
    return node;
}

ASTNode* newInput(char* dest) {
    ASTNode* node = initNode();
    node->type = NODE_INPUT;
    node->idName = strdup(dest);
    return node;
}

ASTNode* newPrint(ASTNode* expr) {
    ASTNode* node = initNode();
    node->type = NODE_PRINT;
    node->left = expr;
    return node;
}

/* ============================================================
   SECUENCIAS Y BLOQUES
   ============================================================ */

ASTNode* newSeq(ASTNode* first, ASTNode* second) {
    // Si alguno es NULL, retornar el otro
    if (!first) return second;
    if (!second) return first;
    
    ASTNode* node = initNode();
    node->type = NODE_SEQ;
    node->left = first;
    node->right = second;
    return node;
}

ASTNode* newBlock(ASTNode* stmts) {
    ASTNode* node = initNode();
    node->type = NODE_BLOCK;
    node->body = stmts;
    return node;
}

/* ============================================================
   OPERADORES BINARIOS
   ============================================================ */

ASTNode* newBinOp(NodeType op, ASTNode* left, ASTNode* right) {
    ASTNode* node = initNode();
    node->type = op;
    node->left = left;
    node->right = right;
    
    // Inferir tipo básico (se refinará en análisis semántico)
    switch (op) {
        case NODE_ADD:
        case NODE_SUB:
        case NODE_MUL:
        case NODE_DIV:
        case NODE_MOD:
            node->varType = TYPE_INT_T; // Puede ser float
            break;
        
        case NODE_LT:
        case NODE_GT:
        case NODE_LTE:
        case NODE_GTE:
        case NODE_EQ:
        case NODE_NEQ:
        case NODE_AND:
        case NODE_OR:
            node->varType = TYPE_BOOL_T;
            break;
        
        default:
            node->varType = TYPE_VOID_T;
            break;
    }
    
    return node;
}

/* ============================================================
   OPERADORES UNARIOS
   ============================================================ */

ASTNode* newUnaryOp(NodeType op, ASTNode* operand) {
    ASTNode* node = initNode();
    node->type = op;
    node->left = operand;
    
    if (op == NODE_NOT) {
        node->varType = TYPE_BOOL_T;
    }
    
    return node;
}

/* ============================================================
   ESTRUCTURAS DE CONTROL
   ============================================================ */

ASTNode* newIf(ASTNode* cond, ASTNode* body) {
    ASTNode* node = initNode();
    node->type = NODE_IF;
    node->cond = cond;
    node->body = body;
    return node;
}

ASTNode* newIfElse(ASTNode* cond, ASTNode* body, ASTNode* elseBody) {
    ASTNode* node = initNode();
    node->type = NODE_IF_ELSE;
    node->cond = cond;
    node->body = body;
    node->elseBody = elseBody;
    return node;
}

ASTNode* newWhile(ASTNode* cond, ASTNode* body) {
    ASTNode* node = initNode();
    node->type = NODE_WHILE;
    node->cond = cond;
    node->body = body;
    return node;
}

ASTNode* newFor(ASTNode* init, ASTNode* cond, ASTNode* inc, ASTNode* body) {
    ASTNode* node = initNode();
    node->type = NODE_FOR;
    node->init = init;
    node->cond = cond;
    node->increment = inc;
    node->body = body;
    return node;
}

/* ============================================================
   FUNCIONES
   ============================================================ */

ASTNode* newFunction(VarType retType, char* name, ASTNode* params, ASTNode* body) {
    ASTNode* node = initNode();
    node->type = NODE_FUNCTION;
    node->varType = retType;
    node->idName = strdup(name);
    node->params = params;
    node->body = body;
    return node;
}

ASTNode* newCall(char* name, ASTNode* args) {
    ASTNode* node = initNode();
    node->type = NODE_CALL;
    node->idName = strdup(name);
    node->args = args;
    return node;
}

ASTNode* newReturn(ASTNode* expr) {
    ASTNode* node = initNode();
    node->type = NODE_RETURN;
    node->left = expr;
    return node;
}

/* ============================================================
   UTILIDADES
   ============================================================ */

// Convierte NodeType a string para debugging
static const char* nodeTypeToString(NodeType type) {
    switch (type) {
        case NODE_INT: return "INT";
        case NODE_FLOAT: return "FLOAT";
        case NODE_BOOL: return "BOOL";
        case NODE_STRING: return "STRING";
        case NODE_ID: return "ID";
        case NODE_ASSIGN: return "ASSIGN";
        case NODE_ARRAY_DECL: return "ARRAY_DECL";
        case NODE_ARRAY_ACCESS: return "ARRAY_ACCESS";
        case NODE_ARRAY_LENGTH: return "ARRAY_LENGTH";
        case NODE_PIXEL: return "PIXEL";
        case NODE_KEY: return "KEY";
        case NODE_INPUT: return "INPUT";
        case NODE_PRINT: return "PRINT";
        case NODE_IF: return "IF";
        case NODE_IF_ELSE: return "IF_ELSE";
        case NODE_WHILE: return "WHILE";
        case NODE_FOR: return "FOR";
        case NODE_FUNCTION: return "FUNCTION";
        case NODE_CALL: return "CALL";
        case NODE_RETURN: return "RETURN";
        case NODE_SEQ: return "SEQ";
        case NODE_ADD: return "ADD";
        case NODE_SUB: return "SUB";
        case NODE_MUL: return "MUL";
        case NODE_DIV: return "DIV";
        case NODE_MOD: return "MOD";
        case NODE_LT: return "LT";
        case NODE_GT: return "GT";
        case NODE_LTE: return "LTE";
        case NODE_GTE: return "GTE";
        case NODE_EQ: return "EQ";
        case NODE_NEQ: return "NEQ";
        case NODE_AND: return "AND";
        case NODE_OR: return "OR";
        case NODE_NOT: return "NOT";
        case NODE_BLOCK: return "BLOCK";
        default: return "UNKNOWN";
    }
}

// Imprime el AST con indentación
void printAST(ASTNode* node, int indent) {
    if (!node) return;
    
    // Imprimir indentación
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    
    printf("<%s", nodeTypeToString(node->type));
    
    // Imprimir información adicional según el tipo
    switch (node->type) {
        case NODE_INT:
            printf(" value=%d", node->intValue);
            break;
        
        case NODE_FLOAT:
            printf(" value=%.2f", node->floatValue);
            break;
        
        case NODE_BOOL:
            printf(" value=%s", node->intValue ? "true" : "false");
            break;
        
        case NODE_STRING:
            printf(" value=\"%s\"", node->stringValue);
            break;
        
        case NODE_ID:
        case NODE_ASSIGN:
        case NODE_ARRAY_DECL:
        case NODE_ARRAY_ACCESS:
        case NODE_ARRAY_LENGTH:
        case NODE_INPUT:
        case NODE_KEY:
        case NODE_CALL:
        case NODE_FUNCTION:
            if (node->idName) {
                printf(" name=\"%s\"", node->idName);
            }
            if (node->type == NODE_ARRAY_DECL) {
                printf(" size=%d", node->arraySize);
            }
            if (node->type == NODE_KEY) {
                printf(" keyNum=%d", node->intValue);
            }
            break;
        
        default:
            break;
    }
    
    printf(">\n");
    
    // Imprimir hijos recursivamente
    if (node->init) {
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("[init]\n");
        printAST(node->init, indent + 2);
    }
    
    if (node->cond) {
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("[cond]\n");
        printAST(node->cond, indent + 2);
    }
    
    if (node->body) {
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("[body]\n");
        printAST(node->body, indent + 2);
    }
    
    if (node->elseBody) {
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("[else]\n");
        printAST(node->elseBody, indent + 2);
    }
    
    if (node->left) {
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("[left]\n");
        printAST(node->left, indent + 2);
    }
    
    if (node->right) {
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("[right]\n");
        printAST(node->right, indent + 2);
    }
    
    if (node->extra) {
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("[extra]\n");
        printAST(node->extra, indent + 2);
    }
    
    if (node->increment) {
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("[increment]\n");
        printAST(node->increment, indent + 2);
    }
    
    if (node->index) {
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("[index]\n");
        printAST(node->index, indent + 2);
    }
    
    if (node->params) {
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("[params]\n");
        printAST(node->params, indent + 2);
    }
    
    if (node->args) {
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("[args]\n");
        printAST(node->args, indent + 2);
    }
    
    if (node->next) {
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("[next]\n");
        printAST(node->next, indent + 2);
    }
}

// Libera recursivamente la memoria del AST
void freeAST(ASTNode* node) {
    if (!node) return;
    
    // Liberar strings
    if (node->idName) free(node->idName);
    if (node->stringValue) free(node->stringValue);
    
    // Liberar hijos recursivamente
    freeAST(node->left);
    freeAST(node->right);
    freeAST(node->extra);
    freeAST(node->cond);
    freeAST(node->body);
    freeAST(node->elseBody);
    freeAST(node->init);
    freeAST(node->increment);
    freeAST(node->params);
    freeAST(node->args);
    freeAST(node->next);
    freeAST(node->index);
    
    // Liberar el nodo
    free(node);
}

/* ============================================================
   FUNCIONES DE DEBUGGING ADICIONALES
   ============================================================ */

// Cuenta el número de nodos en el AST
int countNodes(ASTNode* node) {
    if (!node) return 0;
    
    int count = 1; // Este nodo
    
    count += countNodes(node->left);
    count += countNodes(node->right);
    count += countNodes(node->extra);
    count += countNodes(node->cond);
    count += countNodes(node->body);
    count += countNodes(node->elseBody);
    count += countNodes(node->init);
    count += countNodes(node->increment);
    count += countNodes(node->params);
    count += countNodes(node->args);
    count += countNodes(node->next);
    count += countNodes(node->index);
    
    return count;
}

// Calcula la profundidad del AST
int treeDepth(ASTNode* node) {
    if (!node) return 0;
    
    int depths[12] = {
        treeDepth(node->left),
        treeDepth(node->right),
        treeDepth(node->extra),
        treeDepth(node->cond),
        treeDepth(node->body),
        treeDepth(node->elseBody),
        treeDepth(node->init),
        treeDepth(node->increment),
        treeDepth(node->params),
        treeDepth(node->args),
        treeDepth(node->next),
        treeDepth(node->index)
    };
    
    int maxDepth = 0;
    for (int i = 0; i < 12; i++) {
        if (depths[i] > maxDepth) {
            maxDepth = depths[i];
        }
    }
    
    return maxDepth + 1;
}

// Imprime estadísticas del AST
void printASTStats(ASTNode* root) {
    printf("\n╔════════════════════════════════════════╗\n");
    printf("║   ESTADÍSTICAS DEL AST                 ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf("  Nodos totales: %d\n", countNodes(root));
    printf("  Profundidad:   %d\n", treeDepth(root));
    printf("\n");
}