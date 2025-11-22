#include <stdio.h>
#include <stdlib.h>

typedef enum { PASS, NOT, AND, NAND, NOR, OR, XOR, DECODER, MULTIPLEXER } gate_type;

struct var {
    int val;
    int index;
    char *name;
};

struct varTable {
    struct var *vars;
    int EOI;
    int EOO;
    int endTemp;
};

struct gate {
    int **masterIn;
    int **masterOut;
    struct gate *next;
    int NumOfIn;
    int NumOfOut;
    gate_type type;
};

int compareString(char *ic1, char *ic2) {
    char char1, char2;
    while (1) {
        char1 = *ic1++;
        char2 = *ic2++;
        if (char1 != char2) {
            return char1 - char2;
        }
        if (char1 == '\0') {
            return char1 - char2;
        }
    }
}

void copyString(char *s1, char *s2) {
    while ((*s2 = *s1) != '\0') {
        s2++;
        s1++;
    }
}

void freeTable(struct varTable *table) {
    for (int i = 0; i < table->endTemp; ++i) {
        free(table->vars[i].name);
    }
    free(table->vars);
    free(table);
}

void freeGates(struct gate *list) {
    struct gate *temp;
    while (list) {
        temp = list;
        list = list->next;
        free(temp->masterIn);
        free(temp->masterOut);
        free(temp);
    }
}

void printTable(struct varTable *table) {
    int i = 0;
    for (; i < table->EOI; ++i) {
        printf("%d ", table->vars[i].val);
    }
    printf("| ");
    for (; i < table->EOO; ++i) {
        if (i + 1 == table->EOO) {
            printf("%d", table->vars[i].val);
        } else {
            printf("%d ", table->vars[i].val);
        }
    }
    printf("\n");
}

void readInVar(struct varTable *table, FILE *abc) {
    char IOBUF[7];
    int i, NOI, NOO;
    fscanf(abc, "%s", IOBUF);
    fscanf(abc, "%d", &NOI);
    table->EOI = NOI;
    table->vars = malloc(sizeof(struct var) * NOI);
    for (i = 0; i < NOI; ++i) {
        table->vars[i].name = malloc(sizeof(char) * 17);
        fscanf(abc, "%16s", table->vars[i].name);
        table->vars[i].index = i;
        table->vars[i].val = 0;
    }
    fscanf(abc, "%s", IOBUF);
    fscanf(abc, "%d", &NOO);
    table->vars = realloc(table->vars, sizeof(struct var) * (NOO + table->EOI));
    table->EOO = NOO + table->EOI;
    for (; i < table->EOO; ++i) {
        table->vars[i].name = malloc(sizeof(char) * 17);
        fscanf(abc, "%16s", table->vars[i].name);
        table->vars[i].index = i;
        table->vars[i].val = 0;
    }
}

void tempSearch(struct varTable *table, FILE *abc) {
    char BUFFER[17];
    int NOI, NOO, i, j;
    table->endTemp = table->EOO;
    while (fscanf(abc, "%16s", BUFFER) != EOF) {
        if ((BUFFER[0] == 'N' && BUFFER[2] == 'T') || BUFFER[0] == 'P') {
            NOI = 1;
            NOO = 1;
        } else if (BUFFER[0] == 'D' || BUFFER[0] == 'M') {
            fscanf(abc, "%d", &NOI);
            if (BUFFER[0] == 'D') {
                NOO = 1 << NOI;
            } else {
                NOO = 1;
                NOI += 1 << NOI;
            }
        } else {
            NOI = 2;
            NOO = 1;
        }
        for (i = 0; i < NOI + NOO; ++i) {
            fscanf(abc, "%16s", BUFFER);
            if (BUFFER[0] != '1' && BUFFER[0] != '0' && BUFFER[0] != '_') {
                for (j = 0; j < table->endTemp; ++j) {
                    if (!compareString(BUFFER, table->vars[j].name)) {
                        break;
                    }
                }
                if (j == table->endTemp) {
                    table->vars = realloc(table->vars, sizeof(struct var) * (1 + table->endTemp));
                    table->vars[table->endTemp].name = malloc(sizeof(BUFFER));
                    copyString(BUFFER, table->vars[table->endTemp].name);
                    table->vars[table->endTemp].index = table->endTemp;
                    table->vars[table->endTemp].val = 0;
                    table->endTemp++;
                }
            }
        }
    }
}

