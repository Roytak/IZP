/*
*       Title: IZP Projekt 2 - Prace s datovymi strukturami
*       Authors: Roman Janota, Tomas Valik, Danil Domrachev
*       Logins: xjanot04, xvalik04, xdomra00
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//#define NDEBUG
#define MAX_LINES_COUNT 1000
#define MAX_SET_ELEM_LEN 30
#define ENTITIES_COUNT 4
#define clearPIF clearEntities2D((void**)setArr, MAX_LINES_COUNT, S); \
                 clearEntities2D((void**)relArr, MAX_LINES_COUNT, R);
#ifndef NDEBUG
    #define errMsg(format, ...) fprintf(stderr, "Error in file\n%s" \
                                                "\non line %u\n"    \
                                                "Error message:\n" format, __FILE__, __LINE__, __VA_ARGS__)
#else
    #define errMsg(format, ...) (void)
#endif

typedef unsigned long long ullong;


//U - universe S - set R - relation C - command,
typedef enum {
    U = 0,
    S = 1,
    R = 2,
    C = 3
} entities;

typedef enum {
    EMPTY_S=0, CARD_S=1, COMPL_S=2, UNION_S=3, INTERS_S=4, MINUS_S=5, SUBSTEQ_S=6, SUBST_S=7, EQUAL_S=8,
    REFLEX_R=9, SYM_R=10, ANTISYM_R=11, TRANS_R=12, FUNC_R=13, DOMAIN_R=14, CODOMAIN_R=15, INJECT_R=16, SURJECT_R=17, BIJECT_R=18
} command_id;

typedef struct {
    char **elements;
    ullong id;
    ullong cardinality;
} set;

typedef struct {
    set **elements;
    ullong id;
    ullong cardinality;
    ullong dim;//set array(set **elements) elements(set) cardinality
} relation;

typedef struct {
    int argsCount;
    char **arguments;//command name, arg1, arg2...
} command;

//parsing and help funcitons

void clear2D(void **arr, ullong arrSize);

void clearEntities2D(void **ents, ullong arrSize, entities type);

bool initSet(set **s, char **elements, ullong arrSize, bool dropIdCount);

bool isInStrArr(char *elem, char **arr, ullong arrSize);

void destroySet(set *s);

bool makeUniqueSet(set *sin, set **sout);

bool initRelation(relation **r, set **elements, ullong arrSize, bool dropIdCount);

void destroyRelation(relation *r);

bool initCommand(command **c, char **arguments, int arrSize);

void destroyCommand(command *c);

int findEntity(void **data, ullong arrSize, entities type, ullong id, void **found);

bool printEntity(void *entity, entities type);

bool executeCommand(command *c, set **sets, relation **rels, ullong sizes[]);

bool parseEntity(FILE *file, entities object, void **entity);

bool parseCommandEntity(FILE *file, command **entity);

bool parseRelationEntity(FILE *file, relation **entity);

bool parseSetEntity(FILE *file, set **entity);

bool parseInputFile(FILE *inputFile);

//sets commands

void onSetCommandExit(char** newSetElements, ullong newSetCardinality);

char **allocateNewElements(ullong size);

bool emptyCommand(set* setPtr);

bool cardCommand(set* setPtr);

bool complementCommand(set* setPtr, set* uniPtr);

bool unionCommand(set* setA, set* setB);

int intersectCommand (set* setA, set* setB, set** resultSet, bool print);

bool minusCommand(set* setA, set* setB);

int subseteqCommand(set* setA, set* setB, bool print);

bool subsetCommand(set* setA, set* setB);

bool equalsCommand(set* setA, set* setB);

//relation commands

bool functionCommand(relation *r);

bool transitiveCommand(relation *r);

bool reflexiveCommand(relation *r, set *s);

bool symmetricCommand(relation *r);

bool antisymmetricCommand(relation *r);

bool domainCommand(relation *r);

bool codomainCommand(relation *r);

bool injectiveCommand(relation *r, set *a, set *b);

bool surjectiveCommand(relation *r, set *a, set *b);

bool bijectiveCommand(relation *r, set *a, set *b);

bool helpSurjectiveCommand(relation *r, set *a, set *b);

bool helpInjectiveCommand(relation *r, set *a, set *b);

bool helpFunctionComand(relation *r);


int main(int argc, char *argv[]) {
    if(argc != 2) {
        errMsg("%s", "Wrong argument count\n Use: ./setcal FILENAME");
        return 1;
    }
    char *filename = argv[1];
    FILE *inputFile = fopen(filename, "r");
    if(inputFile == NULL) {
        errMsg("%s", "Cannot open file\n");
        return 2;
    }

    if(!parseInputFile(inputFile)) {
        errMsg("%s", "Cannot parse file\n");
        fclose(inputFile);
        return 3;
    }   

    if(fclose(inputFile) == EOF) {
        errMsg("%s", "Cannot close file\n");
        return 4;
    }

    return 0;
}


//function description convention
/*
Description:

Input:

Output:
*/


/*
Description:
    frees 2d dynamic array

Input:
    arr - array to free of emory

Output:
    no
*/
void clear2D(void **arr, ullong arrSize) {
    if(arr == NULL) {
        return;
    }
    for(ullong i = 0; i < arrSize; i++) {
        free(arr[i]);
    }
    free(arr);
}



/*
Description:
    frees 2d dynamic array of entities

Input:
    ents - entities array to free
    arrSize - array size
    type - type of entity

Output:
    no
*/
void clearEntities2D(void **ents, ullong arrSize, entities type) {
    if(ents == NULL) {
        return;
    }
    for(ullong i = 0; i < arrSize; i++) {
        switch(type) {
            case(U):
            case(S): {
                if(((set**)ents)[i] != NULL) {
                    destroySet(((set**)ents)[i]);
                }
                break;
            }
            case(R): {
                if((relation**)ents[i] != NULL) {
                    destroyRelation(((relation**)ents)[i]);
                }
                break;
            }
            case(C): {
                if((command**)ents[i] != NULL) {
                    destroyCommand(((command**)ents)[i]);
                }
                break;
            }
            default: { return; }
        }
    }
    free(ents);
}


/*
Description:
    Initialising set entity

Input:
    s - output pointer
    elements - set elements
    arrSize - cardinality
    dropIdCount - begin auto indexing with 0

Output:
    true if all is ok
    false if some error
*/
bool initSet(set **s, char **elements, ullong arrSize, bool dropIdCount)
{
    (*s) = malloc(sizeof(set));
    if((*s) == NULL) {
        errMsg("%s", "No memory for initialising set\n");
        return false;
    }

    static ullong id = 0;
    if(dropIdCount) {
        id = 0;
    }
    (*s)->id = id;
    id++;
    (*s)->cardinality = arrSize;

    if(elements == NULL) {
        (*s)->elements = NULL;
        return true;
    }

    (*s)->elements = malloc(arrSize*sizeof(char*));
    if((*s)->elements == NULL) {
        errMsg("%s", "No memory for initialising set elements\n");
        destroySet(*s);
        return false;
    }

    for(ullong i = 0; i < arrSize; i++) {
        (*s)->elements[i] = malloc((strlen(elements[i])+1)*sizeof(char));
        if((*s)->elements[i] == NULL) {
            errMsg("%s", "No memory for initialising set element");
            destroySet(*s);
            return false;
        }
        strcpy((*s)->elements[i],elements[i]);
    }
    return true;
}


/*
Description:
    Checks if an element is in set

Input:
    elem - lement to find
    s - set where search to

Output:
    true if there is an lement in set
    false if not
*/
bool isInStrArr(char *elem, char **arr, ullong arrSize) {
    for(ullong i = 0; i < arrSize; i++) {
        if(strcmp(elem, arr[i]) == 0) {
            return true;
        }
    }
    return false;
}

/*
Description:
    frees set allocated memory

Input:
    s - set to free

Output:
    no
*/
void destroySet(set *s) {
    if(s == NULL) {
        return;
    }
    clear2D((void**)s->elements, s->cardinality);
    free(s);
}


