/* semantic.c - Análisis semántico y verificación de tipos */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "semantic.h"

static int errorCount = 0;
static int warningCount = 0;

void semanticError(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "❌ Error semántico: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
    errorCount++;
}

void semanticWarning(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "⚠️  Advertencia: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
    warningCount++;
}

int isNumericType(VarType type)
{
    return type == TYPE_INT_T || type == TYPE_FLOAT_T;
}

int isBooleanType(VarType type)
{
    return type == TYPE_BOOL_T;
}

int checkTypeCompatibility(VarType t1, VarType t2)
{
    // Tipos idénticos siempre son compatibles
    if (t1 == t2)
        return 1;

    // int y float son compatibles entre sí
    if (isNumericType(t1) && isNumericType(t2))
        return 1;

    // int y bool son compatibles (bool es int)
    if ((t1 == TYPE_INT_T && t2 == TYPE_BOOL_T) ||
        (t1 == TYPE_BOOL_T && t2 == TYPE_INT_T))
        return 1;

    return 0;
}

VarType inferType(ASTNode *node)
{
    if (!node)
        return TYPE_VOID_T;

    switch (node->type)
    {
    case NODE_INT:
    case NODE_BOOL:
        return TYPE_INT_T;

    case NODE_FLOAT:
        return TYPE_FLOAT_T;

    case NODE_STRING:
        return TYPE_STRING_T;

    case NODE_ID:
    {
        Symbol *sym = findSymbol(node->idName);
        if (sym)
        {
            return sym->type;
        }
        else
        {
            semanticError("Variable '%s' no declarada", node->idName);
            return TYPE_VOID_T;
        }
    }

    case NODE_ARRAY_ACCESS:
    {
        Symbol *sym = findSymbol(node->idName);
        if (sym && sym->kind == SYM_ARRAY)
        {
            return sym->type;
        }
        semanticError("'%s' no es un arreglo", node->idName);
        return TYPE_VOID_T;
    }

    case NODE_ADD:
    case NODE_SUB:
    case NODE_MUL:
    case NODE_DIV:
    case NODE_MOD:
    {
        VarType leftType = inferType(node->left);
        VarType rightType = inferType(node->right);

        if (!isNumericType(leftType) || !isNumericType(rightType))
        {
            semanticError("Operación aritmética con tipos no numéricos");
            return TYPE_VOID_T;
        }

        // Si alguno es float, el resultado es float
        if (leftType == TYPE_FLOAT_T || rightType == TYPE_FLOAT_T)
        {
            return TYPE_FLOAT_T;
        }
        return TYPE_INT_T;
    }

    case NODE_LT:
    case NODE_GT:
    case NODE_LTE:
    case NODE_GTE:
    case NODE_EQ:
    case NODE_NEQ:
        return TYPE_BOOL_T;

    case NODE_AND:
    case NODE_OR:
    case NODE_NOT:
        return TYPE_BOOL_T;

    default:
        return TYPE_VOID_T;
    }
}

int validateAssignment(ASTNode *node)
{
    if (node->type != NODE_ASSIGN)
        return 1;

    Symbol *sym = findSymbol(node->idName);
    if (!sym)
    {
        semanticError("Variable '%s' no declarada", node->idName);
        return 0;
    }

    VarType exprType = inferType(node->left);

    if (!checkTypeCompatibility(sym->type, exprType))
    {
        semanticError("Asignación de tipo incompatible: %s := %s",
                      varTypeToString(sym->type),
                      varTypeToString(exprType));
        return 0;
    }

    sym->isInitialized = 1;
    return 1;
}

int validateArrayAccess(ASTNode *node)
{
    if (node->type != NODE_ARRAY_ACCESS)
        return 1;

    Symbol *sym = findSymbol(node->idName);
    if (!sym)
    {
        semanticError("Arreglo '%s' no declarado", node->idName);
        return 0;
    }

    if (sym->kind != SYM_ARRAY)
    {
        semanticError("'%s' no es un arreglo", node->idName);
        return 0;
    }

    VarType indexType = inferType(node->index);
    if (indexType != TYPE_INT_T)
    {
        semanticError("El índice del arreglo debe ser int, es %s",
                      varTypeToString(indexType));
        return 0;
    }

    // Verificar rango si es constante
    if (node->index->type == NODE_INT)
    {
        int idx = node->index->intValue;
        if (idx < 0 || idx >= sym->arraySize)
        {
            semanticError("Índice %d fuera de rango [0, %d]",
                          idx, sym->arraySize - 1);
            return 0;
        }
    }

    return 1;
}

int validateVariableUsage(ASTNode *node)
{
    if (node->type != NODE_ID)
        return 1;

    Symbol *sym = findSymbol(node->idName);
    if (!sym)
    {
        semanticError("Variable '%s' no declarada", node->idName);
        return 0;
    }

    if (!sym->isInitialized)
    {
        semanticWarning("Variable '%s' usada antes de ser inicializada",
                        node->idName);
    }

    return 1;
}

