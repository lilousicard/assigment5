/**
 * The reason for using a hashtable is to store the commands in hash slots by their pids.
 * This way you can lookup a command by a pid and retrieve all the info you need:
 *  command, index (this index means the line number in the text file you read),
 *  start time, and anything else you might need.
 *
 * A hashtable (as you might remember from CS146)
 * has slots and each slot contains a linked list of nodes 
 * (these are the entries that contain all the command info).
 * Thus the hashtable (see hashtab array variable below) is 
 * implemented as an array of nlists. Each array position is a 
 * slot and a linked list of nlist nodes starts at each array slot. 
 * Each array position is a hahstable slot.
 *
 * You find the hashtable slot for a pid by using a hash function, 
 * which will map the pid to a hashtable slot (array position).
 *
 * You can copy this entire code directly in your .c code. No need to have
 * many files. 
 *
 *
 * This nlist is a node, which stores one command's info in the hashtable.
 * TODO: You will need to adopt this code a little.
 *
 * The char *name and char *defn you can remove.
 * The nlist *next field is a pointer to the next node in the linked list.  
 * There is one hashtable slot in each array position, 
 * consequently there is one linked list of nlists under a hashtable slot. 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <signal.h>


struct nlist {
    struct nlist *next;
    char* command;
    struct timespec start, finish;
    int index;
    int pid;
};

#define HASHSIZE 101
static struct nlist *hashtab[HASHSIZE]; /* pointer table */

/* This is the hash function: form hash value for string s */
/* You can use a simple hash function: pid % HASHSIZE */
unsigned hash(int pid) {
    return pid%HASHSIZE;
}

/* lookup: look for s in hashtab */
/* This is traversing the linked list under a slot of the hash table. The array position to look in is returned by the hash function */
struct nlist *lookup(int pid) {
    struct nlist *np;
    for (np = hashtab[hash(pid)]; np != NULL; np = np->next)
        if (pid == np -> pid)
            return np; /* found */
    return NULL; /* not found */
}

/* insert: put (name, defn) in hashtab */
/* This insert returns a nlist node. Thus when you call insert in your main function  */
/* you will save the returned nlist node in a variable (mynode). */
/* Then you can set the starttime and finishtime from your main function: */
/* mynode->starttime = starttime; mynode->finishtime = finishtime; */
struct nlist *insert(char *command, int pid, int index) {
    struct nlist *np;
    unsigned hashval;
    //TODO change to lookup by pid. There are 2 cases:
    if ((np = lookup(pid)) == NULL) {
        /* case 1: the pid is not found, so you have to create it with malloc. Then you want to set the pid, command and index */
        np = (struct nlist *) malloc(sizeof(*np));
        if (np == NULL || (np->command = strdup(command)) == NULL)
            return NULL;
        hashval = hash(pid);
        np -> index = index;
        np -> pid = pid;
        np->next = hashtab[hashval];
        hashtab[hashval] = np;
    } else {
        free((void*) np -> command);
        if (np == NULL || (np->command = strdup(command)) == NULL)
            return NULL;
    }

    return np;
}
void copystr(char *str1, const char *str2)
{
    while(*str2)
    {
        *str1 = *str2;
        str1++;
        str2++;
    }
}

