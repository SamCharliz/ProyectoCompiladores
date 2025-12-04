/* codegen.c - Generación de código intermedio FIS-25 OPTIMIZADO */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"
#include "symtable.h"

int labelCount = 0;
int tempCount = 0;

// Tabla de variables ya declaradas (para evitar duplicados)
static char* declaredVars[1000];
static int declaredCount = 0;

char* newTemp() {
    char* temp = malloc(20);
    sprintf(temp, "T%d", tempCount++);
    declareVar(temp);
    return temp;
}

char* newLabel() {
    char* label = malloc(20);
    sprintf(label, "L%d", labelCount++);
    return label;
}

void declareVar(char* name) {
    // Verificar si ya fue declarada
    for (int i = 0; i < declaredCount; i++) {
        if (strcmp(declaredVars[i], name) == 0) {
            return; // Ya existe
        }
    }
    
    printf("VAR %s\n", name);
    declaredVars[declaredCount++] = strdup(name);
}

int getLabelCount() {
    return labelCount;
}

int getTempCount() {
    return tempCount;
}

// NUEVA FUNCIÓN: Optimizar expresiones constantes
int isConstant(ASTNode* node) {
    return node && (node->type == NODE_INT || 
                    node->type == NODE_FLOAT || 
                    node->type == NODE_BOOL);
}