void createGates(struct gate **First, struct varTable *Table, int *binary, FILE *fp)
{
 struct gate *Newgate, **Indirect = First;
 char BUFFER[17], SKIP[8192];
 gate_type type;
 int inputs, i, j;
 fgets(SKIP, 8192, fp);
 fgets(SKIP, 8192, fp);
 while (fscanf(fp, "%16s", BUFFER) != EOF) {
     // Find what type of gate it is and assign the type
     switch (BUFFER[0]) {
     case 'D':
         type = DECODER;
         break;
     case 'M':
         type = MULTIPLEXER;
         break;
     case 'N':
         if (BUFFER[2] == 'T')
             type = NOT;
         else if (BUFFER[2] == 'N')
             type = NAND;
         else
             type = NOR;
         break;
     case 'P':
         type = PASS;
         break;
     case 'O':
         type = OR;
         break;
     case 'A':
         type = AND;
         break;
     case 'X':
         type = XOR;
         break;
     }
     *Indirect = malloc(sizeof(struct gate));
     Newgate = *Indirect;
     Newgate->next = NULL;
     Newgate->type = type;
     if (type < 2) {
         Newgate->NumOfIn = 1;
         Newgate->NumOfOut = 1;
     } else if (type < 7) {
         Newgate->NumOfIn = 2;
         Newgate->NumOfOut = 1;
     } else {
         fscanf(fp, "%d", &inputs);


         if (type == DECODER) {
             Newgate->NumOfIn = inputs;
             Newgate->NumOfOut = 1 << inputs;
         } else {
             Newgate->NumOfIn = inputs + (1 << inputs);
             Newgate->NumOfOut = 1;
         }
     }
     Newgate->masterIn = malloc(Newgate->NumOfIn * sizeof(int *));
     for (i = 0; i < Newgate->NumOfIn; ++i) {
         fscanf(fp, "%16s", BUFFER);
         if (BUFFER[0] == '0') {
             Newgate->masterIn[i] = &(binary[0]);
         } else if (BUFFER[0] == '1') {
             Newgate->masterIn[i] = &(binary[1]);
         } else if (BUFFER[0] == '_') {
             Newgate->masterIn[i] = &(binary[2]);
         } else {
             for (j = 0; j < Table->endTemp; ++j) {
                 if (!compareString(BUFFER, Table->vars[j].name)) {
                     Newgate->masterIn[i] = &(Table->vars[j].val);
                     break;
                 }
             }
         }
     }
     Newgate->masterOut = malloc(Newgate->NumOfOut * sizeof(int *));
     for (i = 0; i < Newgate->NumOfOut; ++i) {
         fscanf(fp, "%16s", BUFFER);


         if (BUFFER[0] == '0') {
             Newgate->masterOut[i] = &(binary[0]);
         } else if (BUFFER[0] == '1') {
             Newgate->masterOut[i] = &(binary[1]);
         } else if (BUFFER[0] == '_') {
             Newgate->masterOut[i] = &(binary[2]);
         } else {
             for (j = 0; j < Table->endTemp; ++j) {
                 if (!compareString(BUFFER, Table->vars[j].name)) {
                     Newgate->masterOut[i] = &(Table->vars[j].val);
                     break;
                 }
             }
         }
     }
     if (type == MULTIPLEXER)
     Newgate->NumOfIn -= 1 << inputs;
     Indirect = &(Newgate->next);
 }
}

void Solve_Decoder(struct gate *Decoder) {
    int i, incrementer, bit;
    incrementer = 1;
    bit = 0;
    for (i = 0; i < Decoder->NumOfIn; ++i) {
        if (*Decoder->masterIn[i] == 1)
            bit += 1 << (Decoder->NumOfIn - incrementer);
        incrementer++;
    }
    if (*Decoder->masterOut[bit] != -1)
        *Decoder->masterOut[bit] = 1;
}

void Solve_Multiplexer(struct gate *Plex) {
    int i, selectorindex, totalinputs, row, incrementer;
    selectorindex = 1 << Plex->NumOfIn;
    totalinputs = selectorindex + Plex->NumOfIn;
    row = 0;
    incrementer = 1;
    for (i = selectorindex; i < totalinputs; ++i) {
        if (*Plex->masterIn[i] == 1)
            row += 1 << (Plex->NumOfIn - incrementer);
        ++incrementer;
    }
    if (*Plex->masterOut[0] != -1)
        *Plex->masterOut[0] = *Plex->masterIn[row];
}

