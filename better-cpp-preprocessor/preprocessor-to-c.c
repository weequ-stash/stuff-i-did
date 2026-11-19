#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define PATH_TO_INCLUDE_LIST "includes" //TODO: make it depend on path to the compiler
#define PATH_TO_RULES "bits/"

bool ONLY_TOKENS = 0;

void print_help() {
    printf("DESCRIPTION:\n");
    printf("    preprocessor/compiler for \"better c\"\n\n");
    printf("    rewrites \"better c\" code to c\n\n");

    printf("USAGE:\n");
    printf("    ./bcc [options]\n\n");

    printf("FLAGS:\n");
    printf("--help             Shows this message\n");
    printf("-T, --tokens-only  Print only c language tokens of input\n\n");

    printf("NOTE:\n");
    printf("    bcc uses c compiler internally so all the options\n");
    printf("    of c compiler used on Your system are supported\n");
}

void preprocess_file(char* filename);

void arg_handler(int argc, char** argv) {
    int args_to_gcc_idx = 0;
    int args_to_gcc[argc];

    --argc;
    ++argv;
    for(int i = 0; i < argc; ++i) {
        if(argv[i][0] == '-') {
            if(strcmp(argv[i], "--help") == 0) {
                print_help();
                exit(0);
            }
            else if(strcmp(argv[i], "-T") == 0 || strcmp(argv[i], "--tokens-only")) {
                ONLY_TOKENS = true;
            }
            else if(strlen(argv[i]) > 1 && (argv[i][1] == 'B' || argv[i][1] == 'o' || argv[i][1] == 'x')) {
                if(i == argc) {
                    fprintf(stderr, "%s: Not enough parameters specified.", argv[i]);
                    exit(1);
                }
                args_to_gcc[args_to_gcc_idx] = i;
                args_to_gcc[args_to_gcc_idx + 1] = i + 1;

                ++i;
                args_to_gcc_idx += 2;
            }

            continue;
        }

        preprocess_file(argv[i]);
    }
}

#define APPEND_CHAR(c, s, s_cap)            \
    do {                                    \
        if(strlen(s) + 1 == s_cap) {        \
            s_cap *= 2;                     \
            char* tmp = realloc(s, s_cap);  \
            if(!tmp) {                      \
                perror("tokenizer");        \
                exit(1);                    \
            }                               \
            s = tmp;                        \
        }                                   \
        strncat(s, &c, 1);                  \
    } while(0)

