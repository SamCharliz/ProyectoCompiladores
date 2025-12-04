/* symtable.h - Tabla de símbolos para análisis semántico */
#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "ast.h"

// Tipos de símbolos
typedef enum {
    SYM_VARIABLE,
    SYM_ARRAY,
    SYM_FUNCTION,
    SYM_PARAMETER
} SymbolKind;

// Entrada en la tabla de símbolos
typedef struct Symbol {
    char* name;
    VarType type;
    SymbolKind kind;
    
    // Para variables
    int isInitialized;
    
    // Para arreglos
    int arraySize;
    
    // Para funciones
    VarType returnType;
    int paramCount;
    struct Symbol* params;  // Lista de parámetros
    
    // Scope
    int scopeLevel;
    
    // Lista enlazada
    struct Symbol* next;
} Symbol;

// Funciones públicas
void initSymbolTable();
void enterScope();
void exitScope();

Symbol* addSymbol(char* name, VarType type, SymbolKind kind);
Symbol* findSymbol(char* name);
Symbol* findSymbolInCurrentScope(char* name);

void setArraySize(Symbol* sym, int size);
void setFunctionReturn(Symbol* sym, VarType retType);
void addParameter(Symbol* func, Symbol* param);

int getSymbolCount();
void printSymbolTable();

// Funciones auxiliares
const char* varTypeToString(VarType type);
const char* symbolKindToString(SymbolKind kind);

#endif