/*
Description:
    removes repeating elements from set

Input:
    sin - set to check
    sout - set to create, can be NULL

Output:
    true if set was unique before
    false if set was not unique before or some error
*/
bool makeUniqueSet(set *sin, set **sout) {
    if(sin->cardinality == 0) {
        return true;
    }
    bool isUnique = true;
    ullong newSetCardinality = 0;
    ullong repeatingArrSize = 0;
    char **uniqueElements;
    char **repeatingElements;
    uniqueElements = malloc(sin->cardinality*sizeof(char*));
    repeatingElements = malloc(sin->cardinality*sizeof(char*));
    if(uniqueElements == NULL || repeatingElements == NULL) {
        errMsg("%s", "Not enough memory\n");
        return false;
    }
    for(ullong i = 0; i < sin->cardinality; i++) {
        uniqueElements[i] = NULL;
        repeatingElements[i] = NULL;
    }

    for(ullong i = 0; i < sin->cardinality; i++) {
        bool addUnique = true;
        for(ullong j = 0; j < sin->cardinality; j++) {
            if(strcmp(sin->elements[i], sin->elements[j]) == 0 && i != j) {
                isUnique = false;
                addUnique = false;
                if(false == isInStrArr(sin->elements[i], repeatingElements, repeatingArrSize)) {
                    repeatingElements[repeatingArrSize] = malloc((MAX_SET_ELEM_LEN+1)*sizeof(char));
                    strcpy(repeatingElements[repeatingArrSize], sin->elements[i]);
                    repeatingArrSize++;
                }
                break;
            }
        }
        if(addUnique) {
            uniqueElements[newSetCardinality] = malloc((MAX_SET_ELEM_LEN+1)*sizeof(char));
            strcpy(uniqueElements[newSetCardinality],sin->elements[i]);
            newSetCardinality++;
        }
    }

    for(ullong i = 0; i < repeatingArrSize; i++) {
        uniqueElements[i+newSetCardinality] = malloc((MAX_SET_ELEM_LEN+1)*sizeof(char));
        strcpy(uniqueElements[i+newSetCardinality], repeatingElements[i]);
    }
    newSetCardinality += repeatingArrSize;

    if(sout != NULL) {
        initSet(sout, uniqueElements, newSetCardinality, true);
    }
    clear2D((void**)uniqueElements, sin->cardinality);
    clear2D((void**)repeatingElements, sin->cardinality);
    return isUnique;
}


/*
Description:
    removes repeating elements from relation

Input:
    rin - relation to check
    rout - relation to create, can be NULL

Output:
    true if relation was unique before
    false if relation was not unique before or some error
*/
bool makeUniqueRelation(relation *rin, relation **rout)
{
    if(rin->cardinality == 0) { return true; }
    bool isUnique = true;
    ullong newRelationCardinality = 0;
    ullong repeatingArrSize = 0;
    set **uniqueElements;
    set **repeatingElements;
    uniqueElements = malloc(rin->cardinality*sizeof(set*));
    repeatingElements = malloc(rin->cardinality*sizeof(set*));
    if(uniqueElements == NULL || repeatingElements == NULL) {
        errMsg("%s", "Not enough memory\n");
        return false;
    }
    for(ullong i = 0; i < rin->cardinality; i++) {
        uniqueElements[i] = NULL;
        repeatingElements[i] = NULL;
    }

    for(ullong i = 0; i < rin->cardinality; i++) {
        bool addUnique = true;
        for(ullong j = 0; j < rin->cardinality; j++) {
            if( i != j &&
                0 == strcmp(rin->elements[i]->elements[0],rin->elements[j]->elements[0]) &&
                0 ==  strcmp(rin->elements[i]->elements[1],rin->elements[j]->elements[1])) {
                isUnique = false;
                addUnique = false;
                bool isInArr = false;
                for(ullong k = 0; k < repeatingArrSize; k++) {
                    if( 0 == strcmp(rin->elements[i]->elements[0], repeatingElements[k]->elements[0]) &&
                        0 == strcmp(rin->elements[i]->elements[1], repeatingElements[k]->elements[1])) 
                        {
                            isInArr = true;
                            break;
                        }
                }
                if(!isInArr) {
                    //repeatingElements[repeatingArrSize] = malloc(sizeof(set));
                    initSet(&repeatingElements[repeatingArrSize], 
                            rin->elements[i]->elements, 
                            rin->elements[i]->cardinality, false);

                    repeatingArrSize++;
                }
                break;
            }
        }
        if(addUnique) {
            //uniqueElements[newRelationCardinality] = malloc((MAX_SET_ELEM_LEN+1)*sizeof(char));
            initSet(&uniqueElements[newRelationCardinality] , 
                    rin->elements[i]->elements, 
                    rin->elements[i]->cardinality, false);
            newRelationCardinality++;
        }
    }

    for(ullong i = 0; i < repeatingArrSize; i++) {
        // uniqueElements[i+newRelationCardinality] = malloc((MAX_SET_ELEM_LEN+1)*sizeof(char));
        // strcpy(uniqueElements[i+newRelationCardinality], repeatingElements[i]);
        initSet(&uniqueElements[i+newRelationCardinality],   
                repeatingElements[i]->elements, repeatingElements[i]->cardinality, true);

    }
    newRelationCardinality += repeatingArrSize;

    if(rout != NULL) {
        initRelation(rout, uniqueElements, newRelationCardinality, true);
    }

    clearEntities2D((void**)repeatingElements, rin->cardinality, S);
    clearEntities2D((void**)uniqueElements, rin->cardinality, S);

    return isUnique;
}


/*
Description:
    Initialising relation entity

Input:
    r - output pointer
    elements - relation elements
    arrSize - cardinality
    dropIdCount - begin auto indexing with 0

Output:
    true if all is ok
    false if some error
*/
bool initRelation(relation **r, set **elements, ullong arrSize, bool dropIdCount)
{
    (*r) = malloc(sizeof(relation));
    if((*r) == NULL) {
        errMsg("%s", "No memory for initialising relation\n");
        return false;
    }

    static ullong id = 0;
    if(dropIdCount) {
        id = 0;
    }
    (*r)->id = id;
    id++;
    (*r)->cardinality = arrSize;

    if(elements == NULL) {
        (*r)->dim = 0;
        (*r)->elements = NULL;
        return true;
    }
    (*r)->dim = 2;

    (*r)->elements = malloc(arrSize*sizeof(set*));
    if((*r)->elements == NULL) {
        errMsg("%s", "No memory for initialising relation elements\n");
        destroyRelation(*r);
        return false;
    }
    for(ullong i = 0; i < arrSize; i++) {
        (*r)->elements[i] = NULL;
    }


    for(ullong i = 0; i < arrSize; i++) {
        if((*r)->dim != elements[i]->cardinality) {
            errMsg("%s", "Given sets are not of the 2 dimension\n");
            destroyRelation(*r);
            return false;
        }
        if(false == initSet(&((*r)->elements[i]), elements[i]->elements, elements[i]->cardinality, true)) {
            errMsg("%s", "No memory for initialising relation element\n");
            destroyRelation(*r);
            return false;
        }
    }
    return true;
}


/*
Description:
    frees relation allocated memory

Input:
    r - relation to free

Output:
    no
*/
void destroyRelation(relation *r) {
    if(r == NULL) {
        return;
    }
    for(ullong i = 0; i < r->cardinality; i++) {
        destroySet(r->elements[i]);
    }
    free(r->elements);
    free(r);
}



/*
Description:
    Initialising command entity

Input:
    c - output pointer
    arguments - command arguments
    arrSize - arguments count

Output:
    true if all is ok
    false if some error
*/
bool initCommand(command **c, char **arguments, int arrSize) {

    (*c) = malloc(sizeof(command));
    if((*c) == NULL) {
        errMsg("%s", "No memory for initialising command\n");
        return false;
    }

    (*c)->arguments = malloc(arrSize * sizeof(char*));
    if((*c)->arguments == NULL) {
        errMsg("%s", "No memory for initialising command elements\n");
        destroyCommand(*c);
        return false;
    }

    for(int i = 0; i < arrSize; i++) {
        (*c)->arguments[i] = malloc((strlen(arguments[i])+1)*sizeof(char));
        if((*c)->arguments[i] == NULL) {
            errMsg("%s", "No memory for initialising command argument\n");
            destroyCommand(*c);
            return false;
        }
        strcpy((*c)->arguments[i], arguments[i]);
    }

    (*c)->argsCount = arrSize;

    return true;
}


