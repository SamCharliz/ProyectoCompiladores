
/* main.c - Punto de entrada del compilador FIS-25 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symtable.h"
#include "semantic.h"
#include "codegen.h"

// Declaraciones externas de Bison/Flex
extern FILE* yyin;
extern int yyparse();
extern ASTNode* root;
extern int yylineno;

// Opciones del compilador
typedef struct {
    int verbose;
    int printAST;
    int skipSemantic;
    char* inputFile;
    char* outputFile;
} CompilerOptions;

void printUsage(char* progName) {
    printf("Uso: %s [opciones] archivo.fis\n", progName);
    printf("Opciones:\n");
    printf("  -o <archivo>   Archivo de salida (default: salida.fis25)\n");
    printf("  -v             Modo verbose (muestra detalles)\n");
    printf("  -a             Imprime el AST generado\n");
    printf("  -s             Omite análisis semántico\n");
    printf("  -h             Muestra esta ayuda\n");
}

void parseArguments(int argc, char** argv, CompilerOptions* opts) {
    // Valores por defecto
    opts->verbose = 0;
    opts->printAST = 0;
    opts->skipSemantic = 0;
    opts->inputFile = NULL;
    opts->outputFile = "salida.fis25";
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            opts->verbose = 1;
        } else if (strcmp(argv[i], "-a") == 0) {
            opts->printAST = 1;
        } else if (strcmp(argv[i], "-s") == 0) {
            opts->skipSemantic = 1;
        } else if (strcmp(argv[i], "-h") == 0) {
            printUsage(argv[0]);
            exit(0);
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                opts->outputFile = argv[++i];
            } else {
                fprintf(stderr, "Error: -o requiere un nombre de archivo\n");
                exit(1);
            }
        } else if (argv[i][0] != '-') {
            opts->inputFile = argv[i];
        } else {
            fprintf(stderr, "Opción desconocida: %s\n", argv[i]);
            printUsage(argv[0]);
            exit(1);
        }
    }
    
    if (!opts->inputFile) {
        fprintf(stderr, "Error: No se especificó archivo de entrada\n");
        printUsage(argv[0]);
        exit(1);
    }
}

int main(int argc, char** argv) {
    CompilerOptions opts;
    parseArguments(argc, argv, &opts);
    
    printf("╔════════════════════════════════════════╗\n");
    printf("║   COMPILADOR FIS-25                    ║\n");
    printf("║   Proyecto de Compiladores 2025        ║\n");
    printf("╚════════════════════════════════════════╝\n\n");
    
    // ========== FASE 1: ANÁLISIS LÉXICO Y SINTÁCTICO ==========
    if (opts.verbose) {
        printf("🔍 Fase 1: Análisis Léxico y Sintáctico\n");
        printf("   Archivo: %s\n", opts.inputFile);
    }
    
    yyin = fopen(opts.inputFile, "r");
    if (!yyin) {
        fprintf(stderr, "❌ Error: No se pudo abrir '%s'\n", opts.inputFile);
        return 1;
    }
    
    int parseResult = yyparse();
    fclose(yyin);
    
    if (parseResult != 0) {
        fprintf(stderr, "❌ Error de sintaxis en el archivo\n");
        return 1;
    }
    
    if (!root) {
        fprintf(stderr, "❌ Error: No se generó el AST\n");
        return 1;
    }
    
    printf("✅ Análisis sintáctico completado\n\n");
    
    // Imprimir AST si se solicita
    if (opts.printAST) {
        printf("🌳 Árbol de Sintaxis Abstracta:\n");
        printAST(root, 0);
        printf("\n");
    }
    
    // ========== FASE 2: ANÁLISIS SEMÁNTICO ==========
    if (!opts.skipSemantic) {
        if (opts.verbose) {
            printf("🔍 Fase 2: Análisis Semántico\n");
        }
        
        // Inicializar tabla de símbolos
        initSymbolTable();
        
        // Verificar tipos y semántica
        if (!checkSemantics(root)) {
            fprintf(stderr, "❌ Errores semánticos encontrados\n");
            return 1;
        }
        
        printf("✅ Análisis semántico completado\n\n");
        
        if (opts.verbose) {
            printf("📊 Tabla de Símbolos:\n");
            printSymbolTable();
            printf("\n");
        }
    }
    
    // ========== FASE 3: GENERACIÓN DE CÓDIGO ==========
    if (opts.verbose) {
        printf("🔍 Fase 3: Generación de Código Intermedio\n");
    }
    
    // Redirigir salida a archivo
    FILE* output = fopen(opts.outputFile, "w");
    if (!output) {
        fprintf(stderr, "❌ Error: No se pudo crear '%s'\n", opts.outputFile);
        return 1;
    }
    
    // Redirigir stdout temporalmente
    FILE* oldStdout = stdout;
    stdout = output;
    
    // Generar código FIS-25
    printf("// Compilador FIS-25\n");
    printf("// Archivo fuente: %s\n", opts.inputFile);
    printf("// Generado automáticamente\n\n");
    
    generateCode(root);
    
    // Restaurar stdout
    stdout = oldStdout;
    fclose(output);
    
    printf("✅ Código generado: %s\n\n", opts.outputFile);
    
    // ========== RESUMEN ==========
    printf("╔════════════════════════════════════════╗\n");
    printf("║   COMPILACIÓN EXITOSA                  ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf("📄 Archivo de salida: %s\n", opts.outputFile);
    printf("📊 Variables declaradas: %d\n", getSymbolCount());
    printf("🏷️  Etiquetas generadas: %d\n", getLabelCount());
    
    return 0;
}