void DoCircuit(struct gate *First, struct varTable *Table) {
    while (First != NULL) {
        switch (First->type) {
        case PASS:
            if (*First->masterOut[0] != -1)
                *First->masterOut[0] = *First->masterIn[0];
            break;
        case NOT:
            if (*First->masterOut[0] != -1)
                *First->masterOut[0] = (*First->masterIn[0] == 1) ? 0 : 1;
            break;
        case AND:
            if (*First->masterIn[0] == 1 && *First->masterIn[1] == 1) {
                *First->masterOut[0] = 1;
            } else {
                if (*First->masterOut[0] != -1)
                    *First->masterOut[0] = 0;
            }
            break;
        case NAND:
            if (*First->masterIn[0] == 1 && *First->masterIn[1] == 1) {
                *First->masterOut[0] = 0;
            } else {
                if (*First->masterOut[0] != -1)
                    *First->masterOut[0] = 1;
            }
            break;
        case NOR:
            if (*First->masterIn[0] == 1 || *First->masterIn[1] == 1) {
                *First->masterOut[0] = 0;
            } else {
                if (*First->masterOut[0] != -1)
                    *First->masterOut[0] = 1;
            }
            break;
        case OR:
            if (*First->masterIn[0] == 1 || *First->masterIn[1] == 1) {
                *First->masterOut[0] = 1;
            } else {
                if (*First->masterOut[0] != -1)
                    *First->masterOut[0] = 0;
            }
            break;
        case XOR:
            if ((*First->masterIn[0] == 1 && *First->masterIn[1] == 0) ||
                (*First->masterIn[0] == 0 && *First->masterIn[1] == 1)) {
                *First->masterOut[0] = 1;
            } else {
                if (*First->masterOut[0] != -1)
                    *First->masterOut[0] = 0;
            }
            break;
        case DECODER:
            Solve_Decoder(First);
            break;
        case MULTIPLEXER:
            Solve_Multiplexer(First);
            break;
        }
        First = First->next;
    }
    printTable(Table);
}
void sortGates(struct gate **First, struct varTable *Table)
{
    int i, j, NumOfIn;
    int *TempAddr, *TableAddr;
    struct gate **swap;

    swapped:
    while (*First != NULL) {
        if ((*First)->type == MULTIPLEXER)
            NumOfIn = (*First)->NumOfIn + (1 << (*First)->NumOfIn);
        else
            NumOfIn = (*First)->NumOfIn;

        for (i = 0; i < NumOfIn; ++i) {
            TempAddr = (*First)->masterIn[i];

            for (j = Table->EOO; j < Table->endTemp; ++j) {
                TableAddr = &(Table->vars[j].val);

                if (TempAddr == TableAddr) {
                    int k, found = 0;
                    Table->vars[j].val = 1;
                    swap = &((*First)->next);

                    while (*swap != NULL) {
                        for (k = 0; k < ((*swap)->NumOfOut); ++k) {
                            if (*(*swap)->masterOut[k] == 1) {
                                found = 1;
                                break;
                            }
                        }
                        if (found) {
                            struct gate *tmp = (*swap)->next;
                            (*swap)->next = *First;
                            *First = *swap;
                            *swap = tmp;
                            break;
                        } else {
                            swap = &((*swap)->next);
                        }
                    }
                    Table->vars[j].val = 0;

                    if (found)
                        goto swapped;
                }
            }
        }
        First = &((*First)->next);
    }
}

void solveTable(struct gate *First, struct varTable *Table)
{
    int i, start;
    start = (Table->EOI - 1);
    DoCircuit(First, Table);

    for (i = start; i >= 0; --i) {
        if (Table->vars[i].val == 0) {
            Table->vars[i].val = 1;
            i = start + 1;
        } else if (Table->vars[i].val == 1) {
            Table->vars[i].val = 0;
        }

        if (i == start + 1) {
            int j;
            for (j = Table->EOI; j < Table->endTemp; ++j)
                Table->vars[j].val = 0;
            DoCircuit(First, Table);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        puts("Error: Missing Inputs. Usage: ./truthtable [pathtocircuit]");
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        printf("error opening file");
        return 1;
    }

    struct varTable *Table = malloc(sizeof(*Table));
    readInVar(Table, fp);
    tempSearch(Table, fp);

    struct gate *First = NULL;
    int binary[] = {0, 1, -1};
    rewind(fp);
    createGates(&First, Table, binary, fp);
    fclose(fp);

    sortGates(&First, Table);
    solveTable(First, Table);

    freeTable(Table);
    freeGates(First);
    return 0;
}