int main() {
    size_t length = 100;
    char *command = (char*) malloc(sizeof(char)*length);
    char *rcommand = (char*) malloc(sizeof(char)*length);
    int index = 0;
    while ((getline(&command, &length, stdin)) >= 0) {
        index++; // this index is the line number in the commands file.
        command[strlen(command)-1] = 0;//getting rid of new line character
        int pid = fork();

        if (pid < 0) {
            fprintf(stderr, "error forking");
            exit(2);
        } else if (pid == 0) { /*child */
            //creat char arrays and files for both .out and .err
            char fileOut[10] = {};
            char fileErr[10] = {};
            sprintf(fileOut,"%d.out",getpid());
            sprintf(fileErr,"%d.err",getpid());
            int fd, fds;
            //open files with file permissions to be open for everyone
            fd = open(fileOut, O_RDWR | O_CREAT | O_APPEND, 0777);
            fds = open(fileErr, O_RDWR | O_CREAT | O_APPEND, 0777);
            //make file FD 1 (stdout) go to a file PID.out
            dup2(fd, 1);
            //FD 2 (stderr) go to a file PID.err for pid PID
            dup2(fds, 2);
            //create char pointer for arg list and parse through user input with strtok
            char* argument_list[10];
            char *p = strtok (command, " ");
            int i = 0;
            //while loop to parse and add strings to arg list
            while (p != NULL)
            {
                argument_list[i++] = p;
                p = strtok (NULL, " ");
            }
            // NULL terminated array of char* strings
            argument_list[i] = NULL;
            //give control of current process to execvp - GeeksForGeeks
            execvp(argument_list[0], argument_list);
            exit (2);
        } else if (pid > 0) {  /* parent goes to the next node */

            struct nlist *inputEntry = insert(command, pid, index);
            clock_gettime(CLOCK_MONOTONIC, &inputEntry -> start);

            char fileOut[10] = {};
            char fileErr[10] = {};
            sprintf(fileOut,"%d.out", pid);
            sprintf(fileErr,"%d.err", pid);
            int fd, fds;
            //open files with file permissions to be open for everyone
            fd = open(fileOut, O_RDWR | O_CREAT | O_APPEND, 0777);
            fds = open(fileErr, O_RDWR | O_CREAT | O_APPEND, 0777);
            //make file FD 1 (stdout) go to a file PID.out
            dup2(fd, 1);
            //FD 2 (stderr) go to a file PID.err for pid PID
            dup2(fds, 2);
            printf("command = %s\n", inputEntry -> command);
            printf("Starting command INDEX %d: child PID %d of parent PPID %d.\n", index, pid, getpid());
            fflush(NULL);
        }
    } /*end of while loop that gets the command*/


//---------------------------
//Final while loop: waits until anything has completed,
//this will exit (wait returns -1)
//when there is no more child process. Then your parent process exits.

    int pid;
    int status;
    while ((pid = wait(&status)) >= 0) {
        if (pid > 0) {

            struct nlist *node = lookup(pid);
            clock_gettime(CLOCK_MONOTONIC, &node -> finish);
            double elapsed = 0;
	    elapsed =  node -> finish.tv_sec - node -> start.tv_sec;
            elapsed += (node -> finish.tv_nsec - node -> start.tv_nsec)/1000000000.0;
            char fileOut[10] = {};
            char fileErr[10] = {};
            sprintf(fileOut,"%d.out", node -> pid);
            sprintf(fileErr,"%d.err", node -> pid);
            int fd, fds;
            //open files with file permissions to be open for everyone
            fd = open(fileOut, O_RDWR | O_CREAT | O_APPEND, 0777);
            fds = open(fileErr, O_RDWR | O_CREAT | O_APPEND, 0777);
            //make file FD 1 (stdout) go to a file PID.out
            dup2(fd, 1);
            //FD 2 (stderr) go to a file PID.err for pid PID
            dup2(fds, 2);
            printf("finished at %ld, runtime duration = %f\n", node ->finish.tv_sec, elapsed);
            fflush(NULL);
            if(elapsed > 2.0)
            {
                strcpy(rcommand, node -> command);
            }
            if (WIFEXITED(status)) {
                fprintf(stderr, "Child %d terminated normally with exit code: %d\n", pid, WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                fprintf(stderr, "Child %d terminated abnormally with signal number: %d\n", pid, WTERMSIG(status));
            }
            free(node -> command);
            free(node);
            //restart process
	    //printf("Elapse: %f , equation result = %f",elapsed,);
            if(elapsed > 2)
            {
                index = 1; // this index is the line number in the commands file.
                pid = fork();

                if (pid < 0) {
                    fprintf(stderr, "error forking");
                    exit(2);
                } else if (pid == 0) { /*child */
                    //creat char arrays and files for both .out and .err
                    char fileOut[10] = {};
                    char fileErr[10] = {};
                    sprintf(fileOut,"%d.out",getpid());
                    sprintf(fileErr,"%d.err",getpid());
                    int fd, fds;
                    //open files with file permissions to be open for everyone
                    fd = open(fileOut, O_RDWR | O_CREAT | O_APPEND, 0777);
                    fds = open(fileErr, O_RDWR | O_CREAT | O_APPEND, 0777);
                    //make file FD 1 (stdout) go to a file PID.out
                    dup2(fd, 1);
                    //FD 2 (stderr) go to a file PID.err for pid PID
                    dup2(fds, 2);
                    //"flush" unwritten data to buffer
                    fflush(NULL);
                    //create char pointer for arg list and parse through user input with strtok
                    char* argument_list[10];
                    char *p = strtok (rcommand, " ");
                    int i = 0;
                    //while loop to parse and add strings to arg list
                    while (p != NULL)
                    {
                        argument_list[i++] = p;
                        p = strtok (NULL, " ");
                    }
                    // NULL terminated array of char* strings
                    argument_list[i] = NULL;
                    //give control of current process to execvp - GeeksForGeeks
                    execvp(argument_list[0], argument_list);
                    exit (2);
                } else if (pid > 0) {  /* parent goes to the next node */

                    struct nlist *inputEntry = insert(rcommand, pid, index);
                    clock_gettime(CLOCK_MONOTONIC, &inputEntry -> start);

                    char fileOut[10] = {};
                    char fileErr[10] = {};
                    sprintf(fileOut,"%d.out", pid);
                    sprintf(fileErr,"%d.err", pid);
                    int fd, fds;
                    //open files with file permissions to be open for everyone
                    fd = open(fileOut, O_RDWR | O_CREAT | O_APPEND, 0777);
                    fds = open(fileErr, O_RDWR | O_CREAT | O_APPEND, 0777);
                    //make file FD 1 (stdout) go to a file PID.out
                    dup2(fd, 1);
                    //FD 2 (stderr) go to a file PID.err for pid PID
                    dup2(fds, 2);
                    printf("RESTARTING\n");
                    printf("rcommand = %s\n", rcommand);
                    fprintf(stderr, "RESTARTING\n");
                    printf("Starting command INDEX %d: child PID %d of parent PPID %d.\n", index, pid, getpid());
                    fflush(NULL);
                }
            }
        }
    }
    free(command);
    free(rcommand);
    return 0;
}