int checkSemanticsRecursive(ASTNode *node)
{
    if (!node)
        return 1;

    int valid = 1;

    switch (node->type)
    {
    case NODE_SEQ:
        valid &= checkSemanticsRecursive(node->left);
        valid &= checkSemanticsRecursive(node->right);
        break;

    case NODE_BLOCK:
        enterScope();
        valid &= checkSemanticsRecursive(node->body);
        exitScope();
        break;

    case NODE_ASSIGN:
    {
        // DEBUG: Ver qué tipo tiene el nodo
        fprintf(stderr, "DEBUG: NODE_ASSIGN para '%s', varType=%d\n",
                node->idName, node->varType);

        // Verificar si es una declaración con inicialización
        if (node->varType != TYPE_VOID_T) // ← SOLO ESTA CONDICIÓN
        {
            fprintf(stderr, "DEBUG: Es declaración con tipo %d\n", node->varType);
            // Es una declaración: int x = 10;
            Symbol *sym = addSymbol(node->idName, node->varType, SYM_VARIABLE);
            if (sym)
            {
                sym->isInitialized = 1;
            }
            VarType exprType = inferType(node->left);
            if (!checkTypeCompatibility(node->varType, exprType))
            {
                semanticError("Declaración con tipo incompatible: %s := %s",
                              varTypeToString(node->varType),
                              varTypeToString(exprType));
                valid = 0;
            }
        }
        else
        {
            fprintf(stderr, "DEBUG: Es asignación simple\n");
            // Es una asignación simple o a arreglo
            if (node->index != NULL)
            {
                // Asignación a arreglo: arr[i] = 10;
                Symbol *sym = findSymbol(node->idName);
                if (!sym)
                {
                    semanticError("Arreglo '%s' no declarado", node->idName);
                    valid = 0;
                }
                else if (sym->kind != SYM_ARRAY)
                {
                    semanticError("'%s' no es un arreglo", node->idName);
                    valid = 0;
                }
                else
                {
                    // Validar tipo del índice
                    VarType indexType = inferType(node->index);
                    if (indexType != TYPE_INT_T && indexType != TYPE_VOID_T)
                    {
                        semanticError("El índice del arreglo debe ser int");
                        valid = 0;
                    }
                    // Validar tipo del valor
                    VarType exprType = inferType(node->left);
                    if (!checkTypeCompatibility(sym->type, exprType))
                    {
                        semanticError("Tipo incompatible en asignación a arreglo");
                        valid = 0;
                    }
                }
                valid &= checkSemanticsRecursive(node->index);
            }
            else
            {
                // Asignación simple a variable: x = 10;
                valid &= validateAssignment(node);
            }
        }
        valid &= checkSemanticsRecursive(node->left);
        break;
    }

    case NODE_ARRAY_DECL:
    {
        Symbol *sym = addSymbol(node->idName, node->varType, SYM_ARRAY);
        if (sym)
        {
            setArraySize(sym, node->arraySize);
        }
        break;
    }

    case NODE_ARRAY_ACCESS:
        valid &= validateArrayAccess(node);
        valid &= checkSemanticsRecursive(node->index);
        break;

    case NODE_IF:
    case NODE_IF_ELSE:
    {
        VarType condType = inferType(node->cond);
        if (!isBooleanType(condType) && condType != TYPE_INT_T)
        {
            semanticError("Condición de if debe ser booleana o int");
            valid = 0;
        }
        valid &= checkSemanticsRecursive(node->cond);
        valid &= checkSemanticsRecursive(node->body);
        if (node->type == NODE_IF_ELSE)
        {
            valid &= checkSemanticsRecursive(node->elseBody);
        }
        break;
    }

    case NODE_WHILE:
    case NODE_FOR:
    {
        VarType condType = inferType(node->cond);
        if (!isBooleanType(condType) && condType != TYPE_INT_T)
        {
            semanticError("Condición de bucle debe ser booleana o int");
            valid = 0;
        }
        if (node->type == NODE_FOR)
        {
            valid &= checkSemanticsRecursive(node->init);
            valid &= checkSemanticsRecursive(node->increment);
        }
        valid &= checkSemanticsRecursive(node->cond);
        valid &= checkSemanticsRecursive(node->body);
        break;
    }

    case NODE_ID:
        valid &= validateVariableUsage(node);
        break;

    case NODE_ADD:
    case NODE_SUB:
    case NODE_MUL:
    case NODE_DIV:
    case NODE_MOD:
    case NODE_LT:
    case NODE_GT:
    case NODE_LTE:
    case NODE_GTE:
    case NODE_EQ:
    case NODE_NEQ:
    case NODE_AND:
    case NODE_OR:
        valid &= checkSemanticsRecursive(node->left);
        valid &= checkSemanticsRecursive(node->right);
        // El tipo se verifica en inferType
        (void)inferType(node);
        break;

    case NODE_NOT:
        valid &= checkSemanticsRecursive(node->left);
        break;

    case NODE_PIXEL:
        valid &= checkSemanticsRecursive(node->left);
        valid &= checkSemanticsRecursive(node->right);
        valid &= checkSemanticsRecursive(node->extra);
        break;

    case NODE_PRINT:
    case NODE_RETURN:
        valid &= checkSemanticsRecursive(node->left);
        break;

    default:
        // Otros nodos no requieren validación especial
        break;
    }

    return valid;
}

int checkSemantics(ASTNode *root)
{
    errorCount = 0;
    warningCount = 0;

    int valid = checkSemanticsRecursive(root);

    if (errorCount > 0)
    {
        fprintf(stderr, "\n❌ Total de errores semánticos: %d\n", errorCount);
    }
    if (warningCount > 0)
    {
        fprintf(stderr, "⚠️  Total de advertencias: %d\n", warningCount);
    }

    return (errorCount == 0);
}

const char *nodeTypeToString(NodeType type)
{
    switch (type)
    {
    case NODE_INT:
        return "INT";
    case NODE_FLOAT:
        return "FLOAT";
    case NODE_BOOL:
        return "BOOL";
    case NODE_STRING:
        return "STRING";
    case NODE_ID:
        return "ID";
    case NODE_ASSIGN:
        return "ASSIGN";
    case NODE_ADD:
        return "ADD";
    case NODE_SUB:
        return "SUB";
    case NODE_MUL:
        return "MUL";
    case NODE_DIV:
        return "DIV";
    default:
        return "UNKNOWN";
    }
}