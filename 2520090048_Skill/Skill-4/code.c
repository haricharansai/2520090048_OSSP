#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT 1024
#define INITIAL_TOKENS 16



typedef enum {
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_INPUT,
    TOKEN_OUTPUT
} TokenType;

typedef struct {
    char *value;
    TokenType type;
} Token;




typedef struct ParseNode {
    char *value;
    TokenType type;

    struct ParseNode *left;
    struct ParseNode *right;
} ParseNode;




const char *token_type_name(TokenType type)
{
    switch (type) {

        case TOKEN_WORD:
            return "WORD";

        case TOKEN_PIPE:
            return "PIPE";

        case TOKEN_INPUT:
            return "INPUT";

        case TOKEN_OUTPUT:
            return "OUTPUT";

        default:
            return "UNKNOWN";
    }
}




Token create_token(const char *value, TokenType type)
{
    Token token;

    token.value = malloc(strlen(value) + 1);

    if (token.value == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strcpy(token.value, value);

    token.type = type;

    return token;
}




int tokenize(char *input, Token **tokens)
{
    int capacity = INITIAL_TOKENS;
    int count = 0;

    *tokens = malloc(sizeof(Token) * capacity);

    if (*tokens == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    int i = 0;

    while (input[i] != '\0') {

        /* Ignore whitespace */
        if (isspace((unsigned char)input[i])) {
            i++;
            continue;
        }

        /* Resize token array if necessary */
        if (count >= capacity) {

            capacity *= 2;

            Token *temp =
                realloc(*tokens, sizeof(Token) * capacity);

            if (temp == NULL) {
                perror("realloc");
                free(*tokens);
                exit(EXIT_FAILURE);
            }

            *tokens = temp;
        }

        
        if (input[i] == '|') {

            (*tokens)[count++] =
                create_token("|", TOKEN_PIPE);

            i++;
            continue;
        }

        /* INPUT REDIRECTION */
        if (input[i] == '<') {

            (*tokens)[count++] =
                create_token("<", TOKEN_INPUT);

            i++;
            continue;
        }

        /* OUTPUT REDIRECTION */
        if (input[i] == '>') {

            (*tokens)[count++] =
                create_token(">", TOKEN_OUTPUT);

            i++;
            continue;
        }

        /* WORD */

        char buffer[MAX_INPUT];

        int j = 0;

        while (input[i] != '\0' &&
               !isspace((unsigned char)input[i]) &&
               input[i] != '|' &&
               input[i] != '<' &&
               input[i] != '>') {

            if (j < MAX_INPUT - 1)
                buffer[j++] = input[i];

            i++;
        }

        buffer[j] = '\0';

        if (j > 0) {

            (*tokens)[count++] =
                create_token(buffer, TOKEN_WORD);
        }
    }

    return count;
}


/* ==============================
   DEBUG TOKEN OUTPUT
   ============================== */

void print_tokens(Token *tokens, int count)
{
    printf("\n========== TOKEN STREAM ==========\n");

    if (count == 0) {
        printf("No tokens generated.\n");
        return;
    }

    for (int i = 0; i < count; i++) {

        printf("Token %d:\n", i + 1);
        printf("  Value : %s\n", tokens[i].value);
        printf("  Type  : %s\n",
               token_type_name(tokens[i].type));
    }
}




int validate_tokens(Token *tokens, int count)
{
    if (count == 0) {
        printf("\nEmpty command.\n");
        return 0;
    }

    
    if (tokens[0].type == TOKEN_PIPE) {

        printf("\nSyntax Error: Command cannot start with '|'.\n");
        return 0;
    }

    for (int i = 0; i < count; i++) {

        /* Pipe validation */

        if (tokens[i].type == TOKEN_PIPE) {

            if (i == count - 1) {

                printf("\nSyntax Error: Pipe cannot be at the end.\n");
                return 0;
            }

            if (tokens[i + 1].type == TOKEN_PIPE) {

                printf("\nSyntax Error: Consecutive pipes are not allowed.\n");
                return 0;
            }
        }

        /* Input redirection */

        if (tokens[i].type == TOKEN_INPUT) {

            if (i == count - 1 ||
                tokens[i + 1].type != TOKEN_WORD) {

                printf("\nSyntax Error: '<' requires a filename.\n");
                return 0;
            }
        }

        /* Output redirection */

        if (tokens[i].type == TOKEN_OUTPUT) {

            if (i == count - 1 ||
                tokens[i + 1].type != TOKEN_WORD) {

                printf("\nSyntax Error: '>' requires a filename.\n");
                return 0;
            }
        }
    }

    printf("\nToken stream is syntactically valid.\n");

    return 1;
}


/* ==============================
   CREATE PARSE NODE
   ============================== */

ParseNode *create_node(const char *value, TokenType type)
{
    ParseNode *node =
        malloc(sizeof(ParseNode));

    if (node == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    node->value = malloc(strlen(value) + 1);

    if (node->value == NULL) {
        perror("malloc");
        free(node);
        exit(EXIT_FAILURE);
    }

    strcpy(node->value, value);

    node->type = type;

    node->left = NULL;
    node->right = NULL;

    return node;
}


ParseNode *build_parse_tree(Token *tokens, int count)
{
    ParseNode *root = NULL;

    for (int i = 0; i < count; i++) {

        if (tokens[i].type == TOKEN_PIPE) {

            ParseNode *pipe =
                create_node("|", TOKEN_PIPE);

            pipe->left = root;

            pipe->right =
                create_node(tokens[i + 1].value,
                            tokens[i + 1].type);

            root = pipe;

            i++;
        }

        else if (root == NULL) {

            root =
                create_node(tokens[i].value,
                            tokens[i].type);
        }

        else {

            ParseNode *new_node =
                create_node(tokens[i].value,
                            tokens[i].type);

            ParseNode *current = root;

            while (current->right != NULL)
                current = current->right;

            current->right = new_node;
        }
    }

    return root;
}




void print_tree(ParseNode *root, int level)
{
    if (root == NULL)
        return;

    for (int i = 0; i < level; i++)
        printf("    ");

    printf("|-- %s [%s]\n",
           root->value,
           token_type_name(root->type));

    if (root->left != NULL)
        print_tree(root->left, level + 1);

    if (root->right != NULL)
        print_tree(root->right, level + 1);
}




void free_tree(ParseNode *root)
{
    if (root == NULL)
        return;

    free_tree(root->left);
    free_tree(root->right);

    free(root->value);
    free(root);
}



   

void free_tokens(Token *tokens, int count)
{
    for (int i = 0; i < count; i++)
        free(tokens[i].value);

    free(tokens);
}


/* ==============================
   MAIN
   ============================== */

int main()
{
    char input[MAX_INPUT];

    printf("=====================================\n");
    printf(" Linux Tokenizer and Parser\n");
    printf("=====================================\n");

    printf("Enter a shell-like command.\n");
    printf("Examples:\n");
    printf("  ls -l /home\n");
    printf("  cat file.txt | grep hello\n");
    printf("  cat < input.txt > output.txt\n");
    printf("Type 'exit' to quit.\n\n");

    while (1) {

        printf("parser> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        /* Tokenization */

        Token *tokens = NULL;

        int token_count =
            tokenize(input, &tokens);

        /* Debug token stream */

        print_tokens(tokens, token_count);

        /* Validate */

        if (!validate_tokens(tokens, token_count)) {

            free_tokens(tokens, token_count);

            continue;
        }

        /* Build parse tree */

        ParseNode *tree =
            build_parse_tree(tokens, token_count);

        printf("\n========== PARSE TREE ==========\n");

        print_tree(tree, 0);

        printf("\nExecution structure generated successfully.\n");

        /* Cleanup */

        free_tree(tree);

        free_tokens(tokens, token_count);
    }

    printf("\nParser terminated.\n");

    return 0;
}