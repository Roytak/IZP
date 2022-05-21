/*
 *  Nazev: IZP Projekt 1: Overovani sily hesel (prace s textem)
 *  Autor: Roman Janota
 *  Login: xjanot04
 *  Email: xjanot04@stud.fit.vutbr.cz
 *  Datum: 23.10.2021
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define ASCII_LENGTH 128
#define MAX_LENGTH 101
#define ASCII_SPACE 32
#define ASCII_SLASH 47
#define ASCII_COLON 58
#define ASCII_AT 64
#define ASCII_BRACKET 91
#define ASCII_BACKTICK 96
#define ASCII_CURLYBRACKET 123
#define ASCII_TILDE 126
#define BUFFER_SIZE 103

/*
 * Porovna 2 retezce
 */

bool compareTwoStrings(const char first[], const char second[]);

/*
 * Delka retezce po \n
 */

int lengthOfAStringNL(const char string[]);

/*
 * Delka retezce po \0
 */

int lengthOfAStringNull(const char string[]);

/*
 * Zjisti jestli retezec obsahuje male pismeno
 */

bool hasLowercaseLetter (const char password[]);

/*
 * Zjisti jestli retezec obsahuje velke pismeno
 */

bool hasUppercaseLetter (const char password[]);

/*
 * Zjisti jestli retezec obsahuje cislo
 */

bool hasNumber(const char password[]);

/*
 * Zjisti jestli argument n lezi v rozsahu ascii hodnoty specialnich znaku
 */

bool specialCharChecker(int n);

/*
 * Zjisti retezec obsahuje specialni znak
 */

bool hasSpecialCharacter(const char password[]);

/*
 * Nasleduji 4 funkce pro argument --stats
 */

/*
 * 1)
 * Zjisti pocet ruznych znaku napric vsemi hesly
 */

void statCharCounter (char password[], int array[], int* count);

/*
 * 2)
 * Zjisti nejkratsi heslo
 */

void shortestPassword(char password[], int* length);

/*
 * 3)
 * Zjisti prumernou delku hesla
 */

double get_average_length (const int* count, const int* total);

/*
 * 4)
 * Vytiskne statistiky
 */

void printStats (int NCHARS, int MIN, double AVG);

/*
 * Nasleduji 4 funkce pro 4 urovne bezpecnosti
 */

/*
 * Funkce pro overeni levelu 1
 */

bool checkLevelOne(char password[]);

/*
 * Funkce pro overeni levelu 2
 */

bool checkLevelTwo(char password[], long int parameter);

/*
 * Funkce pro overeni levelu 3
 */

bool checkLevelThree(const char password[], long int parameter);

/*
 * Funkce pro overeni levelu 4
 */

bool checkLevelFour(char password[], int parameter);

