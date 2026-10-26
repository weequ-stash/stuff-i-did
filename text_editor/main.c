#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "vector.h"

#ifdef __has_include
#if __has_include("sys/select.h")
#include <sys/select.h>
#else // Windows slop
#include <conio.h>
#define WINDOWS_SLOP
#endif
#endif


int WINDOW_ROWS, WINDOW_COLS;

vector* g_filebufs; // names of files opened
vector* g_curfile;  // data (strings) of current opened file

size_t g_curfile_size;

void print_help() {
}

size_t get_filesize_bytes(char* filename) {
    size_t filesize;

    char tmp_command[7 + strlen(filename)];
    sprintf(tmp_command, "wc -c %s", filename);
    FILE* filesize_fp = popen(tmp_command, "r");
    if(!filesize_fp) exit(1);

    char tmp_filesize[32]; //! max 30-ish digits
    if(!fgets(tmp_filesize, sizeof(tmp_filesize), filesize_fp)) exit(1);

    filesize = atoll(tmp_filesize);
    pclose(filesize_fp);

    return filesize;
}

size_t get_filesize_lines(char* filename) {
    FILE* fp = fopen(filename, "r");

    size_t cnt = 0;
    for(char tmp_c = fgetc(fp); tmp_c != EOF; tmp_c = fgetc(fp)){
        if(tmp_c == '\n') ++cnt;
    }
    fclose(fp);

    return cnt;
}

void print_lines(char* filename, int start, int end) { // 0-indexed!
    FILE* fp = fopen(filename, "r");
    int cnt = 0;
    for(char tmp_c = fgetc(fp); tmp_c != EOF && cnt < start; tmp_c = fgetc(fp)){
        if(tmp_c == '\n') ++cnt;
    }
    for(char tmp_c = fgetc(fp); tmp_c != EOF && cnt < end; tmp_c = fgetc(fp)){
        if(tmp_c == '\n') ++cnt;
        fputc(tmp_c, stdout);
    }
    fclose(fp);
}

void startup_handler(int argc, char** argv) {
    char tmp_dims[32];

    FILE* dims_fp = popen("tput lines", "r");
    if(!dims_fp) exit(1);
    if(!fgets(tmp_dims, sizeof(tmp_dims), dims_fp)) exit(1);
    WINDOW_ROWS = atoll(tmp_dims);
    pclose(dims_fp);

    dims_fp = popen("tput cols", "r");
    if(!dims_fp) exit(1);
    if(!fgets(tmp_dims, sizeof(tmp_dims), dims_fp)) exit(1);
    WINDOW_COLS = atoll(tmp_dims);
    pclose(dims_fp);

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

    g_curfile_size = get_filesize_lines(g_filebufs->data[0]);
    print_lines(g_filebufs->data[0], 0, WINDOW_ROWS);
}

void app_input_handler() {
    int cursor_y = 0;

    char key;
#ifndef WINDOWS_SLOP
    fd_set set;
    struct timeval timeout;
#endif
    while(1) {
        // read key
#ifdef WINDOWS_SLOP
        key = _getch();
#else
        FD_ZERO(&set);
        FD_SET(0, &set);
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // 0.1 second timeout

        if(select(1, &set, NULL, NULL, &timeout) > 0) {
            read(0, &key, 1);
        }
#endif
        // interpret key
        // TODO: make so that the text updates smoothly
        if(key == 17) break; // ^Q == 17
        if(key == 72) {
            cursor_y = cursor_y - 1 < 0 ? cursor_y : cursor_y - 1;
            //system("echo \e[2J");
            //usleep(200);
            print_lines(g_filebufs->data[0], cursor_y, cursor_y + WINDOW_ROWS);
        }
        if(key == 80) {
            cursor_y = cursor_y + 1 > g_curfile_size - WINDOW_ROWS ? cursor_y : cursor_y + 1;
            //system("echo \e[2J");
            //usleep(200);
            print_lines(g_filebufs->data[0], cursor_y, cursor_y + WINDOW_ROWS);
        }
    }
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