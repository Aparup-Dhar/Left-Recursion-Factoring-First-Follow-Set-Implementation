#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SIZE 100
#define MAX_PRODUCTIONS 10
#define MAX 100
#define EPSILON 'e'

// Structure to store productions
typedef struct {
    char non_terminal;
    char production[MAX];
} Production;

// Array of available new non-terminals
char new_non_terminals[] = "XYZUVW";
int used_non_terminals[MAX_PRODUCTIONS] = {0};

// Function prototypes
void computeFirst(char non_terminal, Production productions[], int num_productions, char first[][MAX]);
void addToFirst(char *first, char symbol);
bool containsEpsilon(char *first);
void splitProduction(char *production, char split_productions[][MAX], int *count);
char getNextNonTerminal();
void removeLeftRecursion(char productions[MAX_PRODUCTIONS][SIZE], int num, FILE *outputFile);
void leftFactor(char productions[MAX_PRODUCTIONS][SIZE], int num, FILE *outputFile);
void computeFollow(char non_terminal, Production productions[], int num_productions, char follow[][MAX], char first[][MAX]);

int main() {
    char productions[MAX_PRODUCTIONS][SIZE];
    int num;
    FILE *outputFile;
    FILE *firstFile;
    FILE *followFile;
    Production productionStructs[MAX];
    char first[MAX][MAX];
    char follow[MAX][MAX];
    int num_productions = 0;

    // Initialize first and follow sets
    for (int i = 0; i < MAX; i++) {
        first[i][0] = '\0';
        follow[i][0] = '\0';
    }

    printf("Enter Number of Productions: ");
    scanf("%d", &num);
    getchar(); // Consume the newline character after number input

    printf("Enter the grammar:\n");
    for (int i = 0; i < num; i++) {
        fgets(productions[i], SIZE, stdin);
        productions[i][strcspn(productions[i], "\n")] = '\0'; // Remove newline character
    }

    // Open the file for left recursion output
    outputFile = fopen("lr_output.txt", "w");
    if (outputFile == NULL) {
        printf("Error opening file for writing!\n");
        return 1;
    }

    removeLeftRecursion(productions, num, outputFile);
    fclose(outputFile);

    // Read the output file created by removeLeftRecursion
    outputFile = fopen("lr_output.txt", "r");
    if (outputFile == NULL) {
        printf("Error opening file for reading!\n");
        return 1;
    }

    char new_productions[MAX_PRODUCTIONS][SIZE];
    int new_num = 0;
    while (fgets(new_productions[new_num], SIZE, outputFile) != NULL) {
        new_productions[new_num][strcspn(new_productions[new_num], "\n")] = '\0'; // Remove newline character
        new_num++;
    }
    fclose(outputFile);

    // Open the file for left factoring output
    outputFile = fopen("lf_output.txt", "w");
    if (outputFile == NULL) {
        printf("Error opening file for writing!\n");
        return 1;
    }

    leftFactor(new_productions, new_num, outputFile);
    fclose(outputFile);

    // Read the factored output for computing first sets
    outputFile = fopen("lf_output.txt", "r");
    if (outputFile == NULL) {
        printf("Error opening file for reading!\n");
        return 1;
    }

    while (fscanf(outputFile, " %c->%s", &productionStructs[num_productions].non_terminal, productionStructs[num_productions].production) != EOF) {
        num_productions++;
    }
    fclose(outputFile);

    // Open the file for writing the First sets
    firstFile = fopen("first_sets.txt", "w");
    if (firstFile == NULL) {
        printf("Error opening file for writing!\n");
        return 1;
    }

    // Compute First sets for each non-terminal
    for (int i = 0; i < num_productions; i++) {
        computeFirst(productionStructs[i].non_terminal, productionStructs, num_productions, first);
    }

    // Print and write First sets to the file
    printf("First sets:\n");
    fprintf(firstFile, "First sets:\n");

    // Print terminals
    for (char c = 'a'; c <= 'z'; c++) {
        printf("First(%c) = { %c }\n", c, c);
        fprintf(firstFile, "First(%c) = { %c }\n", c, c);
    }

    // Print non-terminals
    for (int i = 0; i < num_productions; i++) {
        if (first[productionStructs[i].non_terminal][0] != '\0') {  // Avoid printing multiple times for the same non-terminal
            printf("First(%c) = { ", productionStructs[i].non_terminal);
            fprintf(firstFile, "First(%c) = { ", productionStructs[i].non_terminal);
            for (int j = 0; first[productionStructs[i].non_terminal][j] != '\0'; j++) {
                printf("%c ", first[productionStructs[i].non_terminal][j]);
                fprintf(firstFile, "%c ", first[productionStructs[i].non_terminal][j]);
            }
            printf("}\n");
            fprintf(firstFile, "}\n");
        }
    }

    // Close the file for First sets
    fclose(firstFile);

    // Compute Follow sets for each non-terminal
    for (int i = 0; i < num_productions; i++) {
        computeFollow(productionStructs[i].non_terminal, productionStructs, num_productions, follow, first);
    }

    // Open the file for writing the Follow sets
    followFile = fopen("follow_sets.txt", "w");
    if (followFile == NULL) {
        printf("Error opening file for writing!\n");
        return 1;
    }

    // Print and write Follow sets to the file
    printf("Follow sets:\n");
    fprintf(followFile, "Follow sets:\n");
    for (int i = 0; i < num_productions; i++) {
        if (follow[productionStructs[i].non_terminal][0] != '\0') {  // Avoid printing multiple times for the same non-terminal
            printf("Follow(%c) = { ", productionStructs[i].non_terminal);
            fprintf(followFile, "Follow(%c) = { ", productionStructs[i].non_terminal);
            for (int j = 0; follow[productionStructs[i].non_terminal][j] != '\0'; j++) {
                printf("%c ", follow[productionStructs[i].non_terminal][j]);
                fprintf(followFile, "%c ", follow[productionStructs[i].non_terminal][j]);
            }
            printf("}\n");
            fprintf(followFile, "}\n");
        }
    }

    // Close the file for Follow sets
    fclose(followFile);

    return 0;
}

