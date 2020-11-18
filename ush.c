/* Bonafide Unix command shell written in good ol' C

MAJOR TODO:
-Add builtins
-Add support for pipes

MINOR TODO:
-Use PS1 or some other envar for shell prompt
-Recognize ctrl-L and other shortcuts (maybe not so minor?)
*/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>

#define DEFAULT_PROMPT  "C:\\> "       /* DOS Prompt just for shits */
#define TOKEN_DELIM     " \t\r\n\a"
#define ARGS_INITIAL    64
#define ARGS_LIMIT      4096

extern int errno;


char *get_input()
{
    char *ln = NULL;
    size_t ln_size = 0;
    int read_len = 0;

    read_len = getline(&ln, &ln_size, stdin);

    if (read_len == -1) {  /* ctrl-D or something has catually gone wrong */
        if (feof(stdin)) {
            puts("goodbye!");  /* former :) */
            exit(EXIT_SUCCESS);
        }

        perror("getline");  /* latter ;( */
        exit(errno);
    }
    return ln;
}


char **grow_tok_arr(int *sz_p, char **tokens)
{
    *sz_p *= 2;

    if(*sz_p > ARGS_LIMIT) {
        fprintf(stderr, "exceeded argument limit (%d)\n", ARGS_LIMIT);
        exit(EXIT_FAILURE);
    }

    char ** new_arr = realloc(tokens, *sz_p * sizeof(char*));
    assert(new_arr != NULL);

    return new_arr;
}


char **arg_tokens(char *line)
{
    int arr_size = ARGS_INITIAL;
    
    char **tokens = malloc(arr_size * sizeof(char*));
    assert(tokens != NULL);

    int tk_idx = 0;
    tokens[tk_idx] = strtok(line, TOKEN_DELIM);

    while(tokens[tk_idx] != NULL) {
        tk_idx++;

        if (tk_idx >= arr_size) {      /* Grow token array as needed */
            tokens = grow_tok_arr(&arr_size, tokens);
        }

        tokens[tk_idx] = strtok(NULL, TOKEN_DELIM);
    }

    return tokens;
}


void run_cmd(char **args)
{
    int child_pid = fork();
    
    if (child_pid < 0) {
        perror("fork");
        exit(errno);
    }

    if (child_pid == 0) {
        execvp(args[0], args);
        /* execvp should NOT return unless something has gone wrong, hence
        execution of the following line indicates an error in and of itself */
        perror(args[0]);

    } else {
        wait(NULL);
    }
}


int main(int argc, char const *argv[])
{
    for(;;) {
        fputs(DEFAULT_PROMPT, stdout);

        char **cmd_args = arg_tokens(get_input());
        
        if (!*cmd_args) {
            continue;   /* Hit Enter to your hearts content */
        }

        run_cmd(cmd_args);
    }

    /* This should never run, as a shell is inherently an infinite loop */
    return EXIT_FAILURE;
}