/*
Description:
    frees command allocated memory

Input:
    c - command to free

Output:
    no
*/
void destroyCommand(command *c) {
    if(c == NULL) {
        return;
    }
    clear2D((void**)c->arguments, c->argsCount);
    free(c);
}


/*
Description:
    finds entity Id
    not working for entities, that dont have id
Input:
    data - data where find from
    arrSize - data size
    type - entity to find
    id - id to compare with
    found - output pointer

Output:
    negative number if error
    0 if not found
    positive number - id of found entity
*/
int findEntity(void **data, ullong arrSize, entities type, ullong id, void **found) {
    for(ullong i = 0; i < arrSize; i++) {
        switch(type) {
            case(U):
            case(S): {
                set *s = ((set*)((set**)data)[i]);
                if(s->id == id) {
                    *found = (void*)s;
                    return ((set*)*found)->id;
                }
                break;
            }
            case(R): {
                relation *r = ((relation*)((relation**)data)[i]);
                if(r->id == id) {
                    *found = (void*)r;
                    return ((relation*)*found)->id;
                }
                break;
            }
            default: {
                *found = NULL;
                return -1;
            }
        }
    }
    *found = NULL;
    return -1;
}



/*
Description:
    prints entity
Input:
    c - command
    sets - sets array
    rels - relations array

Output:
    true if all is ok
    false if some error
*/
bool printEntity(void *entity, entities type) {

    switch(type) {
        case(U):
        case(S): {
            if(((set*)entity)->cardinality == 0) {
                if (type == U) {
                    printf("U\n");
                }
                else {
                    printf("S\n");
                }
                return true;
            }
            if(type == U) {
                printf("U ");
            }
            else {
                printf("S ");
            }
            for(ullong i = 0; i < ((set*)entity)->cardinality; i++) {
                printf("%s", ((set*)entity)->elements[i]);
                if(i != ((set*)entity)->cardinality - 1) {
                    putchar(' ');
                }
            }
            putchar('\n');
            break;
        }
        case(R): {
             if(((relation*)entity)->cardinality == 0) {
                printf("R\n");
                return true;
            }
            printf("R ");
            for(ullong i = 0; i < ((relation*)entity)->cardinality; i++) {
                putchar('(');
                for(ullong j = 0; j < ((relation*)entity)->dim; j++) {
                    printf("%s", ((relation*)entity)->elements[i]->elements[j]);
                    if(j != ((relation*)entity)->dim-1) {
                        putchar(' ');
                    }
                }
                putchar(')');
                if(i != ((relation*)entity)->cardinality-1) {
                    putchar(' ');
                }
            }
            putchar('\n');
            break;
        }
        default: {
            return false;
        }
    }
    return true;
}


/*
Description:
    chosing
        what command to execute
        what arguments to take
    then executing command
Input:
    c - command
    sets - sets array
    rels - relations array
    sizes - sizes of sets and rels: sizes = [sets sizes, rels sizes]

Output:
    true if all is ok
    false if some error
*/
bool executeCommand(command *c, set **sets, relation **rels, ullong sizes[]) {

    const int commandsArrSize = BIJECT_R+1;
    static const char *commandsArr[] = {
        "empty", "card", "complement", "union", "intersect", "minus", "subseteq","subset", "equals",
        "reflexive", "symmetric", "antisymmetric", "transitive", "function", "domain", "codomain", "injective",
        "surjective", "bijective"
    };

    //find command id
    int id = -1;
    for(int i = 0; i < commandsArrSize; i++) {
        if(0 == strcmp(c->arguments[0], commandsArr[i])) {
            id = i;
            break;
        }
    }
    if(id < 0) {
        errMsg("%s", "Wrong command\n");
        return false;
    }

    //find arguments
    void *found[c->argsCount];

    int foundRels = 0, foundSets = 0;
    char *ptr;
    for(int i = 1; i < c->argsCount; i++) {
        ullong arg = strtoull(c->arguments[i], &ptr, 10);
        if(arg == 0) {
            errMsg("%s", "Wrong function argument\n");
            return false;
        }
        if(0 < findEntity((void**)sets, sizes[0], S, arg, &found[i-1])) {
            foundSets++;
            continue;
        }
        if(0 < findEntity((void**)rels, sizes[1], R, arg, &found[i-1])) {
            foundRels++;
        }
        else {
            errMsg("%s", "Entity with given id not found\n");
            return false;
        }
    }

    switch(id) {
         case(EMPTY_S): {
             if(foundSets != 1 || c->argsCount != 2) { return false; }
             return emptyCommand((set*)found[0]);
         }
         case(CARD_S): {
             if(foundSets != 1 || c->argsCount != 2) { return false; }
             return cardCommand((set*)found[0]);
         }
         case(COMPL_S): {
             if(foundSets != 1 || c->argsCount != 2) { return false; }
             return complementCommand((set*)found[0], sets[0]);
         }
         case(UNION_S): {
             if(foundSets != 2 || c->argsCount != 3) { return false; }
             return unionCommand((set*)found[0], (set*)found[1]);
         }
         case(INTERS_S): {
             if(foundSets != 2 || c->argsCount != 3) { return false; }
             return intersectCommand((set*)found[0], (set*)found[1], NULL, true);
         }
         case(MINUS_S): {
             if(foundSets != 2 || c->argsCount != 3) { return false; }
             return minusCommand((set*)found[0], (set*)found[1]);
         }
         case(SUBSTEQ_S): {
             if(foundSets != 2 || c->argsCount != 3) { return false; }
             return subseteqCommand((set*)found[0], (set*)found[1], true);
         }
         case(SUBST_S): {
             if(foundSets != 2 || c->argsCount != 3) { return false; }
             return subsetCommand((set*)found[0], (set*)found[1]);
         }
         case(EQUAL_S): {
             if(foundSets != 2 || c->argsCount != 3) { return false; }
             return equalsCommand((set*)found[0], (set*)found[1]);
         }
        case(REFLEX_R): {
            if(foundRels != 1 || c->argsCount != 2) { return false; }
            return reflexiveCommand((relation*)found[0], sets[0]);
        }
        case(SYM_R): {
            if(foundRels != 1 || c->argsCount != 2) { return false; }
            return symmetricCommand((relation*)found[0]);
        }
        case(ANTISYM_R): {
            if(foundRels != 1 || c->argsCount != 2) { return false; }
            return antisymmetricCommand((relation*)found[0]);
        }
        case(TRANS_R): {
            if(foundRels != 1 || c->argsCount != 2) { return false; }
            return transitiveCommand((relation*)found[0]);
        }
        case(FUNC_R): {
            if(foundRels != 1 || c->argsCount != 2) { return false; }
            return functionCommand((relation*)found[0]);
        }
        case(DOMAIN_R): {
            if(foundRels != 1 || c->argsCount != 2) { return false; }
            return domainCommand((relation*)found[0]);
        }
        case(CODOMAIN_R): {
            if(foundRels != 1 || c->argsCount != 2) { return false; }
            return codomainCommand((relation*)found[0]);
        }
        case(INJECT_R): {
            if(foundRels != 1 || foundSets != 2 || c->argsCount != 4) { return false; }
            return injectiveCommand((relation*)found[0], (set*)found[1], (set*)found[2]);
        }
        case(SURJECT_R): {
            if(foundRels != 1 || foundSets != 2 || c->argsCount != 4) { return false; }
            return surjectiveCommand((relation*)found[0], (set*)found[1], (set*)found[2]);
        }
        case(BIJECT_R): {
        if(foundRels != 1 || foundSets != 2 || c->argsCount != 4) { return false; }
            return bijectiveCommand((relation*)found[0], (set*)found[1], (set*)found[2]);
        }
    }

    return true;
}

