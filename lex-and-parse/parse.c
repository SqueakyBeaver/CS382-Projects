#include "parse.h"
#include "lex.h"

#include <stdio.h>
#include <stdlib.h>

/*
Parser EBNF grammar:
<expr> -> <term> { (+ | -) <term> }
<term> -> <factor> { (* | /) <factor> }
<factor> -> <id> | <int> | (<expr>)
*/

// Macro magic because I want to do things my way, but variadic functions suck
// I know macros are really bad practice, but this makes things work my way

// Change the constant for a different tab size in the output
// (It's really just a number of spaces)
#define TABS layer * 2

// Output to both stdout and the output file with the appropriate amount of
// indentation
#define OUTPUT(...)                                                            \
    printf("%*.*s", TABS, TABS, " ");                                          \
    fprintf(out_fp, "%*.*s", TABS, TABS, " ");                                 \
    printf(__VA_ARGS__);                                                       \
    fprintf(out_fp, __VA_ARGS__);

// Used for knowing how many tabs to output
// Only callers should increase this
// Only callees should descrease this
unsigned int layer = 0;

// Parses the EBNF rule:
// <expr> -> <term> { (+ | -) <term> }
void expr() {
    OUTPUT("[expr\n");

    // Parse the first term
    layer++;
    term();

    // If there's a + or -, get the next token, then parse the next term
    while (nextToken == ADD_OP || nextToken == SUB_OP) {
        OUTPUT("[%s]\n", lexeme);
        lex();
        term();
    }

    layer--;
    OUTPUT("]\n");
}

// Parses the EBNF rule:
// <term> -> <factor> { (* | /) <factor> }
void term() {
    OUTPUT("[term\n");

    layer++;
    factor();

    while (nextToken == MULT_OP || nextToken == DIV_OP) {
        OUTPUT("[%s]\n", lexeme);
        lex();
        factor();
    }

    layer--;
    OUTPUT("]\n");
}

// Parses the EBNF rule:
// <factor> -> <id> | <int> | ( <expr> )
void factor() {
    OUTPUT("[factor\n");

    if (nextToken == IDENT || nextToken == INT_LIT) {
        // Warning: we must print the lexeme before we lex or we skip the
        // current lexeme!!!
        layer++;
        OUTPUT("[id [%s] ]\n", lexeme);

        lex();
        layer--;
        OUTPUT("]\n");
        return;
    }

    if (nextToken != LEFT_PAREN)
        return error("parsing factor",
                     "Factor is not a valid ID, integer literal, or "
                     "parenthesis-wrapped expression");

    lex();
    layer++;
    expr();

    if (nextToken == RIGHT_PAREN)
        lex();
    else
        return error("parsing factor",
                     "Mismatched parenthesis or error parsing parentheses");

    layer--;
    OUTPUT("]\n");
}

/**
 * Print an error to the console and terminate the program
 * action: What was happening when the error occured
 * msg: What the error is
 */
void error(const char *action, const char *msg) {
    printf("Error when %s: %s", action, msg);
    exit(-1);
}