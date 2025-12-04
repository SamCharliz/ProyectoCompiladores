/* codegen.h - Generación de código intermedio FIS-25 */
#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

// Contadores globales
extern int labelCount;
extern int tempCount;

// Funciones principales
void generateCode(ASTNode* node);
char* generateExpr(ASTNode* node);

// Generación de elementos específicos
void generateDeclarations(ASTNode* node);
void generateStatement(ASTNode* node);
void generateControlFlow(ASTNode* node);
void generateArrayCode(ASTNode* node);
void generateFunctionCode(ASTNode* node);

// Helpers
char* newTemp();
char* newLabel();
void declareVar(char* name);

// Información de generación
int getLabelCount();
int getTempCount();

#endif