/*
Description:
    help function for parseEntity funciton to parse command

Input:
    file - file descriptor where reading from
    entity - output pointer

Output:
    true if all is ok
    false if some error
*/
bool parseCommandEntity(FILE *file, command **entity) {
    int arrSize = 2;//supposed max arguments count
    int argsCount = 0;
    char **arguments = malloc(arrSize*sizeof(char*));
    for(int i = 0; i < arrSize; i++) {
        arguments[i] = NULL;
    }
    int c = ' ';
    while(c == ' ') {
        if(argsCount >= arrSize) {
            arrSize *= 2;
            char** result = realloc(arguments, arrSize*sizeof(char*));
            if(result == NULL) {
                clear2D((void**)arguments, arrSize/2);
            }
            arguments = result;
            for(int i = argsCount; i < arrSize; i++) {
                arguments[i] = NULL;
            }
        }
        if(arguments == NULL) {
            errMsg("%s", "Error occured while parsing command entity\n");
            return false;
        }
        arguments[argsCount] = malloc((MAX_SET_ELEM_LEN+1)*sizeof(char));
        if(arguments[argsCount] == NULL) {
            errMsg("%s", "No memory for command argument\n");
            clear2D((void**)arguments, arrSize);
            return false;
        }
        for(ullong i = 0; i < MAX_SET_ELEM_LEN + 1; i++) {
            c = getc(file);
            if(c != ' ' && c != '\n' && c != EOF)  {
                arguments[argsCount][i] = c;
            }
            else {
                arguments[argsCount][i] = '\0';
                break;
            }
        }
        arguments[argsCount][MAX_SET_ELEM_LEN] = '\0';
        argsCount++;
    }
    ungetc(c, file);
    bool retVal = initCommand(entity, arguments, argsCount);
    clear2D((void**)arguments, arrSize);
    return retVal;
}

/*
Description:
    help function for parseEntity funciton to parse Relation

Input:
    file - file descriptor where reading from
    entity - output pointer
Output:
    true if all is ok
    false if some error
*/
bool parseRelationEntity(FILE *file, relation **entity) {
    ullong arrSize = 100;//supposed max relation elements count
    ullong elemsCount = 0;
    set **elements = malloc(arrSize*sizeof(set*));
    if(elements == NULL) {
        errMsg("%s", "No memory for relation array\n");
        return false;
    }
    for(ullong i = 0; i < arrSize; i++) {
        elements[i] = NULL;
    }
    int c = ' ';
    while(c == ' ') {
        c = getc(file);
        if(c != '(') {
            errMsg("%s", "Wrong input file information got while reading relation entity\n");
            clear2D((void**)elements, arrSize);
            return false;
        }
        if(elemsCount >= arrSize) {
            arrSize*=2;
            set** result = realloc(elements, arrSize*sizeof(set*));
            if(result == NULL) {
                clearEntities2D((void **) elements, arrSize / 2, S);
            }
            elements = result;
            for(ullong i = elemsCount; i < arrSize; i++) {
                elements[i] = NULL;
            }
        }
        if(elements == NULL) {
            errMsg("%s", "No memory for relation array\n");
            return false;
        }
        if(false == parseSetEntity(file, &(elements[elemsCount]))) {
            errMsg("%s", "No memory for relation array element\n");
            clearEntities2D((void**)elements, arrSize, S);
            return false;
        }
        elemsCount++;
        c = getc(file);
        if(c != ')') {
            errMsg("%s", "Wrong input file information got while reading relation entity\n");
            clearEntities2D((void**)elements, arrSize, S);
            return false;
        }
        c = getc(file);
    }
    ungetc(c, file);
    bool retVal = initRelation(entity, elements, elemsCount, false);
    clearEntities2D((void**)elements, arrSize, S);
    return retVal;
}


/*
Description:
    help function for parseEntity funciton to parse Set

Input:
    file - file descriptor where reading from
    entity - output pointer

Output:
    true if all is ok
    false if some error
*/
bool parseSetEntity(FILE *file, set **entity) {
    ullong arrSize = 100;//supposed max set elements count
    ullong elemsCount = 0;
    char **elements = malloc(arrSize*sizeof(char*));
    for(ullong i = 0; i < arrSize; i++) {
        elements[i] = NULL;
    }
    int c = ' ';
    while(c == ' ') {
        if(elemsCount >= arrSize) {
            arrSize *= 2;
            char **result = realloc(elements, arrSize*sizeof(char*));
            if(result == NULL) {
                clear2D((void **) elements, arrSize / 2);
            }
            elements = result;
            for(ullong i = elemsCount; i < arrSize; i++) {
                elements[i] = NULL;
            }
        }
        if(elements == NULL) {
            errMsg("%s", "No memory for set array\n");
            return false;
        }
        elements[elemsCount] = malloc((MAX_SET_ELEM_LEN+1)*sizeof(char));
        if(elements[elemsCount] == NULL) {
            errMsg("%s", "No memory for set element\n");
            clear2D((void**)elements, arrSize);
            return false;
        }
        for(int i = 0; i < MAX_SET_ELEM_LEN + 1; i++) {
            c = getc(file);
            //english characters only
            if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
                elements[elemsCount][i] = (char)c;
            }
            else {
                elements[elemsCount][i] = '\0';
                break;
            }
        }
        elements[elemsCount][MAX_SET_ELEM_LEN] = '\0';
        elemsCount++;
    }
    ungetc(c, file);

    bool retVal = initSet(entity, elements, elemsCount, false);
    clear2D((void**)elements, arrSize);
    return retVal;
}


/*
Description:
    reads given entity from text file

Input:
    file - opened file descriptor where reading from
    object - type of entity to parse
    entity - output pointer
            initialized entity
            or
            NULL if it is another existing entity type

Output:
    true if all is ok
    false if some error
    pointer to initialized entity

*/
bool parseEntity(FILE *file, entities object, void **entity) {
    if(file == NULL) {
        errMsg("%s", "Wrong file descriptor\n");
        return false;
    }
    if(feof(file) != 0) {
        *entity = NULL;
        return true;
    }
    //array depends on entities datatype
    static const char charEntities[ENTITIES_COUNT] = {'U', 'S', 'R', 'C'};
    static int c = '\0';       //why static? after function found out it got another entity type,
    if(c == '\0') {             //next function call should remember that.
        c = getc(file);   //Otherwise it just reading next symbol in file)
    }
    if(c != charEntities[object])
    {
        //check if it is another antity, otherway it is definitely error
        bool another = false;
        for(int i = 0; i < ENTITIES_COUNT; i++) {
            if(c == charEntities[i]) {
                another = true;
                break;
            }
        }
        if(!another) {
            errMsg("%s", "Found non existing entity\n");
            return false;
        }
        else {
            *entity = NULL;
            return true;
        }
    }
    //parsing entity logic
    c = getc(file);
    if(c != ' ' && c != '\n' && c != EOF) {
        errMsg("%s", "Wrong input file information got while reading entity\n");
        return false;
    }
    bool retVal = true;
    switch(object) {
        case(U):
        case(S): {
            if(c == ' ') {
                retVal = parseSetEntity(file, (set**)entity);
            } 
            else {
                retVal = initSet((set**)entity, NULL, 0, false);
                ungetc(c, file);
            }
            break;
        }
        case(R): {
            if(c == ' ') {
                retVal = parseRelationEntity(file, (relation**)entity);
            }
            else {
                retVal = initRelation((relation**)entity, NULL, 0, false);
                ungetc(c, file);
            }
            break;
        }
        case(C): {
            retVal = parseCommandEntity(file, (command**)entity);
            break;
        }
        default: {
            errMsg("%s", " Unknown entity type\n");
            return false;
        }
    }
    c = getc(file);
    if((c != EOF && c != '\n') || (retVal == false)) {
        errMsg("%s", "Wrong input file information got while reading set entity\n");
        return false;
    }
    if(c == '\n') {
        if((c = getc(file)) != EOF) {
            ungetc(c, file);
        }
    }
    c = '\0';//entity is readed => function can forget about what entity type it had
    return retVal;
}

