#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"
/*
#ifdef __has_include
#if __has_include("sys/wait.h")
#include <sys/wait.h>
#else // Windows slop (it doesn't encode exit status)
#define WEXITSTATUS(status) (status)
#endif
#endif
*/

vector* g_filebufs; // names of files opened
vector* g_curfile;  // data (strings) of current opened file

void print_help() {
}

void open_from_filebuf(char* filename, vector* cur_file_data);

void startup_handler(int argc, char** argv) {
    --argc;
    ++argv;

    if(!(g_filebufs = vector_init(argc))) exit(1);

    for(int i = 0; i < argc; ++i) {
        if(strcmp(argv[i], "--help") == 0) {
            print_help();
            exit(0);
        }
        char tmp_filename[strlen(argv[i]) + 2];

        sprintf(tmp_filename, "%s ", argv[i]); // add a space at the end
        vector_push(g_filebufs, tmp_filename);
    }

    if(g_filebufs->size > 0) {
        for(int i = 0; i < g_filebufs->size; ++i) {
            char tmp_command[4 + 2 * strlen(g_filebufs->data[i])];
            char tmp_filename[strlen(g_filebufs->data[i]) + 1];
            strcpy(tmp_filename, g_filebufs->data[i]); // existing "filename"

            g_filebufs->data[i][strlen(g_filebufs->data[i]) - 1] = '~'; // new temp "filename~"

            sprintf(tmp_command, "cp %s%s", tmp_filename, g_filebufs->data[i]);
#ifdef DEBUG
            printf("CREATED TMP FILE WITH: %s\n", tmp_command);
#endif
            system(tmp_command);
        }
    }
    else {
        // get the number of temp(new) files created
        FILE* dir_wc_fp = popen("find . -maxdepth 1 -type f -name '[0-9]*-new-file~' | wc -w", "r");
        if(!dir_wc_fp) exit(1);
        char digitbuf[256];
        if(!fgets(digitbuf, sizeof(digitbuf), dir_wc_fp)) exit(1);
        if(pclose(dir_wc_fp) == -1) exit(1);

        // end the string where \n is (wc returns a string with \n at the end)
        digitbuf[strlen(digitbuf) - 1] = '\0';

        char tmp_filename[strlen(digitbuf) + 11];

        sprintf(tmp_filename, "%s-new-file~", digitbuf);

        char tmp_command[256 + 6];
        sprintf(tmp_command, "touch %s", tmp_filename);
#ifdef DEBUG
        printf("CREATED TMP FILE WITH: %s\n", tmp_command);
#endif
        system(tmp_command);

        vector_push(g_filebufs, tmp_filename);
    }

    // now we have g_filebufs filled with names of the created temp files

    system("echo \e[?1049h"); // save cursor pos and switch to an alternate screen buffer
    system("echo \e[H");

    open_from_filebuf(g_filebufs->data[0], g_curfile);
}

void open_from_filebuf(char* filename, vector* cur_file_data) {
    size_t filesize;

    // get the filesize
    char tmp_command[7 + strlen(filename)];
    sprintf(tmp_command, "wc -c %s", filename);
    FILE* filesize_fp = popen(tmp_command, "r");
    if(!filesize_fp) exit(1);

    char tmp_filesize[32]; //! max 30-ish digits
    if(!fgets(tmp_filesize, sizeof(tmp_filesize), filesize_fp)) exit(1);

    filesize = atoll(tmp_filesize);
    pclose(filesize_fp);

    if(!(g_curfile = vector_init(filesize))) exit(1);

    FILE* fp = fopen(filename, "r");
    /* TODO:
     * get the buffer for the string
     * and in while loop fgets the string from a file and put it in the corresponding g_curfile string
     */
}

void app_input_handler() {
    getchar();
}

int main(int argc, char** argv) {
    startup_handler(argc, argv);

    // main 'loop'
    app_input_handler();
    // end of the main 'loop'

    system("echo \e[?1049l"); // restore cursor pos and switch back to the original screen buffer
    vector_delete(g_filebufs);
    return 0;
}