int main(int argc, char *argv[]) {
    char *ptr; /* Ukazatel pro strtol a jemu obdobne */

    bool stats = false; /* bool pro --stats */

    char buffer[BUFFER_SIZE]; /* pole pro hesla */

    /*
     * Promenne pro pocitani statistik
     */

    int total_passwords_length = 0;         /* Celkova delka hesel */
    int count_of_passwords = 0;             /* Celkovy pocet hesel */
    int count_of_different_letters = 0;     /* Celkovy pocet ruznych znaku napric vsemi hesly */
    int ascii[ASCII_LENGTH] = {0};                /* Pole o velikosti ascii tabulky pro ukladani 1 nebo 0 */
    int min_pw_length = MAX_LENGTH;         /* Promenna pro minimalni delku hesla */

    /*
     *  Nesleduje overovani argumentu
     */

    /* Pocet argumentu mensi nez 3 */

    if (argc < 3) {
        fprintf(stderr,"[ERROR]: Zadali jste malo argumentu. \nPocet argumentu: %i. \n", argc);
        return 1;
    }

    /* Pocet argumentu vetsi nez 4 */

    if (argc > 4) {
        fprintf(stderr,"[ERROR]: Zadali jste moc argumentu. \nPocet argumentu: %i. \n", argc);
        return 2;
    }

    /* Prvni argument mensi nez 1 nebo vetsi nez 4 nebo delka argumentu vetsi nez 1 (napr 2 a ascii znak s hodnotou 1) */

    if (strtol(argv[1], &ptr, 10) < 1 || lengthOfAStringNull(argv[1]) > 1 || strtol(argv[1], &ptr, 10) > 4) {
        fprintf(stderr, "Chybne zadani prvniho argumentu.\n");
        return 3;
    }

    /* Druhy argument mensi nez 1 */

    if (strtoll(argv[2], &ptr, 10) < 1) {
        fprintf(stderr, "Chybne zadani druheho argumentu. Argument musi byt kladne cele cislo.\n");
        return 4;
    }

    /* Pokud jsou 3 argumenty a 3. argument neni "--stats"*/

    if ((argc == 4 && !compareTwoStrings(argv[3], "--stats"))) {
        fprintf(stderr, "Chybne zadani tretiho argumentu.\n");
        return 5;
    }

    /* Pokud jsou 3 argumenty a 3. je "--stats", tak se bool stats nastavi na true */

    if (argc == 4 && compareTwoStrings(argv[3], "--stats")) {
        stats = true;
    }

    /*
     * Cyklus pro cteni hesel a praci s nemi
     */

    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        if (lengthOfAStringNull(buffer) > MAX_LENGTH) {
            fprintf(stderr, "Heslo: '%s' je moc dlouhe", buffer);       /* Overeni delky hesla */
            return 6;
        }

        if (stats) {
            shortestPassword(buffer, &min_pw_length);
            statCharCounter(buffer, ascii, &count_of_different_letters);
            total_passwords_length += lengthOfAStringNL(buffer);        /* Pocitani statistik */
            count_of_passwords++;
        }

        switch (strtol(argv[1], &ptr, 10)) {                  /* Udela s heslem to, jaky level bezpecnosti byl zadan*/
            case 1:
                if (checkLevelOne(buffer)) {
                    printf("%s", buffer);
                }
                break;
                case 2:
                    if (checkLevelOne(buffer) && checkLevelTwo(buffer, strtol(argv[2], &ptr, 10))) {
                        printf("%s", buffer);
                    }
                    break;
                    case 3:
                        if (checkLevelOne(buffer) && checkLevelTwo(buffer, strtol(argv[2], &ptr, 10)) && checkLevelThree(buffer, strtol(argv[2], &ptr, 10))) {
                            printf("%s", buffer);
                        }
                        break;
                        case 4:
                            if (checkLevelOne(buffer) && checkLevelTwo(buffer, strtol(argv[2], &ptr, 10)) && checkLevelThree(buffer, strtol(argv[2], &ptr, 10))
                            && checkLevelFour(buffer, strtol(argv[2], &ptr, 10))) {
                                printf("%s", buffer);
                            }
                            break;
                            default:
                                break;
        }
    }
    if (stats) {
        if (get_average_length(&count_of_passwords, &total_passwords_length) == -1) {
            printf("Deleni nulou!");
            return 7;
        }
        printStats(count_of_different_letters, min_pw_length, get_average_length(&count_of_passwords, &total_passwords_length));  /* Vytiskne statistiky pokud byly zadany v argumentech */
    }
    return 0;
}

/*
 * Vraci true, pokud jsou 2 retezce shodne
 */

bool compareTwoStrings(const char first[], const char second[]) {
    for (int i = 0; first[i] != '\0' || second[i] != '\0'; i++) {
        if (first[i] != second[i])
            return false;
    }
    return true;
}

/*
 * Vraci delku retezce po \n
 */

int lengthOfAStringNL(const char string[]) {
    int length = 0;
    for (int i = 0; string[i] != '\n'; i++) {
        length++;
    }
    return length;
}

/*
 *  Vraci delku retezce po \0
 */

int lengthOfAStringNull(const char string[]) {
    int length = 0;
    for (int i = 0; string[i] != '\0'; i++) {
        length++;
    }
    return length;
}

/*
 * Vraci true, pokud retezec obsahuje male pismeno
 */

bool hasLowercaseLetter (const char password[]) {
    int lowercase_count = 0;
    for (int i = 0; password[i] != '\0'; i++) {
        if (password[i] >= 'a' && password[i] <= 'z') {
            lowercase_count++;
            break;
        }
    }
    if (!lowercase_count)
        return false;
    else
        return true;
}

/*
 * Vraci true, pokud retezec obsahuje velke pismeno
 */

bool hasUppercaseLetter (const char password[]) {
    int uppercase_count = 0;
    for (int i = 0; password[i] != '\0'; i++) {
        if (password[i] >= 'A' && password[i] <= 'Z') {
            uppercase_count++;
            break;
        }
    }
    if (!uppercase_count)
        return false;
    else
        return true;
}

/*
 * Vraci true, pokud retezec obsahuje cislo
 */

bool hasNumber(const char password[]) {
    int number_count = 0;
    for (int i = 0; password[i] != '\0'; i++) {
        if (password[i] >= '0' && password[i] <= '9') {
            number_count++;
            break;
        }
    }
    if (!number_count)
        return false;
    else
        return true;
}

/*
 * Vraci true, pokud argument n lezi v rozsahu ascii hodnoty specialnich znaku
 */

bool specialCharChecker(int n) {
    if ((n >= ASCII_SPACE && n <= ASCII_SLASH) || (n >= ASCII_COLON && n <= ASCII_AT) || (n >= ASCII_BRACKET && n <= ASCII_BACKTICK) || (n >= ASCII_CURLYBRACKET && n <= ASCII_TILDE))
        return true;

    else
        return false;
}

/*
 * Vraci true, pokud retezec obsahuje specialni znak
 */