/*
Description:
    parses given text file

Input:
    filename - string

Output:
    true if all is ok
    false if some error
*/


bool parseInputFile(FILE *inputFile) {

    if(inputFile == NULL || feof(inputFile) != 0) {
        return true;
    }
    ullong lineNum = 1;
    bool foundRepeating = false;
    //preparing forbidden words, parsing universe
    const int forbiddenSize = 20;
    static char *forbidden[] = {
        "true", "false", "empty", "card", "complement", "union", "intersect", "minus", "subseteq", "equals",
        "reflexive", "symmetric", "antisymmetric", "transitive", "function", "domain", "codomain", "injective",
        "surjective", "bijective"
    };
    set *forbiddenSet = NULL;
    if(false == initSet(&forbiddenSet, forbidden, forbiddenSize, true)) {
        errMsg("%s", "Error occured while defining forbidden set\n");
        destroySet(forbiddenSet);
        return false;
    }

    set *universe = NULL;
    void *arg = NULL;
    bool retVal = parseEntity(inputFile, U, &arg);
    universe = (set*)arg;
    if((retVal == false) || (universe == NULL)) {
        errMsg("%s", "First entity is not universe\n");
        destroySet(forbiddenSet);
        destroySet(universe);
        return false;
    }


    set *resultSet = NULL;
    intersectCommand(universe, forbiddenSet, &resultSet, false);
    if(resultSet->cardinality != 0)
    {
        errMsg("%s", "Found forbidden words\n");
        destroySet(resultSet);
        destroySet(forbiddenSet);
        destroySet(universe);
        return false;
    }
    destroySet(resultSet);
    destroySet(forbiddenSet);

    set *uniqieSet = NULL;
    if(false == makeUniqueSet(universe, &uniqieSet)) {
         foundRepeating = true;
    }
    if(uniqieSet != NULL) {
        destroySet(universe);
        universe = uniqieSet;
    }



    universe->id = lineNum;
    lineNum++;

    printEntity(universe, U);

    //preparing to parse sets and relations
    set **setArr;
    setArr = malloc(sizeof(set*)*MAX_LINES_COUNT);
    if(setArr == NULL) {
        errMsg("%s", "No memory for array of sets\n");
        destroySet(universe);
        return false;
    }
    for(ullong i = 0; i < MAX_LINES_COUNT; i++) {
        setArr[i] = NULL;
    }
    setArr[0] = universe;


    relation **relArr;
    relArr = malloc(sizeof(relation*)*MAX_LINES_COUNT);
    if(relArr == NULL) {
        errMsg("%s", "No memory for array of realtions\n");
        clearEntities2D((void**)setArr, MAX_LINES_COUNT, S);
        return false;
    }
    for(ullong i = 0; i < MAX_LINES_COUNT; i++) {
        relArr[i] = NULL;
    }

    //parsing relations and sets
    ullong setIdx = 1, relIdx = 0;//after universe
    int wrongTries = 0;
    while(wrongTries < ENTITIES_COUNT) {
        if(lineNum > MAX_LINES_COUNT) {  
            errMsg("%s", "Too many lines in input file\n");
            clearPIF
            return false;
        }
        while(true) {
            void *arg = NULL;
            if(false == parseEntity(inputFile, S, &arg)) {
                errMsg("%s", "Error occured while parsing set\n");
                destroySet((set*)arg);
                clearPIF
                return false;
            }
            setArr[setIdx] = (set*)arg;
            if(setArr[setIdx] == NULL) {
                wrongTries++; break;
            }

            set *uniqieSet = NULL;
            if(false == makeUniqueSet(setArr[setIdx], &uniqieSet)) {
                foundRepeating = true;
            }
            if(uniqieSet != NULL) {
                destroySet(setArr[setIdx]);
                setArr[setIdx] = uniqieSet;
            }



            int result = subseteqCommand(setArr[setIdx], universe, false); // TODO
            if(result == -1) {
                errMsg("%s", "Set elements are not from universe\n");
                clearPIF
                return false;
            }

            wrongTries = 0;
            setArr[setIdx]->id = lineNum;
            printEntity(setArr[setIdx], S);
            setIdx++; lineNum++;

        }

        while(true) {
            void *arg = NULL;
            if(false == parseEntity(inputFile, R, &arg)) {
                errMsg("%s", "Error occured while parsing relation\n");
                destroyRelation((relation*)arg);
                clearPIF
                return false;
            }
            relArr[relIdx] = (relation*)arg;
            if(relArr[relIdx] == NULL) {
                wrongTries++; break;
            }

            relation *uniqueRelation = NULL;
            if(false == makeUniqueRelation(relArr[relIdx], &uniqueRelation)) {
                foundRepeating = true;
            }
            if(uniqueRelation != NULL) {
                destroyRelation(relArr[relIdx]);
                relArr[relIdx] = uniqueRelation;
            }


            int result;
            for(ullong i = 0; i < relArr[relIdx]->cardinality; i++) {
                result = subseteqCommand(relArr[relIdx]->elements[i], universe, false);
                if(result == -1) {
                    errMsg("%s", "Set elements of a relation are not from universe\n");
                    clearPIF
                    return false;
                }
            }

            wrongTries = 0;
            relArr[relIdx]->id = lineNum;
            printEntity(relArr[relIdx], R);
            relIdx++; lineNum++;
        }
    }

    if(feof(inputFile) != 0) {
        errMsg("%s", "Error, no commands found\n");
        clearPIF
        return false;
    }

    while(feof(inputFile) == 0) {
        command *c = NULL;
        void *arg = NULL;
        bool retVal = parseEntity(inputFile, C, &arg);
        c = (command*)arg;
        if(retVal == false || c == NULL) {//no entities after command => NULL value becomes error
            errMsg("%s", "Error occured while parsing command\n");
            clearPIF
            destroyCommand(c);
            return false;
        }
        lineNum++;
        ullong sizes[] = {setIdx, relIdx};
        if(false == executeCommand(c, setArr, relArr, sizes)) {
            errMsg("%s", "Error occured while executing command\n");
            destroyCommand(c);
            clearPIF
            return false;
        }
        destroyCommand(c);
    }


    clearPIF
    if(foundRepeating) {
        errMsg("%s", "Warning! found repeating elements\n");
        return false;
    }
    if(setIdx == 1 && relIdx == 0) {
        return false;
    }
    return true;
}

////////// SETS COMMANDS //////////

/*
Description:
    initializes a set command's result set
    prints it and free's all memory used there

Input:
    char** newSetElements - array of strings that are going to be printed
    ullong newSetCardinality - cardinality of the resulted set

Output:
    -        
*/


void onSetCommandExit(char** newSetElements, ullong newSetCardinality) {
    set* newSet;
    initSet(&newSet, newSetElements, newSetCardinality, false);
    printEntity(newSet, S);
    clear2D((void **)newSetElements, newSetCardinality);
    destroySet(newSet);    
}

/*
Description:
    allocates space for new set elements

Input:
    ullong size - the amount to be allocated (set's cardinality)

Output:
    returns a pointer to the allocated space
    or returns NULL if there's an error       
*/

char **allocateNewElements(ullong size) {
    char **newElements = malloc(size * sizeof(char *));

    if (newElements == NULL) {
        return NULL;
    }

    for (ullong i = 0; i < size; i++) {                   
        newElements[i] = malloc((MAX_SET_ELEM_LEN+1)*sizeof(char));
        if (newElements[i] == NULL) {
            clear2D((void **)newElements, i);
            return NULL;
        }
    }
    return newElements;
}

/*
Description:
    checks whether given set is empty

Input: 
    setPtr - pointer to set struct

Output:
    prints false if its empty or
    prints true if its not empty and returns true
    returns false if the set does not exist
*/

bool emptyCommand(set* setPtr) {
    if (setPtr == NULL) {
        errMsg("%s", "Error occured while executing command empty.\n");
        return false;
    }

    if (setPtr -> cardinality == 0) { 
        puts("true");
    }

    else { 
        puts("false");
    }

    return true;
}

