// Gabriel Alves
// Day-Month-Year = 01/10/2025
// COP 3402 - HW2

/*
    Assignment :
    lex - Lexical Analyzer for PL /0

    Author : < Gabriel Alves >
    
    Language : C ( only )
    
    To Compile :
    gcc -O2 -std=c11 -o lex lex.c
    
    To Execute ( on Eustis ):
    ./ lex < input file >
    
    where :
    < input file > is the path to the PL /0 source program
    
    Notes :
    - Implement a lexical analyser for the PL /0 language .
    - The program must detect errors such as
        - numbers longer than five digits
        - identifiers longer than eleven characters
        - invalid characters .
    - The output format must exactly match the specification .
    - Tested on Eustis .
    
    Class : COP 3402 - System Software - Fall 2025
    
    Instructor : Dr . Jie Lin
    
    Due Date : Friday , October 3 , 2025 at 11:59 PM ET
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKEN_SIZE 500

#define ERR_FREE 0
#define ID_TOO_LONG 1
#define NUM_TOO_LONG 2
#define INVALID_SYMBOL 3

#define RESERVED_WORD_COUNT 15
#define SPECIAL_SYMBOL_COUNT 16

int lexCount = 0;

typedef enum {
    skipsym = 1 , // Skip / ignore token
    identsym , // Identifier
    numbersym , // Number
    plussym , // +
    minussym , // -
    multsym , // *
    slashsym , // /
    eqsym , // =
    neqsym , // <>
    lessym , // <
    leqsym , // <=
    gtrsym , // >
    geqsym , // >=
    lparentsym , // (
    rparentsym , // )
    commasym , // ,
    semicolonsym , // ;
    periodsym , // .
    becomessym , // :=
    beginsym , // begin
    endsym , // end
    ifsym , // if
    fisym , // fi
    thensym , // then
    whilesym , // while
    dosym , // do
    callsym , // call
    constsym , // const
    varsym , // var
    procsym , // procedure
    writesym , // write
    readsym , // read
    elsesym , // else
    evensym // even
} TokenType;

typedef struct{
    char *lexeme;
    TokenType type;
    int error;
} Lexeme;

Lexeme reservedWords[] = {
    {"begin", beginsym},
    {"end", endsym},
    {"if", ifsym},
    {"fi", fisym},
    {"then", thensym},
    {"while", whilesym},
    {"do", dosym},
    {"call", callsym},
    {"const", constsym},
    {"var", varsym},
    {"procedure", procsym},
    {"write", writesym},
    {"read", readsym},
    {"else", elsesym},
    {"even", evensym}
};

Lexeme specialSymbols[] = {
    {":=", becomessym},
    {">=", geqsym},
    {"<=", leqsym},
    {"<>", neqsym},
    {"+", plussym},
    {"-", minussym},
    {"*", multsym},
    {"/", slashsym},
    {"=", eqsym},
    {"<", lessym},
    {">", gtrsym},
    {"(", lparentsym},
    {")", rparentsym},
    {",", commasym},
    {";", semicolonsym},
    {".", periodsym}
    
};


void printInputText(char *input[]){
    printf("Source Program:\n\n");
    //lexCount--;

    for(int i = 0; i < MAX_TOKEN_SIZE; i++){
        if(!input[i]){
            printf("\n\n");
            return;
        }

        printf("%s", input[i]);        
    }
}

// Frees all memory allocated for each line held in inputFile
void clearInputText(char *input[]){
    for(int i = 0; i < MAX_TOKEN_SIZE; i++){
        if(!input[i])
            return;

        free(input[i]);
    }
}

void printLexemeTable(Lexeme table[]){
    printf("Lexeme Table:\n\n");

    printf("lexeme\ttoken type\n");
    for(int i = 0; i < lexCount; i++){
        printf("%s\t", table[i].lexeme);

        if(table[i].error == 3){
            printf("Identifier too long\n");
        }
        else if(table[i].error == 4){
            printf("Number too long\n");
        }
        else if(table[i].error == 5){
            printf("Invalid symbol\n");
        }
        else{
            printf("%d\n", table[i].type);
        }
    }
    printf("\n\n");

    return;
}

// Frees all memory allocated for each lexeme string 
void freeLexTable(Lexeme table[]){
    for(int i = 0; i < lexCount; i++)
        free(table[i].lexeme);

    return;
}

// Check if token is == 2 (identifier) or == 3 (number) then print value first then identifier/number, for the rest only print value
void printTokenList(Lexeme table[]){ 
    printf("Token List:\n\n");

    for(int i = 0; i < lexCount; i++){
        printf("%d ", table[i].type);

        if(table[i].type == identsym || table[i].type == numbersym)
            printf("%s ", table[i].lexeme);
    }

    return;
}

// [e parameter]: 0 = no error (identifier), 1 = no error (number), 2 = no error (symbols/reserved words), 3 = id too long, 4 = num too long, 5 = invalid symbol
// [type parameter]: 0 = not a reserved word/symbol, !0 = reserved word/symbol
void addLexeme(Lexeme table[], char token[], int e, TokenType type){
    table[lexCount].lexeme = malloc((sizeof(char) * strlen(token)) + 1);
    if(table[lexCount].lexeme == NULL){
        printf("Malloc Error");
        return;
    }

    strcpy(table[lexCount].lexeme, token);

    if(e == 0){
        table[lexCount].type = identsym;
    }
    else if(e == 1){
        table[lexCount].type = numbersym;
    }
    else if(e == 2){
        table[lexCount].type = type;
    }
    else if(e == 3){
        table[lexCount].type = skipsym;
        table[lexCount].error = 3;
    }
    else if(e == 4){
        table[lexCount].type = skipsym;
        table[lexCount].error = 4;
    }
    else if(e == 5){
        table[lexCount].type = type;
        table[lexCount].error = 5;
    }    

    lexCount++;
    return;
}

// Reads and handles number token to process into potential lexemes
void handleNum(char line[], int *lineIndex, Lexeme table[]){
    char storedTokenBuffer[MAX_TOKEN_SIZE + 1] = {'\0'};
    int bufferIndex = 0;

    while(isdigit(line[*lineIndex])){
        storedTokenBuffer[bufferIndex] = line[*lineIndex];

        bufferIndex++;
        (*lineIndex)++;
    }

    // Length Check
    if(bufferIndex > 5)
        addLexeme(table, storedTokenBuffer, 4, 0);
    else if(bufferIndex <= 5)
        addLexeme(table, storedTokenBuffer, 1, 0);  

    return;
}

void handleId(char line[], int *lineIndex, Lexeme table[]){
    char storedTokenBuffer[MAX_TOKEN_SIZE + 1] = {'\0'};
    int bufferIndex = 0;

    while(isalnum(line[*lineIndex])){
        storedTokenBuffer[bufferIndex] = line[*lineIndex];

        bufferIndex++;
        (*lineIndex)++;
    }

    // Reserved word check
    for(int i = 0; i < RESERVED_WORD_COUNT; i++){
        if(strcmp(reservedWords[i].lexeme, storedTokenBuffer) == 0){
            addLexeme(table, storedTokenBuffer, 2, reservedWords[i].type);
            return;
        }
    }
    
    // Length check
    if(bufferIndex > 11)
        addLexeme(table, storedTokenBuffer, 3, 0);
    else if(bufferIndex <= 11)
        addLexeme(table, storedTokenBuffer, 0, 0);  

    return;
}

// 
void handleSym(char line[], int *lineIndex, Lexeme table[]){
    char storedTokenBuffer[3] = {'\0'}; // Max symbol length is 2

    storedTokenBuffer[0] = line[*lineIndex];
    storedTokenBuffer[1] = line[*lineIndex + 1];

    for(int i = 0; i < 4; i++){ // If symbol len == 2
        if(strcmp(storedTokenBuffer, specialSymbols[i].lexeme) == 0){
            addLexeme(table, storedTokenBuffer, 2, specialSymbols[i].type);
            *lineIndex += 2;
            return;
        }
    }

   storedTokenBuffer[1] = '\0';
    for(int i = 0; i < SPECIAL_SYMBOL_COUNT; i++){ // If symbol len == 1
        if(strcmp(storedTokenBuffer, specialSymbols[i].lexeme) == 0){
            addLexeme(table, storedTokenBuffer, 2, specialSymbols[i].type);
            *lineIndex += 1;
            return;
        }
    }
    
    addLexeme(table, storedTokenBuffer, 5, skipsym);
    *lineIndex += 1;
    return;
}

void skipComment(char line[], int *lineIndex){
    *lineIndex += 2; // Checked before function

    while(line[*lineIndex] != '\0'){
        if(line[*lineIndex] == '*'){
            if(line[*lineIndex + 1] == '/'){
                *lineIndex += 2;
                return;
            }
        }

        (*lineIndex)++;
    }

    return;
}

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("Argument Count Error");
        return 1;
    }
    
    FILE *file = fopen(argv[1], "r");
    if(!file){
        printf("File Open Error");
        return 1;
    }

    // Reads from the input text line by line or until max characters read is reached, then allocates space in the input placeholder array and copies it there.
    char *inputFile[MAX_TOKEN_SIZE] = {NULL};
    int line = 0;
    char buff[MAX_TOKEN_SIZE + 1];
    while(fgets(buff, sizeof(buff), file)){
        if(line >= MAX_TOKEN_SIZE){
            printf("Input Overload");
            return 1;
        }

        inputFile[line] = malloc((sizeof(char) * (strlen(buff)) + 1));
        if(inputFile[line] == NULL){
            printf("Malloc Error");
            return 1;
        }

        strcpy(inputFile[line++], buff);
    };
    fclose(file);

    line = 0;
    Lexeme lexTabel[MAX_TOKEN_SIZE];
    // Loops input text and processes the new line
    while(inputFile[line] != NULL){
        int lineIndex = 0; 
        
        while(inputFile[line][lineIndex] != '\0'){ // Loops input text line and proccess the new token
            char current = inputFile[line][lineIndex];

            if(current == ' ' || current == '\t' || current == '\n' || current == '\r'){ // Ignore characters
                lineIndex++;
            }
            else if(current == '/'){ // Comments
                char next = inputFile[line][lineIndex + 1];
                if(next == '*')
                    skipComment(inputFile[line], &lineIndex);
                else{ // False alarm, not a comment
                    char temp[] = {current, '\0'};
                    addLexeme(lexTabel, temp, 2, slashsym);
                    lineIndex++;
                }
            }
            else if(isdigit(current)){ // Handles if the first char of the token is a number
                handleNum(inputFile[line], &lineIndex, lexTabel);
            }
            else if(isalpha(current)){ // Handles if the first char of the token is a letter
                handleId(inputFile[line], &lineIndex, lexTabel);
            }
            else{ // Catch all for symbols, invalid or not, will be processed later
                handleSym(inputFile[line], &lineIndex, lexTabel);
            } 
        }

        line++;
    }

    printInputText(inputFile);
    printLexemeTable(lexTabel);
    printTokenList(lexTabel);

    clearInputText(inputFile);
    freeLexTable(lexTabel);
}