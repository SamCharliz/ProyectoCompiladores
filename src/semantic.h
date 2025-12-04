/* semantic.h - Análisis semántico y verificación de tipos */
#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symtable.h"

// Resultado del análisis semántico
typedef struct {
    int hasErrors;
    int errorCount;
    int warningCount;
} SemanticResult;

// Funciones principales
int checkSemantics(ASTNode* root);
VarType inferType(ASTNode* node);
int checkTypeCompatibility(VarType t1, VarType t2);

// Validaciones específicas
int validateVariableUsage(ASTNode* node);
int validateArrayAccess(ASTNode* node);
int validateFunctionCall(ASTNode* node);
int validateAssignment(ASTNode* node);

// Mensajes de error
void semanticError(const char* format, ...);
void semanticWarning(const char* format, ...);

// Helpers
int isNumericType(VarType type);
int isBooleanType(VarType type);
const char* nodeTypeToString(NodeType type);

#endif