char* get_token(FILE* fp) {
    if(!fp) return NULL;

    char c = fgetc(fp);
    if(c == EOF) return NULL;

    for(; c != EOF && isspace(c); c = fgetc(fp));

    int s_cap = 100;
    char* s = malloc(s_cap);
    if(!s) {
        perror("tokenizer");
        exit(1);
    }
    s[0] = '\0';

    if(c == '"') {
        APPEND_CHAR(c, s, s_cap);
        c = fgetc(fp);
        for(; c != EOF && c != '"'; c = fgetc(fp)) {
            APPEND_CHAR(c, s, s_cap);

        }
        APPEND_CHAR(c, s, s_cap);
    }
    else if(c == '\'') {
        APPEND_CHAR(c, s, s_cap);
        c = fgetc(fp);
        for(; c != EOF && c != '\''; c = fgetc(fp)) {
            APPEND_CHAR(c, s, s_cap);
        }
        APPEND_CHAR(c, s, s_cap);
    }
    else if(c == '(') {
        APPEND_CHAR(c, s, s_cap);
    }
    else if(c == '[') {
        APPEND_CHAR(c, s, s_cap);
    }
    else if(c == '{') {
        APPEND_CHAR(c, s, s_cap);
    }
    else if(c == ')') {
        APPEND_CHAR(c, s, s_cap);
    }
    else if(c == ']') {
        APPEND_CHAR(c, s, s_cap);
    }
    else if(c == '}') {
        APPEND_CHAR(c, s, s_cap);
    }
    else if(c == ';') {
        APPEND_CHAR(c, s, s_cap);
    }
    else if(c == ':') {
        APPEND_CHAR(c, s, s_cap);
    }
    else if(c == '?') {
        APPEND_CHAR(c, s, s_cap);
    }
    else if(c == '.') {
        APPEND_CHAR(c, s, s_cap);
    }
    else if(c == ',') {
        APPEND_CHAR(c, s, s_cap);
    }
    else if(c == '=') {
        for(; c != EOF && (c == '='); c = fgetc(fp)) {
            APPEND_CHAR(c, s, s_cap);
        }
        ungetc(c, fp); // go back one char
    }
    else if(c == '-') {
        for(; c != EOF && (c == '-' || c == '=' || c == '>'); c = fgetc(fp)) {
            APPEND_CHAR(c, s, s_cap);
        }
        ungetc(c, fp); // go back one char
    }
    else if(c == '+') {
        for(; c != EOF && (c == '+' || c == '='); c = fgetc(fp)) {
            APPEND_CHAR(c, s, s_cap);
        }
        ungetc(c, fp); // go back one char
    }
    else if(c == '/') {
        for(; c != EOF && (c == '/' || c == '='); c = fgetc(fp)) { // Note: there will be no comments anyway
            APPEND_CHAR(c, s, s_cap);
        }
        ungetc(c, fp); // go back one char
    }
    else if(c == '!') {
        APPEND_CHAR(c, s, s_cap);
        c = fgetc(fp);
        for(; c != EOF && (c == '='); c = fgetc(fp)) {
            APPEND_CHAR(c, s, s_cap);
        }
        ungetc(c, fp); // go back one char
    }
    else if(c == '%') {
        APPEND_CHAR(c, s, s_cap);
        c = fgetc(fp);
        for(; c != EOF && (c == '='); c = fgetc(fp)) {
            APPEND_CHAR(c, s, s_cap);
        }
        ungetc(c, fp); // go back one char
    }
    else if(c == '^') {
        APPEND_CHAR(c, s, s_cap);
        c = fgetc(fp);
        for(; c != EOF && (c == '='); c = fgetc(fp)) {
            APPEND_CHAR(c, s, s_cap);
        }
        ungetc(c, fp); // go back one char
    }
    else if(c == '&') {
        for(; c != EOF && (c == '&' || c == '='); c = fgetc(fp)) {
            APPEND_CHAR(c, s, s_cap);
        }
        ungetc(c, fp); // go back one char
    }
    else if(c == '*') {
        APPEND_CHAR(c, s, s_cap);
        c = fgetc(fp);
        for(; c != EOF && (c == '='); c = fgetc(fp)) {
            APPEND_CHAR(c, s, s_cap);
        }
        ungetc(c, fp); // go back one char
    }
    else if(c == '|') {
        for(; c != EOF && (c == '|' || c == '='); c = fgetc(fp)) {
            APPEND_CHAR(c, s, s_cap);
        }
        ungetc(c, fp); // go back one char
    }
    else if(c == '<') {
        for(; c != EOF && (c == '<' || c == '='); c = fgetc(fp)) {
            APPEND_CHAR(c, s, s_cap);
        }
        ungetc(c, fp); // go back one char
    }
    else if(c == '>') {
        for(; c != EOF && (c == '>' || c == '='); c = fgetc(fp)) {
            APPEND_CHAR(c, s, s_cap);
        }
        ungetc(c, fp); // go back one char
    }
    else if(isalnum(c) || c == '_') {
        for(; c != EOF && (isalnum(c) || c == '_'); c = fgetc(fp)) {
            APPEND_CHAR(c, s, s_cap);
        }
        ungetc(c, fp); // go back one char
    }
    else {
        for(; c != EOF && !isalnum(c) && !isspace(c) && c != '_'; c = fgetc(fp)) {
            APPEND_CHAR(c, s, s_cap);
        }
        ungetc(c, fp); // go back one char
    }
    return s;
}

void prepare_file(char* filename) {
    // get rid of the comments and macros
    char* tmp_command = malloc(18 + 1 + 2 * strlen(filename));
    sprintf(tmp_command, "gcc -x c -E -o %s.e %s", filename, filename);
    system(tmp_command);
    sprintf(tmp_command, "%s.e", filename); // filename.e
    FILE* fp = fopen(tmp_command, "r");
    if(!fp) {
        perror(tmp_command);
        exit(1);
    }

    // skip some directives (lines starting with #)
    char c = fgetc(fp);
    while(true) {
        // skip potential whitespaces at the start of the line
        for(; c != EOF && isspace(c); c = fgetc(fp));
        if(c == EOF) break;

        // skip the directive
        if(c == '#') for(; c != EOF && c != '\n'; c = fgetc(fp));
        else break;
    }
    ungetc(c, fp); // go back one char

    char* new_file = malloc(3 + 1 + strlen(filename));
    char* old_file = strdup(tmp_command); // filename.e
    sprintf(new_file, "%s2", old_file); // filename.e2
    sprintf(tmp_command, "mv %s %s", new_file, old_file); // prepare command for: delete filename.e2
    FILE* new_fp = fopen(new_file, "w");
    if(!new_fp) {
        perror(new_file);
        exit(1);
    }

    // copy the rest
    while(true) {
        c = fgetc(fp);
        if(c == EOF) break;

        fputc(c, new_fp);
    }

    fclose(new_fp);
    fclose(fp);

    system(tmp_command);

    free(tmp_command);
    free(new_file);
    free(old_file);
}