/*
Description:
    prints the cardinality of a set

Input: 
    setPtr - pointer to set struct

Output:
    prints the cardinality of a set and returns true 
    returns false if the set does not exist
*/


bool cardCommand(set* setPtr) {
    if (setPtr == NULL) {
        errMsg("%s", "Error occured while executing command card.\n");
        return false;
    }
    
    printf("%llu\n", setPtr -> cardinality);
    return true;
}

/*
Description:
    prints the complement of a set

Input: 
    setPtr - pointer to set struct
    uniPtr - pointer to universum set struct

Output:
    prints the complement of a set and returns true
    or just returns true if the complement is an empty set 
    returns false if there's an error
*/


bool complementCommand(set* setPtr, set* uniPtr) {
    bool difference = false;
    ullong pos = 0;

    char *errorStr = "Error occured while executing command complement.\n";

    if (setPtr == NULL || uniPtr == NULL) {
        printf("%s", errorStr);
        return false;
    }

    if ((setPtr -> cardinality) == (uniPtr -> cardinality)) {  //the complement is an empty set
        puts("S");
        return true;
    }

    if ((setPtr -> cardinality == 0)) { // the given set is an empty set, so we print the universum
        printEntity(uniPtr, S);
        return true;
    }

    ullong newCardinality = ((uniPtr -> cardinality) - (setPtr -> cardinality));

    char **newElements = allocateNewElements(newCardinality);

    if (newElements == NULL) {
        printf("%s", errorStr);
        return false;
    }

    for (ullong i = 0; i < (uniPtr -> cardinality); i++) { // looping through the universum's elements
        for (ullong j = 0; j < (setPtr -> cardinality); j++) { //if we find an i-th element of universum in the set, we break
            if (strcmp(uniPtr -> elements[i], setPtr -> elements[j]) == 0) {   
                difference = false; 
                break;
            }
            else {
                difference = true;
            }
        }

        if (difference) {
            strcpy(newElements[pos],uniPtr -> elements[i]);
            pos ++;
            difference = false; 
        }
    }

    onSetCommandExit(newElements, newCardinality);    
    return true;
}


/*
Description:
    prints the union of two sets

Input: 
    *setA - pointer to first set struct
    *setB - pointer to second set struct

Output:
    prints the union of two sets and returns true
    returns false if there's an error
*/


bool unionCommand (set* setA, set* setB) {
    ullong newCardinality = 0;
    ullong count = 0;
    ullong offset = 0;
    char *errorStr = "Error occured while executing command union.\n";

    if (setA == NULL || setB == NULL) {
        printf("%s", errorStr);
        return false;
    }

    if (setA -> cardinality == 0 && setB -> cardinality == 0) { // both sets are empty, so the union is an empty set
        puts("S");
        return true;
    }

    if (setA -> cardinality == 0) {
        printEntity(setB, S);
        return true;
    }

    if (setB -> cardinality == 0) {
        printEntity(setA, S);
        return true;
    }

    newCardinality = setA -> cardinality;

    for (ullong i = 0; i < setB -> cardinality; i++) {  // here we are calculating the cardinality of the union
        for (ullong j = 0; j < setA -> cardinality; j++) { 
            if (strcmp(setB -> elements[i], setA -> elements[j]) == 0) {  // same element found
                break;               
            }                                   // we are looking for elements, that set B has, but set A doesnt and then add the amount to set A's cardinality
            else {
                count++;
            } 
        }
        if (count == setA -> cardinality) {
            newCardinality++;
        }
        count = 0;
    }

    char **newElements = allocateNewElements(newCardinality);
    
    if (newElements == NULL) {
        printf("%s", errorStr);
        return false;
    }

    for (ullong i = 0; i < setA -> cardinality; i++) {   // copy setA elements to union elements
        strcpy(newElements[i],setA -> elements[i]);
    }

    for (ullong i = 0; i < setB -> cardinality; i++) {  //  find elements that setB has but setA doesnt
        for (ullong j = 0; j < setA -> cardinality; j++) { 
            if (strcmp(setB -> elements[i], setA -> elements[j]) == 0) {  // same element found
                break;
            }
            else {
                count++;
            } 
        }
        if (count == setA -> cardinality) {
            strcpy(newElements[setA -> cardinality + offset],setB -> elements[i]); // append it to the union set's elements
            offset++;
        }
        count = 0;
    }

    onSetCommandExit(newElements, newCardinality);
    return true;
}

/*
Description:
    prints the intersect of two sets

Input: 
    *setA - pointer to first set struct
    *setB - pointer to second set struct
    **resultSet - this can be a NULL pointer, if its not, then the intersect will be stored in here
    bool print - if true is passed, the intersect gets printed

Output:
    either prints the intersect of two sets and returns 2 
    or initializes the intersect of 2 sets, but doesn't print it & returns 1
    or else returns 0 if there's and error
*/

int intersectCommand (set* setA, set* setB, set** resultSet, bool print) { 
    ullong newCardinality = 0;
    ullong elementId = 0;
    char *errorStr = "Error occured while executing command intersect.\n";

    if (setA == NULL || setB == NULL) {
        printf("%s", errorStr);
        return 0;
    }

    for (ullong i = 0; i < setA -> cardinality; i++) { 
        for (ullong j = 0; j < setB -> cardinality; j++) { 
            if (strcmp(setA -> elements[i], setB -> elements[j]) == 0) {   // calculate the new cardinality by counting same elements
                newCardinality++;
                break;
            }
        }
    }

    if (newCardinality == 0) {      // sets have the same elements
        if (resultSet == NULL && print) {
            puts("S");
            return 2;
        }
        else {
            initSet(resultSet, NULL, 0, false);
            return 1;
        }
    }

    char **newElements = allocateNewElements(newCardinality);
    
    if (newElements == NULL) {
        printf("%s", errorStr);
        return 0;
    }

    for (ullong i = 0; i < setA -> cardinality; i++) { // look for the same elements that both sets have
        for (ullong j = 0; j < setB -> cardinality; j++) { // and append them to newElements array
            if (strcmp(setA -> elements[i], setB -> elements[j]) == 0) {
                strcpy(newElements[elementId], setA -> elements[i]);
                elementId++;
                break;
            }
        }
    }

    if (resultSet == NULL && print) {
        onSetCommandExit(newElements, newCardinality);
        return 2;
    }
    else {
        initSet(resultSet, newElements, newCardinality, false);
        clear2D((void **) newElements, newCardinality);
        return 1;
    }
}

/*
Description:
    prints the difference of two sets

Input: 
    *setA - pointer to first set struct
    *setB - pointer to second set struct

Output:
    prints the difference of two sets and returns true
    returns false if there's an error
*/

bool minusCommand(set* setA, set* setB) {
    ullong newCardinality = 0;
    ullong sameElements = 0;
    ullong offset = 0;
    char *errorStr = "Error occured while executing command minus.\n";

    if (setA == NULL || setB == NULL) {
        errMsg("%s", errorStr);
        return false;
    }

    for (ullong i = 0; i < setA -> cardinality; i++) { // we count the number of same elements
        for (ullong j = 0; j < setB -> cardinality; j++) {  // and use this to calculate the minus set's cardinality
            if (strcmp(setA -> elements[i], setB -> elements[j]) == 0) {
                sameElements ++;
                break;
            }
        }
    }

    newCardinality = (setA -> cardinality) - sameElements;

    char **newElements = allocateNewElements(newCardinality);
    
    if (newElements == NULL) {
        printf("%s", errorStr);
        return false;
    }

    sameElements = 0;

    for (ullong i = 0; i < setA -> cardinality; i++) { // we do the same thing
        for (ullong j = 0; j < setB -> cardinality; j++) {  // but now we append the elements
            if (strcmp(setA -> elements[i], setB -> elements[j]) == 0) {
                sameElements ++;
                break;
            }
        }
        if (sameElements == 0) {
            strcpy(newElements[offset],setA -> elements[i]);
            offset++;
        }
        sameElements = 0;
    }

    onSetCommandExit(newElements, newCardinality);
    return true;
}