// Function to get the next available non-terminal
char getNextNonTerminal() {
    for (int i = 0; i < strlen(new_non_terminals); i++) {
        if (!used_non_terminals[i]) {
            used_non_terminals[i] = 1;
            return new_non_terminals[i];
        }
    }
    return '\0';  // No available non-terminals
}

void removeLeftRecursion(char productions[MAX_PRODUCTIONS][SIZE], int num, FILE *outputFile) {
    for (int i = 0; i < num; i++) {
        char non_terminal = productions[i][0];
        char* options[MAX_PRODUCTIONS];
        int option_count = 0;

        printf("\n\nGRAMMAR: %s", productions[i]);

        // Extract options
        char* token = strtok(productions[i] + 3, "|");
        while (token != NULL) {
            options[option_count++] = token;
            token = strtok(NULL, "|");
        }

        // Separate options into recursive and non-recursive
        char recursive_options[MAX_PRODUCTIONS][SIZE];
        char non_recursive_options[MAX_PRODUCTIONS][SIZE];
        int recursive_count = 0, non_recursive_count = 0;

        for (int j = 0; j < option_count; j++) {
            if (options[j][0] == non_terminal) {
                strcpy(recursive_options[recursive_count++], options[j] + 1); // Exclude non-terminal
            } else {
                strcpy(non_recursive_options[non_recursive_count++], options[j]);
            }
        }

        if (recursive_count > 0) {
            // Create new non-terminal
            char new_non_terminal = getNextNonTerminal();
            fprintf(outputFile, "%c->", non_terminal);
            for (int j = 0; j < non_recursive_count; j++) {
                fprintf(outputFile, "%s%c|", non_recursive_options[j], new_non_terminal);
            }
            fprintf(outputFile, "\n");
            fprintf(outputFile, "%c->", new_non_terminal);
            for (int j = 0; j < recursive_count; j++) {
                fprintf(outputFile, "%s%c|", recursive_options[j], new_non_terminal);
            }
            fprintf(outputFile, "\n");
        } else {
            fprintf(outputFile, "%s\n", productions[i]);
        }
    }
}