void append_lines_file(char* filename, int n, int* lines_idx, char** new_lines) { // idx - 0-based line number to append after
    // assuming indexes are sorted (they are normally)
    // assuming every line ends with \n

    if(n == 0) return;

    FILE* fp = fopen(filename, "r");
    if(!fp) {
        perror(filename);
        exit(1);
    }
    char* tmp_file = malloc(1 + 1 + strlen(filename));
    sprintf(tmp_file, "%s2", filename);
    FILE* tmp_fp = fopen(tmp_file, "w");
    if(!tmp_fp) {
        perror(filename);
        exit(1);
    }

    char c = fgetc(fp);
    for(int i = 0; i <= lines_idx[n - 1]; ++i) {
        for(; c != EOF && c != '\n'; c = fgetc(fp)) fputc(c, tmp_fp);
        fputc(c, tmp_fp);

        // append new lines after
        for(int j = 0; j < n; ++j) {
            if(lines_idx[j] != i) continue;
            fputs(new_lines[j], tmp_fp);
        }
    }

    fclose(fp);
    fclose(tmp_fp);

    char* tmp_command = malloc(4 + 1 + strlen(filename) + strlen(tmp_file));
    sprintf(tmp_command, "mv %s %s", tmp_file, filename);

    free(tmp_command);
    free(tmp_file);
}

void replace_lines_file(char* filename, int n, int* lines_idx, char** new_lines) {
    // assuming indexes are sorted (they are normally)
    // assuming every line ends with \n

    if(n == 0) return;

    FILE* fp = fopen(filename, "r");
    if(!fp) {
        perror(filename);
        exit(1);
    }
    char* tmp_file = malloc(1 + 1 + strlen(filename));
    sprintf(tmp_file, "%s2", filename);
    FILE* tmp_fp = fopen(tmp_file, "w");
    if(!tmp_fp) {
        perror(filename);
        exit(1);
    }

    char c = fgetc(fp);
    bool changed = 0;
    for(int i = 0; i <= lines_idx[n - 1]; ++i) {
        changed = 0;
        for(int j = n - 1; j >= 0; --j) { // reversed in case of repeating idx
            if(lines_idx[j] == i) {
                changed = 1;
                fputs(new_lines[j], tmp_fp);
                break;
            }
        }
        if(!changed) {
            for(; c != EOF && c != '\n'; c = fgetc(fp)) fputc(c, tmp_fp);
            fputc(c, tmp_fp);
        }
    }

    fclose(fp);
    fclose(tmp_fp);

    char* tmp_command = malloc(4 + 1 + strlen(filename) + strlen(tmp_file));
    sprintf(tmp_command, "mv %s %s", tmp_file, filename);

    free(tmp_command);
    free(tmp_file);
}

//bool is_valid_rule_keyword(char* token) {
//    if(token[0] == '"') return true;
//    if(strlen(token) > 1 && token[0] == '\\' && (token[1] == '{' || token[1] == '}')) return true;
//    if(strcmp(token, "keyword") == 0 ||
//       strcmp(token, "literal") == 0 ||
//       strcmp(token, "operator") == 0 ||
//       strcmp(token, "...") == 0 ||
//    ) return true;
//}

void handle_rule(char*** tokens, int big_size, int small_size) {
}