/*
Description:
    prints true if a set is a subset of another set, false otherwise

Input: 
    *setA - pointer to first set struct
    *setB - pointer to second set struct
    bool print - prints the result if true is passed

Output:
    if bool print is true: 
    prints true if setA is a subset of setB and returns 2
    prints false if setA is not a subset of setB and returns 2

    if bool print is false:
    returns 1 if setA is a subset of setB
    returns -1 if setA is not a subset of setB

    returns 0 if there's an error
*/

int subseteqCommand(set* setA, set* setB, bool print) { 
    ullong sameElements = 0;

    if (setA == NULL || setB == NULL) {
        errMsg("%s", "Error occured while executing command subseteq.\n");
        return 0;
    }

    if (setA -> cardinality > setB -> cardinality) { // if a set's cardinality is greater than another's, it can't be it's subset
        if (!print) {
            return -1;
        }
        else {
            puts("false");
            return 2;
        }
    }

    for( ullong i = 0; i < setA -> cardinality; i++) { // calculating the amount of same elements
        for( ullong j = 0; j < setB -> cardinality; j++) { 
            if (strcmp(setA -> elements[i], setB -> elements[j]) == 0) {
                sameElements++;
                break;
            }
        }
    }

    if (sameElements == (setA -> cardinality)) { // if the count of same elements that both set A and B have
        if (!print) {                            // is the same as set A's cardinality
            return 1;                            // then set A is a subset of set B
        }
        else {
            puts("true");
            return 2;
        }
    }

    else {
        if (!print) {
            return -1;
        }
        else {
            puts("false");
            return 2;
        }
    }
}

/*
Description:
    prints true if a set is a proper subset of another set, false otherwise

Input: 
    *setA - pointer to first set struct
    *setB - pointer to second set struct

Output:
    prints true if setA is a proper subset of setB and returns true
    prints false if setA is not a proper subset of setB and returns true
    returns false if there's an error
*/


bool subsetCommand(set* setA, set* setB) {

    if (setA == NULL || setB == NULL) {
        errMsg("%s", "Error occured while executing command subset.\n");
        return false;
    }

    ullong sameElements = 0;

    if (setA -> cardinality >= setB -> cardinality) {  // completely same as subseteq, just need to check, whether the cardinality is the same
        puts("false");
        return true;
    }

    for( ullong i = 0; i < setA -> cardinality; i++) { 
        for( ullong j = 0; j < setB -> cardinality; j++) { 
            if (strcmp(setA -> elements[i], setB -> elements[j]) == 0) {
                sameElements++;
                break;
            }
        }
    }



    if (sameElements == (setA -> cardinality)) {
        puts("true");
    }

    else {
        puts("false");
    }

    return true;
}

/*
Description:
    prints true if a set is equal to another set, false otherwise

Input: 
    *setA - pointer to first set struct
    *setB - pointer to second set struct

Output:
    prints true if the sets are equal, false otherwise and returns true
    returns false if there's an error
*/

bool equalsCommand(set* setA, set* setB) {
    ullong sameElements = 0;

    if (setA == NULL || setB == NULL) {
        errMsg("%s", "Error occured while executing command equal.\n");
        return false;
    }

    if (setA -> cardinality != setB -> cardinality) { // if the cardinalities arent the same, the sets cant be equal
        puts("false");
        return true;
    }

    for( ullong i = 0; i < setA -> cardinality; i++) { // count the number of same elements the sets have
        for( ullong j = 0; j < setB -> cardinality; j++) {  
            if (strcmp(setA -> elements[i], setB -> elements[j]) == 0) {
                sameElements++;
                break;
            }
        }
    }

    if (sameElements == (setA -> cardinality)) { // and compare it to sets' cardinalities
        puts("true");
    }

    else {
        puts("false");
    }

    return true;
}

////// RELATIONS COMMANDS ///////
/*
Description:
    return true if a relation is function, otherwise false
Input:
    *r - pointer to relation
Output:
    If relation is function returns true, if not false
*/
bool helpFunctionComand(relation *r){
  for(ullong i = 0;i < r->cardinality;i++) {
    for(ullong j = 0;j < r->cardinality;j++) {
      if(!(strcmp(r->elements[i]->elements[0], r->elements[j]->elements[0]))) {
        if(!(strcmp(r->elements[i]->elements[1], r->elements[j]->elements[1]))){
          continue;
        }
        return false;
      }
    }
  }
  return true;
}
/*
Description:
    prints true if a relation is function, otherwise false
Input:
    *r - pointer to relation
Output:
    If relation is function prints true, if not false
*/
bool functionCommand(relation *r) {
  if(r == NULL) {
      errMsg("%s", "Error occured while executing function command.\n");
      return false;
  }
  if(helpFunctionComand(r)){
    printf("%s\n", "true");
  }else{
    printf("%s\n", "false");
  }
  return true;
}

/*
Description:
    prints true if a relation antiSymmetric, otherwise false
Input:
    *r - pointer to relation
Output:
    If relation is antisymmetric prints true, if not false
*/

bool antisymmetricCommand(relation *r){
  if(r == NULL) {
      errMsg("%s", "Error occured while executing antisymmetric command.\n");
      return false;
  }
  for(ullong i = 0; i < r->cardinality;i++){
    for(ullong j = 0;j < r->cardinality;j++){
      if(!(strcmp(r->elements[j]->elements[0], r->elements[i]->elements[1]))){
        if(!(strcmp(r->elements[j]->elements[1], r->elements[i]->elements[0]))){
          if(!(strcmp(r->elements[j]->elements[0], r->elements[j]->elements[1]))){
            continue;
          }else{
            printf("%s\n", "false");
            return true;
          }
        }
      }
    }
  }
  printf("%s\n", "true");
  return true;
}

/*
Description:
    prints true if a relation is symetric, otherwise false
Input:
    *r - pointer to relation
Output:
    If relation is symetric prints true, if not false
*/

bool symmetricCommand(relation *r){
  if(r == NULL) {
      errMsg("%s", "Error occured while executing symmetric command.\n");
      return false;
  }
  bool symPair = false;
  for(ullong i = 0;i < r->cardinality;i++){
    for(ullong j = 0;j < r->cardinality;j++){
      if(!(strcmp(r->elements[i]->elements[0], r->elements[i]->elements[1]))){
        symPair = true;
        break;
      }
      if(i == r->cardinality -1){
        printf("%s\n", "false");
        return true;
      }
      if(!(strcmp(r->elements[j]->elements[0], r->elements[i]->elements[1]))){
        if(!(strcmp(r->elements[j]->elements[1], r->elements[i]->elements[0]))){
          symPair = true;
          if(i == r->cardinality -2){
            printf("%s\n", "true");
            return true;
          }
          break;
        }
      }
    }
    if(!symPair){
      printf("%s\n", "false");
      return true;
    }
    symPair = false;
  }
  printf("%s\n", "true");
  return true;
}

/*
Description:
    prints true if a relation is transitive, otherwise false
Input:
    *r - pointer to relation
Output:
    If relation is transitive prints true, if not false
*/

bool transitiveCommand(relation *r){
  if(r == NULL) {
      errMsg("%s", "Error occured while executing transitive command.\n");
      return false;
  }
  bool isThere = false;
  bool wasThere = false;
  for(ullong i=0;i<r->cardinality;i++){
    for(ullong j=0;j<r->cardinality;j++){
      if(!(strcmp(r->elements[i]->elements[1], r->elements[j]->elements[0]))){
        wasThere = true;
        for(ullong k=0;k<r->cardinality;k++){
          if(!(strcmp(r->elements[i]->elements[0], r->elements[k]->elements[0]))){
            if(!(strcmp(r->elements[j]->elements[1], r->elements[k]->elements[1]))){
              isThere = true;
            }
          }
        }
      }
    }
    if(!isThere && wasThere){
      printf("%s\n", "false");
      return true;
    }
    isThere = false;
    wasThere = false;
  }
  printf("%s\n", "true");
  return true;
}