void leftFactor(char productions[MAX_PRODUCTIONS][SIZE], int num, FILE *outputFile) {
    for (int i = 0; i < num; i++) {
        char non_terminal = productions[i][0];
        char* options[MAX_PRODUCTIONS];
        int option_count = 0;

        // Extract options
        char* token = strtok(productions[i] + 3, "|");
        while (token != NULL) {
            options[option_count++] = token;
            token = strtok(NULL, "|");
        }

        // Find common prefixes
        char common_prefix[MAX_PRODUCTIONS];
        memset(common_prefix, 0, sizeof(common_prefix));

        for (int j = 0; j < option_count; j++) {
            if (j == 0) {
                strcpy(common_prefix, options[j]);
            } else {
                int k = 0;
                while (common_prefix[k] != '\0' && common_prefix[k] == options[j][k]) {
                    k++;
                }
                common_prefix[k] = '\0';
            }
        }

        if (strlen(common_prefix) > 0) {
            // Create new non-terminal
            char new_non_terminal = getNextNonTerminal();
            fprintf(outputFile, "%c->%s%c|\n", non_terminal, common_prefix, new_non_terminal);
            fprintf(outputFile, "%c->%c%s\n", new_non_terminal, common_prefix, options[0] + strlen(common_prefix));

            // Write remaining options for the new non-terminal
            for (int j = 1; j < option_count; j++) {
                if (strncmp(common_prefix, options[j], strlen(common_prefix)) == 0) {
                    fprintf(outputFile, "%c%s%c|\n", new_non_terminal, options[j] + strlen(common_prefix), new_non_terminal);
                }
            }
        } else {
            fprintf(outputFile, "%s\n", productions[i]);
        }
    }
}

void computeFirst(char non_terminal, Production productions[], int num_productions, char first[][MAX]) {
    // Initialize the First set for the non-terminal
    char first_set[MAX] = {0};
    addToFirst(first_set, non_terminal);

    // Loop through the productions to compute the First set
    for (int i = 0; i < num_productions; i++) {
        if (productions[i].non_terminal == non_terminal) {
            char production[MAX];
            strcpy(production, productions[i].production);
            char split_productions[MAX_PRODUCTIONS][MAX];
            int count = 0;
            splitProduction(production, split_productions, &count);

            for (int j = 0; j < count; j++) {
                char symbol = split_productions[j][0];
                if (symbol >= 'a' && symbol <= 'z') {
                    addToFirst(first_set, symbol);
                } else {
                    computeFirst(symbol, productions, num_productions, first);
                    for (int k = 0; first[symbol][k] != '\0'; k++) {
                        if (first[symbol][k] != EPSILON) {
                            addToFirst(first_set, first[symbol][k]);
                        }
                    }
                    if (containsEpsilon(first[symbol])) {
                        continue; // Skip adding epsilon
                    } else {
                        break;
                    }
                }
            }
        }
    }

    // Copy the computed First set to the result
    strcpy(first[non_terminal], first_set);
}

void addToFirst(char *first, char symbol) {
    if (strchr(first, symbol) == NULL) {
        int len = strlen(first);
        first[len] = symbol;
        first[len + 1] = '\0';
    }
}

bool containsEpsilon(char *first) {
    return strchr(first, EPSILON) != NULL;
}

void splitProduction(char *production, char split_productions[][MAX], int *count) {
    char *token = strtok(production, "|");
    while (token != NULL) {
        strcpy(split_productions[(*count)++], token);
        token = strtok(NULL, "|");
    }
}

void computeFollow(char non_terminal, Production productions[], int num_productions, char follow[][MAX], char first[][MAX]) {
    char follow_set[MAX] = {0};
    if (non_terminal == productions[0].non_terminal) {
        addToFirst(follow_set, '$');
    }

    for (int i = 0; i < num_productions; i++) {
        char *prod = productions[i].production;
        for (int j = 0; prod[j] != '\0'; j++) {
            if (prod[j] == non_terminal) {
                if (prod[j + 1] != '\0') {
                    char next_symbol = prod[j + 1];
                    if (next_symbol >= 'a' && next_symbol <= 'z') {
                        addToFirst(follow_set, next_symbol);
                    } else {
                        for (int k = 0; first[next_symbol][k] != '\0'; k++) {
                            if (first[next_symbol][k] != EPSILON) {
                                addToFirst(follow_set, first[next_symbol][k]);
                            }
                        }
                        if (containsEpsilon(first[next_symbol])) {
                            if (next_symbol != non_terminal) {
                                computeFollow(next_symbol, productions, num_productions, follow, first);
                                for (int l = 0; follow[next_symbol][l] != '\0'; l++) {
                                    addToFirst(follow_set, follow[next_symbol][l]);
                                }
                            }
                        }
                    }
                } else {
                    if (prod[j] != non_terminal) {
                        computeFollow(productions[i].non_terminal, productions, num_productions, follow, first);
                        for (int l = 0; follow[productions[i].non_terminal][l] != '\0'; l++) {
                            addToFirst(follow_set, follow[productions[i].non_terminal][l]);
                        }
                    }
                }
            }
        }
    }

    // Copy the computed Follow set to the result
    strcpy(follow[non_terminal], follow_set);
}
