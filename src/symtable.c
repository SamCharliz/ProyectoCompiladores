/* symtable.c - Implementación de la tabla de símbolos */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"

// Tabla hash simple (lista enlazada)
#define TABLE_SIZE 256

static Symbol* symbolTable[TABLE_SIZE];
static int currentScope = 0;
static int symbolCount = 0;

// Función hash simple
static unsigned int hash(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash % TABLE_SIZE;
}

void initSymbolTable() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        symbolTable[i] = NULL;
    }
    currentScope = 0;
    symbolCount = 0;
}

void enterScope() {
    currentScope++;
}

void exitScope() {
    // Eliminar símbolos del scope actual
    for (int i = 0; i < TABLE_SIZE; i++) {
        Symbol* sym = symbolTable[i];
        Symbol* prev = NULL;
        
        while (sym) {
            if (sym->scopeLevel == currentScope) {
                Symbol* toDelete = sym;
                if (prev) {
                    prev->next = sym->next;
                    sym = sym->next;
                } else {
                    symbolTable[i] = sym->next;
                    sym = symbolTable[i];
                }
                free(toDelete->name);
                free(toDelete);
                symbolCount--;
            } else {
                prev = sym;
                sym = sym->next;
            }
        }
    }
    currentScope--;
}

Symbol* addSymbol(char* name, VarType type, SymbolKind kind) {
    unsigned int index = hash(name);
    
    // Verificar si ya existe en el scope actual
    Symbol* existing = findSymbolInCurrentScope(name);
    if (existing) {
        fprintf(stderr, "Error semántico: Variable '%s' ya declarada en este scope\n", name);
        return NULL;
    }
    
    // Crear nuevo símbolo
    Symbol* sym = malloc(sizeof(Symbol));
    sym->name = strdup(name);
    sym->type = type;
    sym->kind = kind;
    sym->isInitialized = 0;
    sym->arraySize = 0;
    sym->returnType = TYPE_VOID_T;
    sym->paramCount = 0;
    sym->params = NULL;
    sym->scopeLevel = currentScope;
    
    // Insertar al inicio de la lista
    sym->next = symbolTable[index];
    symbolTable[index] = sym;
    
    symbolCount++;
    return sym;
}

Symbol* findSymbol(char* name) {
    unsigned int index = hash(name);
    Symbol* sym = symbolTable[index];
    
    // Buscar en todos los scopes (del actual hacia arriba)
    while (sym) {
        if (strcmp(sym->name, name) == 0) {
            return sym;
        }
        sym = sym->next;
    }
    return NULL;
}

Symbol* findSymbolInCurrentScope(char* name) {
    unsigned int index = hash(name);
    Symbol* sym = symbolTable[index];
    
    while (sym) {
        if (strcmp(sym->name, name) == 0 && sym->scopeLevel == currentScope) {
            return sym;
        }
        sym = sym->next;
    }
    return NULL;
}

void setArraySize(Symbol* sym, int size) {
    if (sym) {
        sym->arraySize = size;
    }
}

void setFunctionReturn(Symbol* sym, VarType retType) {
    if (sym) {
        sym->returnType = retType;
    }
}

void addParameter(Symbol* func, Symbol* param) {
    if (func) {
        param->next = func->params;
        func->params = param;
        func->paramCount++;
    }
}

int getSymbolCount() {
    return symbolCount;
}

void printSymbolTable() {
    printf("┌────────────────┬──────────┬──────────┬───────┬────────┐\n");
    printf("│ Nombre         │ Tipo     │ Clase    │ Scope │ Info   │\n");
    printf("├────────────────┼──────────┼──────────┼───────┼────────┤\n");
    
    for (int i = 0; i < TABLE_SIZE; i++) {
        Symbol* sym = symbolTable[i];
        while (sym) {
            printf("│ %-14s │ %-8s │ %-8s │ %5d │ ", 
                   sym->name, 
                   varTypeToString(sym->type),
                   symbolKindToString(sym->kind),
                   sym->scopeLevel);
            
            if (sym->kind == SYM_ARRAY) {
                printf("[%d]   ", sym->arraySize);
            } else if (sym->kind == SYM_FUNCTION) {
                printf("(%d)   ", sym->paramCount);
            } else {
                printf("       ");
            }
            printf("│\n");
            
            sym = sym->next;
        }
    }
    printf("└────────────────┴──────────┴──────────┴───────┴────────┘\n");
}

const char* varTypeToString(VarType type) {
    switch (type) {
        case TYPE_INT_T: return "int";
        case TYPE_FLOAT_T: return "float";
        case TYPE_BOOL_T: return "bool";
        case TYPE_STRING_T: return "string";
        case TYPE_VOID_T: return "void";
        default: return "unknown";
    }
}

const char* symbolKindToString(SymbolKind kind) {
    switch (kind) {
        case SYM_VARIABLE: return "var";
        case SYM_ARRAY: return "array";
        case SYM_FUNCTION: return "function";
        case SYM_PARAMETER: return "param";
        default: return "unknown";
    }
}