/*
Description:
    prints true if a relation is function, otherwise false
Input:
    *r - pointer to relation
    *u - pointer to universe
Output:
    If relation is function prints true, if not false
*/
bool reflexiveCommand(relation *r, set *u){
  if(r == NULL || u == NULL) {
      errMsg("%s", "Error occured while executing reflexive command.\n");
      return false;
  }
  ullong count = 0;
  for(ullong i = 0;i<u->cardinality;i++){
    for(ullong j = 0;j<r->cardinality;j++){
      if(!(strcmp(u->elements[i], r->elements[j]->elements[0]))){
        if(!(strcmp(u->elements[i], r->elements[j]->elements[1]))){
          count++;
          break;
        }
      }
    }
  }
  if(count == u->cardinality){
    printf("%s\n", "true");
    return true;
  }
  printf("%s\n", "false");
  return true;
}
/*
Description:
    function that prints codomain of a relation
Input:
    *r - pointer to relation
Output:
    prints codomain of relation
*/
bool codomainCommand(relation *r){
  ullong pos = 0;
  ullong newCardinality = r->cardinality;
  char **newElements = malloc(newCardinality*sizeof(char*));

  if(newElements == NULL || r == NULL){
    errMsg("%s", "Error occured while executing codomain command.\n");
    return false;
  }
  for (ullong i = 0; i < newCardinality; i++) {
      newElements[i] = malloc((MAX_SET_ELEM_LEN + 1)*sizeof(char));
      if (newElements[i] == NULL) {
          errMsg("%s", "Error occured while executing codomain command.\n");
          return false;
      }
      strcpy(newElements[i], "\0");
  }
  bool isThere = false;
  for(ullong i = 0;i<r->cardinality;i++){
    for(ullong j = 0;j<r->cardinality;j++){
      if(!(strcmp(r->elements[i]->elements[1], newElements[j]))){
        isThere = true;
        newCardinality--;
        break;
      }
    }
    if(!isThere){
      strcpy(newElements[pos], r->elements[i]->elements[1]);
      pos++;
    }
    isThere = false;
  }
  set* newSet;

  initSet(&newSet, newElements, newCardinality, false);
  printEntity(newSet, S);
  destroySet(newSet);
  clear2D((void **) newElements, r->cardinality);
  return true;
}
/*
Description:
    function that prints domain of a relation
Input:
    *r - pointer to relation
Output:
    prints domain of relation
*/
bool domainCommand(relation *r){
  ullong pos = 0;
  ullong newCardinality = r->cardinality;
  char **newElements = malloc(newCardinality*sizeof(char *));

  if(newElements == NULL || r == NULL){
    errMsg("%s", "Error occured while executing domain command.\n");
    return false;
  }
  for (ullong i = 0; i < newCardinality; i++) {
      newElements[i] = malloc((MAX_SET_ELEM_LEN + 1)*sizeof(char));
      if (newElements[i] == NULL) {
          errMsg("%s", "Error occured while executing domain command.\n");
          return false;
      }
      strcpy(newElements[i], "\0");
  }
  bool isThere = false;
  for(ullong i = 0;i<r->cardinality;i++){
    for(ullong j = 0;j<r->cardinality;j++){
      if(!(strcmp(r->elements[i]->elements[0], newElements[j]))){
        isThere = true;
        newCardinality--;
        break;
      }
    }
    if(!isThere){
      strcpy(newElements[pos], r->elements[i]->elements[0]);
      pos++;
    }
    isThere = false;
  }
  set* newSet;

  initSet(&newSet, newElements, newCardinality, false);
  printEntity(newSet, S);
  destroySet(newSet);
  clear2D((void **) newElements, r->cardinality);
  return true;
}
/*
Description:
    function that checks if first element of relation is in set A, and if second
    element of relation is in B
Input:
    *r - pointer to relation
    *a - pointer to set A
    *b - pointer to set B
Output:
    return true or false
*/
bool helpJectiveCommand(relation *r, set *a, set *b){
  bool isThere = false;
  for(ullong i = 0;i < r->cardinality;i++){
    for(ullong j = 0;j < a->cardinality;j++){
      if(!(strcmp(r->elements[i]->elements[0], a->elements[j]))){
          isThere = true;
          break;
      }
    }
    if(!isThere){
      return false;
    }
    isThere = false;
  }
  isThere = false;
  for(ullong i = 0;i < r->cardinality;i++){
    for(ullong j = 0;j < b->cardinality;j++){
      if(!(strcmp(r->elements[i]->elements[1], b->elements[j]))){
          isThere = true;
          break;
      }
    }
    if(!isThere){
      return false;
    }
    isThere = false;
  }
  if(r->cardinality == 0){
    if(a->cardinality == 0 && b->cardinality == 0){
      return true;
    }else{
      return false;
    }
  }
  return true;
}
/*
Description:
    help function that checks if relation r on set A and set B is injective
Input:
    *r - pointer to relation
    *a - pointer to set A
    *b - pointer to set B
Output:
    return true or false
*/
bool helpInjectiveCommand(relation *r, set *a, set *b) {
  if(helpFunctionComand(r) && helpJectiveCommand(r,a,b)){
    if(r->cardinality != 0){
      for(ullong i = 0;i<r->cardinality-1;i++){
        for(ullong j = i+1;j<r->cardinality;j++){
          if(!(strcmp(r->elements[i]->elements[1], r->elements[j]->elements[1]))){
            if(!(strcmp(r->elements[i]->elements[0], r->elements[j]->elements[0]))){
              continue;
            }else{
              return false;
            }
          }
        }
      }
    }
  }else{
    return false;
  }
  return true;
}
/*
Description:
    function that checks if relation r on set A and set B is injective
Input:
    *r - pointer to relation
    *a - pointer to set A
    *b - pointer to set B
Output:
    return true if everything went well otherwise false
*/
bool injectiveCommand(relation *r, set *a, set *b){
  if(r == NULL || a == NULL || b == NULL){
    errMsg("%s", "Error occured while executing injective command.\n");
    return false;
  }
  if(helpInjectiveCommand(r,a,b)){
    printf("%s\n", "true");
  }else{
    printf("%s\n", "false");
  }
  return true;
}
/*
Description:
    help function that checks if relation r on set A and set B is surjective
Input:
    *r - pointer to relation
    *a - pointer to set A
    *b - pointer to set B
Output:
    return true or false
*/
bool helpSurjectiveCommand(relation *r, set *a, set *b) {
  if(helpFunctionComand(r) && helpJectiveCommand(r,a,b)){
    bool isThere = false;
    for(ullong i = 0;i < b->cardinality;i++){
      for(ullong j = 0;j < r->cardinality;j++){
        if(!(strcmp(b->elements[i], r->elements[j]->elements[1]))){
            isThere = true;
            break;
        }
      }
      if(!isThere){
        return false;
      }
      isThere = false;
    }
  }else{
    return false;
  }
  return true;
}
/*
Description:
    function that checks if relation r on set A and set B is injective
Input:
    *r - pointer to relation
    *a - pointer to set A
    *b - pointer to set B
Output:
    return true if everything went well, otherwise false
*/
bool surjectiveCommand(relation *r, set *a, set *b){
  if(r == NULL || a == NULL || b == NULL){
    errMsg("%s", "Error occured while executing surjective command.\n");
    return false;
  }
  if(helpSurjectiveCommand(r,a,b)){
    printf("%s\n", "true");
  }else{
    printf("%s\n", "false");
  }
  return true;
}
/*
Description:
    function that checks if relation r on set A and set B is bijective
Input:
    *r - pointer to relation
    *a - pointer to set A
    *b - pointer to set B
Output:
    return true if everything went well, otherwise false
*/
bool bijectiveCommand(relation *r, set *a, set *b) {
  if(r == NULL || a == NULL || b == NULL){
    errMsg("%s", "Error occured while executing bijective command.\n");
    return false;
  }
  if(helpInjectiveCommand(r,a,b) && helpSurjectiveCommand(r,a,b)){
    printf("%s\n", "true");
  }else{
    printf("%s\n", "false");
  }
  return true;
}