char* generateExpr(ASTNode* node) {
    if (!node) return NULL;
    
    switch (node->type) {
        case NODE_INT: {
            // OPTIMIZACIÓN: Retornar el literal directamente si es pequeño
            if (node->intValue >= 0 && node->intValue <= 100) {
                char* literal = malloc(20);
                sprintf(literal, "%d", node->intValue);
                return literal;
            }
            char* temp = newTemp();
            printf("ASSIGN %d %s\n", node->intValue, temp);
            return temp;
        }
        
        case NODE_FLOAT: {
            char* temp = newTemp();
            printf("ASSIGN %.6f %s\n", node->floatValue, temp);
            return temp;
        }
        
        case NODE_BOOL: {
            char* temp = newTemp();
            printf("ASSIGN %d %s\n", node->intValue, temp);
            return temp;
        }
        
        case NODE_ID:
            return node->idName;
        
        case NODE_ARRAY_ACCESS: {
            // Simplificado: solo índices constantes
            if (node->index->type == NODE_INT) {
                char* varName = malloc(100);
                sprintf(varName, "%s_%d", node->idName, node->index->intValue);
                return varName;
            } else {
                // Índice dinámico - más complejo
                char* indexVar = generateExpr(node->index);
                char* temp = newTemp();
                printf("// TODO: Acceso dinámico arr[%s]\n", indexVar);
                return temp;
            }
        }
        
        case NODE_ARRAY_LENGTH: {
            char* lengthVar = malloc(100);
            sprintf(lengthVar, "%s_length", node->idName);
            return lengthVar;
        }
        
        // OPTIMIZACIÓN: Operaciones con constantes
        case NODE_ADD: {
            // Si ambos son constantes, no generar código
            if (isConstant(node->left) && isConstant(node->right)) {
                int result = node->left->intValue + node->right->intValue;
                char* literal = malloc(20);
                sprintf(literal, "%d", result);
                return literal;
            }
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("ADD %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_SUB: {
            if (isConstant(node->left) && isConstant(node->right)) {
                int result = node->left->intValue - node->right->intValue;
                char* literal = malloc(20);
                sprintf(literal, "%d", result);
                return literal;
            }
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("SUB %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_MUL: {
            if (isConstant(node->left) && isConstant(node->right)) {
                int result = node->left->intValue * node->right->intValue;
                char* literal = malloc(20);
                sprintf(literal, "%d", result);
                return literal;
            }
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("MUL %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_DIV: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("DIV %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_MOD: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("MOD %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_EQ: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("EQ %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_NEQ: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("NEQ %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_LT: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("LT %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_GT: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("GT %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_LTE: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("LTE %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_GTE: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("GTE %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_AND: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("MUL %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_OR: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* sum = newTemp();
            char* result = newTemp();
            printf("ADD %s %s %s\n", left, right, sum);
            printf("GT %s 0 %s\n", sum, result);
            return result;
        }
        
        case NODE_NOT: {
            char* operand = generateExpr(node->left);
            char* result = newTemp();
            printf("EQ %s 0 %s\n", operand, result);
            return result;
        }
        
        default:
            return newTemp();
    }
}

void generateCode(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_SEQ:
            generateCode(node->left);
            generateCode(node->right);
            break;
        
        case NODE_BLOCK:
            generateCode(node->body);
            break;
        
        case NODE_ASSIGN:
            declareVar(node->idName);
            
            // OPTIMIZACIÓN: Asignaciones directas sin temporales
            if (node->left->type == NODE_INT) {
                printf("ASSIGN %d %s\n", node->left->intValue, node->idName);
            } else if (node->left->type == NODE_FLOAT) {
                printf("ASSIGN %.6f %s\n", node->left->floatValue, node->idName);
            } else if (node->left->type == NODE_BOOL) {
                printf("ASSIGN %d %s\n", node->left->intValue, node->idName);
            } else if (node->left->type == NODE_ID) {
                // Asignación directa variable a variable
                printf("ASSIGN %s %s\n", node->left->idName, node->idName);
            } else {
                char* expr = generateExpr(node->left);
                printf("ASSIGN %s %s\n", expr, node->idName);
            }
            break;
        
        case NODE_ARRAY_DECL: {
            // Declarar cada elemento del arreglo
            for (int i = 0; i < node->arraySize; i++) {
                char varName[100];
                sprintf(varName, "%s_%d", node->idName, i);
                declareVar(varName);
            }
            // Variable para length
            char lenName[100];
            sprintf(lenName, "%s_length", node->idName);
            declareVar(lenName);
            printf("ASSIGN %d %s\n", node->arraySize, lenName);
            break;
        }
        
        case NODE_PIXEL: {
            char* x = generateExpr(node->left);
            char* y = generateExpr(node->right);
            char* c = generateExpr(node->extra);
            printf("PIXEL %s %s %s\n", x, y, c);
            break;
        }
        
        case NODE_KEY:
            declareVar(node->idName);
            printf("KEY %d %s\n", node->intValue, node->idName);
            break;
        
        case NODE_INPUT:
            declareVar(node->idName);
            printf("INPUT %s\n", node->idName);
            break;
        
        case NODE_PRINT:
            if (node->left->type == NODE_STRING) {
                printf("PRINT \"%s\"\n", node->left->stringValue);
            } else {
                char* expr = generateExpr(node->left);
                printf("PRINT %s\n", expr);
            }
            break;
        
        case NODE_IF: {
            char* cond = generateExpr(node->cond);
            char* labelEnd = newLabel();
            
            printf("IFFALSE %s GOTO %s\n", cond, labelEnd);
            generateCode(node->body);
            printf("LABEL %s\n", labelEnd);
            break;
        }
        
        case NODE_IF_ELSE: {
            char* cond = generateExpr(node->cond);
            char* labelElse = newLabel();
            char* labelEnd = newLabel();
            
            printf("IFFALSE %s GOTO %s\n", cond, labelElse);
            generateCode(node->body);
            printf("GOTO %s\n", labelEnd);
            printf("LABEL %s\n", labelElse);
            generateCode(node->elseBody);
            printf("LABEL %s\n", labelEnd);
            break;
        }
        
        case NODE_WHILE: {
            char* labelStart = newLabel();
            char* labelEnd = newLabel();
            
            printf("LABEL %s\n", labelStart);
            char* cond = generateExpr(node->cond);
            printf("IFFALSE %s GOTO %s\n", cond, labelEnd);
            generateCode(node->body);
            printf("GOTO %s\n", labelStart);
            printf("LABEL %s\n", labelEnd);
            break;
        }
        
        case NODE_FOR: {
            generateCode(node->init);
            
            char* labelStart = newLabel();
            char* labelEnd = newLabel();
            
            printf("LABEL %s\n", labelStart);
            char* cond = generateExpr(node->cond);
            printf("IFFALSE %s GOTO %s\n", cond, labelEnd);
            
            generateCode(node->body);
            generateCode(node->increment);
            
            printf("GOTO %s\n", labelStart);
            printf("LABEL %s\n", labelEnd);
            break;
        }
        
        case NODE_RETURN:
            if (node->left) {
                char* expr = generateExpr(node->left);
                printf("RETURN %s\n", expr);
            } else {
                printf("RETURN\n");
            }
            break;
        
        default:
            break;
    }
}