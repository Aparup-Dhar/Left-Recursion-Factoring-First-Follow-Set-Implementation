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

int main() {
    char productions[MAX_PRODUCTIONS][SIZE];
    int num;
    FILE *outputFile;
    FILE *firstFile;
    Production productionStructs[MAX];
    char first[MAX][MAX];
    int num_productions = 0;

    // Initialize first sets
    for (int i = 0; i < MAX; i++) {
        first[i][0] = '\0';
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
    //fprintf(firstFile, "First sets:\n");
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

            printf("\n\nGrammar after removing left recursion:\n");

            // Output new productions without left recursion
            fprintf(outputFile, "%c->", non_terminal);
            printf("%c->", non_terminal);
            for (int j = 0; j < non_recursive_count; j++) {
                fprintf(outputFile, "%s%c", non_recursive_options[j], new_non_terminal);
                printf("%s%c", non_recursive_options[j], new_non_terminal);
                if (j < non_recursive_count - 1) {
                    fprintf(outputFile, "|");
                    printf("|");
                }
            }
            fprintf(outputFile, "\n");
            printf("\n");

            // Output new non-terminal productions
            fprintf(outputFile, "%c->", new_non_terminal);
            printf("%c->", new_non_terminal);
            for (int j = 0; j < recursive_count; j++) {
                fprintf(outputFile, "%s%c|", recursive_options[j], new_non_terminal);
                printf("%s%c|", recursive_options[j], new_non_terminal);
            }
            fprintf(outputFile, "e\n");
            printf("e\n");
        } else {
            // If no left recursion found, print the original production
            printf("\n\nNo left recursion found.\n");
            printf("%c->", non_terminal);
            fprintf(outputFile, "%c->", non_terminal);
            for (int j = 0; j < option_count; j++) {
                printf("%s", options[j]);
                fprintf(outputFile, "%s", options[j]);
                if (j < option_count - 1) {
                    printf("|");
                    fprintf(outputFile, "|");
                }
            }
            printf("\n");
            fprintf(outputFile, "\n");
        }
    }
}

void leftFactor(char productions[MAX_PRODUCTIONS][SIZE], int num, FILE *outputFile) {
    for (int i = 0; i < num; i++) {
        char non_terminal = productions[i][0];
        char* options[MAX_PRODUCTIONS];
        int option_count = 0;

        printf("\n\nGRAMMAR: %s\n", productions[i]);

        // Extract options
        char* token = strtok(productions[i] + 3, "|");
        while (token != NULL) {
            options[option_count++] = token;
            token = strtok(NULL, "|");
        }

        int prefix_found = 0;
        int used[MAX_PRODUCTIONS] = {0};

        while (1) {
            int prefix_length = 0;
            int min_length = SIZE;
            char* common_prefix = NULL;

            // Find the longest common prefix among unused options
            for (int k = 0; k < option_count; k++) {
                if (used[k]) continue;

                for (int j = k + 1; j < option_count; j++) {
                    if (used[j]) continue;

                    int len = 0;
                    while (options[k][len] && options[k][len] == options[j][len]) {
                        len++;
                    }

                    if (len > prefix_length) {
                        prefix_length = len;
                        common_prefix = options[k];
                    }
                }
            }

            if (prefix_length == 0) break;

            prefix_found = 1;
            char new_non_terminal = getNextNonTerminal();

            fprintf(outputFile, "%c->%.*s%c\n", non_terminal, prefix_length, common_prefix, new_non_terminal);
            printf("%c->%.*s%c\n", non_terminal, prefix_length, common_prefix, new_non_terminal);
            fprintf(outputFile, "%c->", new_non_terminal);
            printf("%c->", new_non_terminal);

            int first = 1;
            for (int j = 0; j < option_count; j++) {
                if (used[j]) continue;
                if (strncmp(options[j], common_prefix, prefix_length) == 0) {
                    used[j] = 1;
                    if (!first) {
                        fprintf(outputFile, "|");
                        printf("|");
                    }
                    first = 0;
                    if (strlen(options[j] + prefix_length) > 0) {
                        fprintf(outputFile, "%s", options[j] + prefix_length);
                        printf("%s", options[j] + prefix_length);
                    } else {
                        fprintf(outputFile, "e");  // Replace empty string with 'e'
                        printf("e");
                    }
                }
            }
            fprintf(outputFile, "\n");
            printf("\n");

            // Handle remaining productions
            int remaining = 0;
            for (int j = 0; j < option_count; j++) {
                if (!used[j]) {
                    remaining = 1;
                    break;
                }
            }

            if (!remaining) break;
        }

        if (!prefix_found) {
            fprintf(outputFile, "%c->", non_terminal);
            printf("\n\nNo common prefix found.\n");
            printf("%c->", non_terminal);
            for (int j = 0; j < option_count; j++) {
                printf("%s", options[j]);
                fprintf(outputFile, "%s", options[j]);
                if (j < option_count - 1) {
                    printf("|");
                    fprintf(outputFile, "|");
                }
            }
            printf("\n");
            fprintf(outputFile, "\n");
        }
    }
}

void computeFirst(char non_terminal, Production productions[], int num_productions, char first[][MAX]) {
    for (int i = 0; i < num_productions; i++) {
        if (productions[i].non_terminal == non_terminal) {
            char split_productions[MAX][MAX];
            int count = 0;

            // Split production on '|'
            splitProduction(productions[i].production, split_productions, &count);

            for (int j = 0; j < count; j++) {
                char *production = split_productions[j];
                int length = strlen(production);
                bool epsilon_in_production = false;

                for (int k = 0; k < length; k++) {
                    char symbol = production[k];

                    // Rule 1: X → e
                    if (symbol == EPSILON) {
                        addToFirst(first[non_terminal], EPSILON);
                        epsilon_in_production = true;
                        break;
                    }

                    // Rule 2: X → a (a is a terminal)
                    if ((symbol >= 'a' && symbol <= 'z') || symbol == '(' || symbol == ')' || symbol == ',' || symbol == '$'
                        || symbol == '+'|| symbol == '-'|| symbol == '*'|| symbol == '/') {
                        addToFirst(first[non_terminal], symbol);
                        break;
                    }

                    // Rule 3: X → Y1Y2Y3...
                    if (symbol >= 'A' && symbol <= 'Z') {
                        computeFirst(symbol, productions, num_productions, first);
                        for (int l = 0; first[symbol][l] != '\0'; l++) {
                            if (first[symbol][l] != EPSILON) {
                                addToFirst(first[non_terminal], first[symbol][l]);
                            }
                        }

                        if (containsEpsilon(first[symbol])) {
                            epsilon_in_production = true;
                        } else {
                            epsilon_in_production = false;
                            break;
                        }
                    }
                }

                if (epsilon_in_production) {
                    addToFirst(first[non_terminal], EPSILON);
                }
            }
        }
    }
}

void addToFirst(char *first, char symbol) {
    int length = strlen(first);
    for (int i = 0; i < length; i++) {
        if (first[i] == symbol) {
            return;
        }
    }
    first[length] = symbol;
    first[length + 1] = '\0';
}

bool containsEpsilon(char *first) {
    for (int i = 0; first[i] != '\0'; i++) {
        if (first[i] == EPSILON) {
            return true;
        }
    }
    return false;
}

void splitProduction(char *production, char split_productions[][MAX], int *count) {
    char *token = strtok(production, "|");
    while (token != NULL) {
        strcpy(split_productions[(*count)++], token);
        token = strtok(NULL, "|");
    }
}
