#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define MAX_ARG_SIZE 128
#define ARG_MAX 32

typedef enum { NONE, L, R, RR } REDIR;

struct cmd {
    char* name;
    REDIR redir;
    char* refile;
    struct cmd* pipedcmd;
    int argnum;
    char* args[ARG_MAX];
};

void processcmd(struct cmd* cmd);

struct cmd* makecmd() {
    struct cmd* cmd;
    
    cmd = malloc(sizeof(*cmd));
    memset(cmd, 0, sizeof(*cmd));
    cmd->redir = NONE;
    
    return cmd;
}

// Returns index if element in list, else -1
int in(const char* el, const char* l[], uint len) {
    for (int i = 0; i < len; i++)
        if (strcmp(el, l[i]) == 0) 
            return i;
    return -1;
}

// Returns next arg from stdin
// Status:
//   1 -> success
//   0 -> success/end of line
//  -1 -> error
//   2 -> sequence of arg (;)
char* getarg(int* status) {
    char* arg = malloc(MAX_ARG_SIZE), * argp = arg;
    int len = 0;
    char c;

    *status = 1;

    if(arg == (char *)0) {
        fprintf(2, "Failed to malloc\n");
        *status = -1;
        return (char *)0;
    }

    for(;;) {
        int n = read(0, &c, 1);
        if(n < 0)
            break;

        if(c == ' ') {
            if (len == 0)
                continue;
            else
                break;   
        }     
        else if(c == '\n') {
            *status = 0;
            break;
        }
        else if(c == ';') {
            *status = 2;
            break;
        }

        if(++len == MAX_ARG_SIZE - 1) {
            break;
        }

        *arg++ = c;
    }
    *arg = '\0';
    return argp;
}

// Returns a command struct containing args
// Optional redirect
// Optional pipe to next command
int getcmd(struct cmd* cmd) {
    int argstatus = 0;
    int status = 0;

    char *arg;
    const char* redirsymbols[] = {"<", ">", ">>"};

    cmd->redir = NONE;
    cmd->argnum = 0;

    do {
        arg = getarg(&argstatus);

        int ind;
        // Zero length arg
        if (strlen(arg) <= 0) {
            status = 2;
            break;
        }
        // Redirect
        else if ((ind = in(arg, redirsymbols, 3)) > -1) {
            cmd->refile = getarg(&argstatus);

            switch(ind) {
                case L - 1: cmd->redir = L; break;
                case R - 1: cmd->redir = R; break;
                case RR - 1: cmd->redir = RR; break;
            }

            break;
        }
        // Pipe
        else if (strcmp(arg, "|") == 0 && argstatus == 1) {
            cmd->pipedcmd = makecmd();
            argstatus = getcmd(cmd->pipedcmd);
            if (argstatus < 0)
                return -1;
            break;
        };

        cmd->args[cmd->argnum++] = arg;
    } while (argstatus == 1);

    cmd->name = cmd->args[0];

    if (argstatus < 0)
        return -1;
    else if(argstatus == 2)
        return 1;
    return status;
}

// Redirect <
int redirectin(const char *file) {
    close(0);
    if (open(file, O_RDONLY) == 0) return 0;
    return -1;
}

// Redirect >
int redirectout(const char *file) {
    close(1);
    if (open(file, O_WRONLY | O_CREATE | O_TRUNC) == 1) return 0;
    return -1;
}

// Redirect >>
int redirectoutapp(const char *file) {
    close(1);
    if (open(file, O_WRONLY | O_CREATE) == 1) return 0;
    return -1;
}

// Decide redirect and execute
int redirect(struct cmd* cmd) {
    int status = 0;
    switch (cmd->redir) {
        case L: status = redirectin(cmd->refile); break;
        case R: status = redirectout(cmd->refile); break;
        case RR: status = redirectoutapp(cmd->refile); break;
        case NONE: break;
    }
    return status;
}

// Execute command using fork
void execcmd(struct cmd* cmd) {    
    if (redirect(cmd) < 0)
        printf("Failed to redirect to %s\n", cmd->refile);
    if (exec(cmd->name, cmd->args) < 0)
        printf("Failed to execute %s\n", cmd->name);
    exit(0);
}

// Execute command with pipe to associated command
void execpipe(struct cmd* cmd) {
    int p[2];
    if(pipe(p) < 0) {
        printf("Failed to pipe\n");
        return;
    }

    if(fork() == 0) {
        close(1);
        dup(p[1]);
        close(p[0]);
        close(p[1]);
        execcmd(cmd);
    }
    if(fork() == 0) {
        close(0);
        dup(p[0]);
        close(p[0]);
        close(p[1]);
        processcmd(cmd->pipedcmd);
        exit(0);
    }

    close(p[0]);
    close(p[1]);
    wait((int *) 0);
    wait((int *) 0);
}

// Process command setting up pipes and forks, then execute
void processcmd(struct cmd* cmd) {
    if (cmd->pipedcmd != (struct cmd*) 0)
        execpipe(cmd);
    else {
        if (fork() == 0)
            execcmd(cmd);
        else
            wait((int *) 0);
    }
}

// Free and nullify cmd elements, including args and sequence of pipes
void cleancmd(struct cmd* cmd) {
    for (int i = 0; i < cmd->argnum; i++) {
        free(cmd->args[i]);
        cmd->args[i] = (char *) 0;
    }

    if (cmd->redir != NONE)
        free(cmd->refile);
    cmd->refile = (char *) 0;
    cmd->redir = NONE;
    cmd->name = (char *) 0;

    if (cmd->pipedcmd != (struct cmd*) 0) {
        cleancmd(cmd->pipedcmd);
        free(cmd->pipedcmd);
        cmd->pipedcmd = (struct cmd*) 0;
    }

    cmd->argnum = 0;
}


int main (int argc, char *argv[]) 
{
    struct cmd cmd;
    int status = 0;

    for (;;) {
        if (status == 0)
            printf(">>>");

        // Get command
        status = getcmd(&cmd);
        if (status < 0) {
            printf("Failed to read command\n");
            break;
        }
        else if (status == 2) {
            status = 0;
            cleancmd(&cmd);
            continue;
        }

        // Execute command
        if (strcmp(cmd.name, "cd") == 0) {
            if (chdir(cmd.args[1]) < 0)
                printf("Failed to cd\n");
        }
        else if (strcmp(cmd.name, "exit") == 0) {
            exit(0);
        }
        else
            processcmd(&cmd);
        
        cleancmd(&cmd);
    }

    exit(0);
}