void generate_code_from_rules(char* filename, char* rulename) {
    // will buffer the changes and update the file
    char* tmp_rulepath = malloc(1 + strlen(PATH_TO_RULES) + strlen(rulename));
    sprintf(tmp_rulepath, PATH_TO_RULES"%s", rulename);
    FILE* fp = fopen(tmp_rulepath, "r");
    if(!fp) {
        perror(tmp_rulepath);
        exit(1);
    }
    free(tmp_rulepath);

    char* cur_token;
    char*** tokens = malloc(4 * sizeof(char**));
    for(int i = 0; i < 4; ++i) tokens[i] = malloc(4 * sizeof(char*));
    int big_cap = 4, big_idx = 0;
    int* small_cap = malloc(4 * sizeof(char*));
    for(int i = 0; i < 4; ++i) small_cap[i] = 4;
    int* small_idx = malloc(4 * sizeof(char*));
    for(int i = 0; i < 4; ++i) small_idx[i] = 0;

    cur_token = get_token(fp);
    if(!cur_token || cur_token[0] == '}' || cur_token[0] == ';' || cur_token[0] == '=') {
        fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
        exit(1);
    }
    while(!cur_token) {
        // left side
        while(true) {
            while(cur_token && cur_token[0] != '{') {
                cur_token = get_token(fp);
                if(!cur_token || cur_token[0] == ';') {
                    fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
                    exit(1);
                }

                if(cur_token[0] == '=') break;
            }
            if(!cur_token) {
                fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
                exit(1);
            }

            cur_token = get_token(fp); //should be {
            if(!cur_token) {
                fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
                exit(1);
            }

            cur_token = get_token(fp); // first after {
            if(!cur_token || cur_token[0] == '{' || cur_token[0] == '}' || cur_token[0] == ';' || cur_token[0] == '=') {
                fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
                exit(1);
            }

            while(cur_token && cur_token[0] != '}') {
                tokens[big_idx][small_idx[big_idx]] = strdup(cur_token);

                ++small_idx[big_idx];
                if(small_idx[big_idx] == small_cap[big_idx]) {
                    small_cap[big_idx] *= 2;
                    char** tmp = realloc(tokens[big_idx], small_cap[big_idx]);
                    if(!tmp) {
                        perror(rulename);
                        exit(1);
                    }
                    tokens[big_idx] = tmp;
                }

                cur_token = get_token(fp);
                if(!cur_token || cur_token[0] == '{' || cur_token[0] == ';' || cur_token[0] == '=') {
                    fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
                    exit(1);
                }
            }
            if(!cur_token) {
                fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
                exit(1);
            }

            cur_token = get_token(fp); // should be }
            if(!cur_token) {
                fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
                exit(1);
            }

            cur_token = get_token(fp); // first after }
            if(!cur_token) {
                fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
                exit(1);
            }

            if(cur_token[0] == ';') {
                fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
                exit(1);
            }

            if(cur_token[0] == '=') break;

            ++big_idx;
            if(big_idx == big_cap) {
                big_cap *= 2;
                char*** tmp = realloc(tokens, big_cap);
                if(!tmp) {
                    perror(rulename);
                    exit(1);
                }
                tokens = tmp;
            }
        }
        // '='
        tokens[big_idx][small_idx[big_idx]] = strdup(cur_token); // should be tokens[big_idx][0]

        ++small_idx[big_idx];
        // no need to allocate new space in tokens[big_idx] since there will only be '='
        ++big_idx;
        if(big_idx == big_cap) {
            big_cap *= 2;
            char*** tmp = realloc(tokens, big_cap);
            if(!tmp) {
                perror(rulename);
                exit(1);
            }
            tokens = tmp;
        }

        // right side
        cur_token = get_token(fp);
        if(!cur_token || cur_token[0] == ';' || cur_token[0] == '=') {
            fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
            exit(1);
        }
        while(true) {
            while(cur_token && cur_token[0] != '{') {
                cur_token = get_token(fp);
                if(!cur_token || cur_token[0] == '=') {
                    fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
                    exit(1);
                }

                if(cur_token[0] == ';') break;
            }
            if(!cur_token) {
                fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
                exit(1);
            }

            cur_token = get_token(fp); //should be {
            if(!cur_token) {
                fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
                exit(1);
            }

            cur_token = get_token(fp); // first after {
            if(!cur_token || cur_token[0] == '{' || cur_token[0] == '}' || cur_token[0] == ';' || cur_token[0] == '=') {
                fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
                exit(1);
            }

            while(cur_token && cur_token[0] != '}') {
                tokens[big_idx][small_idx[big_idx]] = strdup(cur_token);

                ++small_idx[big_idx];
                if(small_idx[big_idx] == small_cap[big_idx]) {
                    small_cap[big_idx] *= 2;
                    char** tmp = realloc(tokens[big_idx], small_cap[big_idx]);
                    if(!tmp) {
                        perror(rulename);
                        exit(1);
                    }
                    tokens[big_idx] = tmp;
                }

                cur_token = get_token(fp);
                if(!cur_token || cur_token[0] == '{' || cur_token[0] == ';' || cur_token[0] == '=') {
                    fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
                    exit(1);
                }
            }
            if(!cur_token) {
                fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
                exit(1);
            }

            cur_token = get_token(fp); // should be }
            if(!cur_token) {
                fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
                exit(1);
            }

            cur_token = get_token(fp); // first after }
            if(!cur_token) {
                fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
                exit(1);
            }

            ++big_idx;
            if(big_idx == big_cap) {
                big_cap *= 2;
                char*** tmp = realloc(tokens, big_cap);
                if(!tmp) {
                    perror(rulename);
                    exit(1);
                }
                tokens = tmp;
            }

            if(cur_token[0] == '=') {
                fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
                exit(1);
            }

            if(cur_token[0] == ';') break;
        }

        // ';'
        tokens[big_idx][small_idx[big_idx]] = strdup(cur_token); // should be tokens[big_idx][0]

        ++small_idx[big_idx];
        ++big_idx;
        handle_rule(tokens, big_idx, small_idx);

        // re-malloc tokens
        for(int i = 0; i < big_cap; ++i) {
            for(int j = 0; j < small_cap[i]; ++j) {
                free(tokens[i][j]);
            }
            free(tokens[i]);
        }
        free(tokens);

        tokens = malloc(4 * sizeof(char**));
        for(int i = 0; i < 4; ++i) tokens[i] = malloc(4 * sizeof(char*));
        big_cap = 4, big_idx = 0;
        small_cap = malloc(4 * sizeof(char*));
        for(int i = 0; i < 4; ++i) small_cap[i] = 4;
        small_idx = malloc(4 * sizeof(char*));
        for(int i = 0; i < 4; ++i) small_idx[i] = 0;

        // pre-left check
        cur_token = get_token(fp);
        if(!cur_token || cur_token[0] == '}' || cur_token[0] == ';' || cur_token[0] == '=') {
            fprintf(stderr, "%s: malformed rules found", rulename); //TODO: better error message
            exit(1);
        }
    }

    for(int i = 0; i < big_cap; ++i) {
        for(int j = 0; j < small_cap[i]; ++j) {
            free(tokens[i][j]);
        }
        free(tokens[i]);
    }
    free(tokens);

    free(small_cap);
    free(small_idx);

    if(cur_token) free(cur_token);
    fclose(fp);
}