bool hasSpecialCharacter(const char password[]) {
    int special_char_count = 0;
    for (int i = 0; password[i] != '\0'; ++i) {
        if (specialCharChecker((int) password[i])) {
            special_char_count++;
            break;
        }
    }
    if (!special_char_count)
        return false;
    else
        return true;
}

/*
 * Zjisti pocet ruznych znaku napric vsemi hesly
 */

void statCharCounter (char password[], int array[], int* count) {
    int char_count = 0;
    for (int i = 0; i < lengthOfAStringNL(password); ++i) {
        if (array[(int) password[i]] != 1) {            /* podiva se, jestli je 1 na pozici ascii hodnoty daneho znaku */
            array[(int) password[i]] = 1;               /* a pokud ne, tak ji tam prida a navysi pocitadlo znaku*/
            char_count++;
        }
    }
    *count += char_count;
}

/*
 * Zjisti nejkratsi heslo abc
 */

void shortestPassword(char password[], int* length) {
    if (lengthOfAStringNL(password) < (*length)) {
        *length = lengthOfAStringNL(password);
    }
}

/*
 * Zjisti prumernou delku hesla
 */

double get_average_length (const int* count, const int* total) {
    if (*count == 0) {          /* k deleni 0 by nikdy nemelo dojit, ale jistota je jistota*/
        return -1;
    }
    else {
        double avg = ((double)(*total))/((double)(*count));
        return avg;
    }
}

/*
 * Vytiskne statistiky
 */

void printStats (int NCHARS, int MIN, double AVG) {
    printf("Statistika:\n");
    printf("Ruznych znaku: %d\n", NCHARS);
    printf("Minimalni delka: %d\n", MIN);
    printf("Prumerna delka: %0.1f\n", AVG);
}

/*
 * Funkce pro overeni levelu 1
 * Vraci true, pokud heslo obsahuje alespon 1 velke a 1 malo pismeno
 */

bool checkLevelOne(char password[]) {
    if (hasLowercaseLetter(password) && hasUppercaseLetter(password))
        return true;
    else
        return false;
}

/*
 * Funkce pro overeni levelu 2
 * Vraci true, pokud heslo obsahuje znaky z alespon parameter skupin (mala pismena, velka pismena, cisla, spec. znak)
 */

bool checkLevelTwo(char password[], long int parameter) {
    if (parameter > 4) {
        parameter = 4;
    }
    int number_of_conditions_met = 0;
    if(hasLowercaseLetter(password)) {
        number_of_conditions_met++;
    }
    if(hasUppercaseLetter(password)) {
        number_of_conditions_met++;
    }
    if(hasNumber(password)) {
        number_of_conditions_met++;
    }
    if(hasSpecialCharacter(password)) {
        number_of_conditions_met++;
    }
    if (number_of_conditions_met >= (int) parameter)
        return true;
    else
        return false;
}

/*
 * Funkce pro overeni levelu 3
 * Vraci true, pokud heslo neobsahuje sekvenci stejnych znaku delky parameter
 */

bool checkLevelThree(const char password[], long int parameter) {
    if (parameter == 1) {
        return false;
    }
    for (int i = 0; password[i] != '\0'; i++) {
        char temp = password[i];
        int count_of_consecutive_letters = 1;
        for (int j = 1; j <= parameter; ++j) {
            if (password[i + j] == '\0') {
                return true;
            }
            if (temp != password[i + j]) {
                break;
            }
            else {
                count_of_consecutive_letters++;
                if (count_of_consecutive_letters == parameter) {
                    return false;
                }
            }
        }
    }
    return 1;
}

/*
 * Funkce pro overeni levelu 4 - vraci true, pokud heslo neobsahuje 2 stejne podretezce delky parameter
 * Zjisti pocet podretezcu delky parameter, pak jimy iteruje, ulozi novy podretezec do pole substring, projde heslem a hleda stejny podretezec
 * Promenna offset zarucuje, ze podretezec v substring nebude ten se stejnou pozici jako v hesle
 */

bool checkLevelFour(char password[], int parameter) {
    int number_of_substrings = lengthOfAStringNL(password) - parameter + 1; // 6 3 = 4 123456    6*7/2
    int offset = 0, count_of_same_letters = 1;
    for (int i = 1; i <= number_of_substrings; i++) {
        char substring[parameter];
        for (int j = 0; j < parameter; j++) {
            substring[j] = password[j + offset];
        }
        for (int j = 1 + offset; password[j] != '\n'; j++) {
            if (password[j] == substring[0]) {
                for (int k = 1; k < parameter; k++) {
                    if (password[j + k] == '\0') {
                        return true;
                    }
                    if (password[j + k] == substring[k]) {
                        count_of_same_letters++;
                        if (count_of_same_letters == parameter) {
                            return false;
                        }
                    } else {
                        count_of_same_letters = 1;
                        break;
                    }
                }
            }
        }
        offset++;
    }
    return true;
}