void include_file(char* file, char* destination) {
//    printf("%s\n", file);
}

int needs_include(char* token) {
    FILE* include_fp = fopen(PATH_TO_INCLUDE_LIST, "r");
    if(!include_fp) {
        perror("include list");
        exit(1);
    }

    int idx = 0;
    char* cur;
    while(true) {
        cur = get_token(include_fp);
        if(!cur) break;
        if(!isalnum(cur[0])) continue;

        if(strcmp(cur, token) == 0) return idx;
        ++idx;
    }

    if(cur) free(cur);
    fclose(include_fp);

    return -1;
}

void parse_includes(char* filename) {
    char* new_filename = malloc(2 + 1 + strlen(filename));
    sprintf(new_filename, "%s.e", filename);

    FILE* fp = fopen(new_filename, "r");
    if(!fp) {
        perror(new_filename);
        exit(1);
    }
    free(new_filename);

    FILE* wc = popen("wc -w " PATH_TO_INCLUDE_LIST, "r");
    if(!wc) {
        perror("include list");
        exit(1);
    }
    char digitbuf[256];
    fgets(digitbuf, sizeof(digitbuf), wc);
    pclose(wc);

    bool* include_idx = malloc(atoi(digitbuf) * sizeof(bool));

    char* cur_token;
    int idx = 0;
    while(true) {
        cur_token = get_token(fp);
        if(!cur_token) break;
        if(!isalnum(cur_token[0])) continue;

        idx = needs_include(cur_token);
        if(idx != -1) include_idx[idx] = 1;
    }
    fclose(fp);

    FILE* include_fp = fopen(PATH_TO_INCLUDE_LIST, "r");
    if(!include_fp) {
        perror("include list");
        exit(1);
    }

    // include found

    idx = 0;
    while(true) {
        cur_token = get_token(include_fp);
        if(!cur_token) break;
        if(!isalnum(cur_token[0])) continue;

        if(include_idx[idx]) include_file(cur_token, new_filename);
        ++idx;
    }

    fclose(include_fp);

    if(cur_token) free(cur_token);
    free(include_idx);
}

void preprocess_file(char* filename) {
    // parsing all the necessary includes (looking for data type keywords)
    prepare_file(filename);
    if(ONLY_TOKENS) return;

    parse_includes(filename);
}

int main(int argc, char** argv) {
    arg_handler(argc, argv);

    return 0;
}