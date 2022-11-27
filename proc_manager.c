/**
 * Description: This module takes one or more lines of input and executes them simultaneously. The result is display
 * in various file.
 * Author name: Lilou Sicard-Noel and Kobern Dare
 * Author email: lilou.sicard-noel@sjsu.edu && kobern.dare@sjsu.edu
 * Last modified date: 11/15/2022
 * Creation date: 11/04/2022
 * GitHub Repo : https://github.com/lilousicard/assigment5
 **/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <signal.h>

struct nlist { /* table entry: */
    struct nlist *next; /* next entry in chain */
    char *command;
    struct timespec start, finish;
    int index;
    int pid;
};
#define HASHSIZE 101
static struct nlist *hashtab[HASHSIZE]; /* pointer table */

/* This is the hash function: form hash value for string s */
/* You can use a simple hash function: pid % HASHSIZE */
int hash(int pid) {
    return pid % HASHSIZE;
}
/* lookup: look for s in hashtab */
/* This is traversing the linked list under a slot of the hash
table. The array position to look in is returned by the hash
function */
struct nlist *lookup(int pid) {
    struct nlist *np;
    for (np = hashtab[hash(pid)]; np != NULL; np = np->next)
        if (pid == np->pid)
            return np; /* found */
    return NULL; /* not found */
}
/**
 * 
 * insert: put (name, defn) in hashtab 
 * This insert returns a nlist node. Thus when you call insert in
 *your main function  
 * you will save the returned nlist node in a variable (mynode).
 *
 * Then you can set the starttime and finishtime from your main
 *function: 
 * mynode->starttime = starttime; mynode->finishtime = finishtime;
**/
struct nlist *insert(char *command, int pid, int index) {
    struct nlist *np;
    unsigned hashval;
    if ((np = lookup(pid)) == NULL) {
        np = (struct nlist *) malloc(sizeof(*np));
        if (np == NULL || (np->command = strdup(command)) == NULL)
            return NULL;
        np->pid = pid;
        np->index = index;
        hashval = hash(pid);
        np->next = hashtab[hashval];
        hashtab[hashval] = np;
    } else {
        free((void *) np->command); /*free previous defn */
        if ((np->command = strdup(command)) == NULL)
            return NULL;
    }
    return np;
}


int main() {
    int pid;
    int index = 0;
    size_t len = 100;
    char *Rcommand = (char *) malloc(sizeof(char) * len);
    char *command = (char *) malloc(sizeof(char) * len);

    //Start Getting the lines from the file or stdin
    while (getline(&command, &len, stdin) != -1) {
        //Remove new line symbole
        command[strlen(command) - 1] = 0;
        index++;
        pid = fork();

        //Error check
        if (pid < 0) {
            fprintf(stderr, "error forking");
            exit(2);
        } else if (pid == 0) {/*child */
            char fileO[10];
            char fileE[10];
            sprintf(fileO, "%d.out", getpid());
            sprintf(fileE, "%d.err", getpid());
            int fdout = open(fileO, O_RDWR | O_CREAT | O_APPEND, 0777);
            int fderr = open(fileE, O_RDWR | O_CREAT | O_APPEND, 0777);
            dup2(fdout, 1);
            dup2(fderr, 2);

            char *argument_list[10];
            int i = 0;
            fflush(stdout);


            char *Ccommand = strdup(command);
            char *p = strtok(Ccommand, " ");
            while (p != NULL) {
                argument_list[i++] = p;
                p = strtok(NULL, " ");
            }
            argument_list[i] = NULL;

            free(Ccommand);

            int result = execvp(argument_list[0], argument_list);

            //Error process
            if (result != 0) {
                exit(2);
            }
            exit(0); //END of Child
        } else if (pid > 0) {  /* parent*/
            //insert the new pid into the hash table
            struct nlist *entry_new = insert(command, pid, index);
            clock_gettime(CLOCK_MONOTONIC, &entry_new->start);

            char fileO[10];
            char fileE[10];
            sprintf(fileO, "%d.out", pid);
            sprintf(fileE, "%d.err", pid);
            int fdout = open(fileO, O_RDWR | O_CREAT | O_APPEND, 0777);
            int fderr = open(fileE, O_RDWR | O_CREAT | O_APPEND, 0777);
            dup2(fdout, 1);
            dup2(fderr, 2);

            printf("Starting command INDEX %d: child PID %d of parent PPID %d.\n", entry_new->index, pid, getpid());
            fflush(NULL);
        }//END of Parent
    }//End of While loop that get command

    int pidChild, status;

    //While wait loop
    while ((pidChild = wait(&status)) > 0) {
        char fileO[10];
        char fileE[10];
        sprintf(fileO, "%d.out", pidChild);
        sprintf(fileE, "%d.err", pidChild);
        int fdout = open(fileO, O_RDWR | O_CREAT | O_APPEND, 0777);
        int fderr = open(fileE, O_RDWR | O_CREAT | O_APPEND, 0777);
        dup2(fdout, 1);
        dup2(fderr, 2);

        //Find node in the hash table
        struct nlist *node = lookup(pidChild);

        if(node != NULL){
            clock_gettime(CLOCK_MONOTONIC, &node->finish);
            double elapsed = (node->finish.tv_sec - node->start.tv_sec);
            elapsed += (node->finish.tv_nsec - node->start.tv_nsec)/1000000000.0;
            printf("Ending command INDEX %d: child PID %d of parent PPID %d.\n", node->index, node->pid, getpid());
            fflush(NULL);
            printf("Finished at %ld, runtime duration = %f \n", node->finish.tv_sec, elapsed);
            fflush(NULL);
            if (WIFEXITED(status)) {//Command Exited with a code
                fprintf(stderr, "Exited with exitcode = %d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) { //Command was killed
                fprintf(stderr, "Killed with signal %d\n", WTERMSIG(status));
            }

            if (elapsed>2){
                strcpy(Rcommand,node->command);
            }

            free(node->command);
            free(node);
            if (elapsed > 2) { //Restart process
                index = 1;
                struct timespec start;
                clock_gettime(CLOCK_MONOTONIC, &start);
                pid = fork();
                if (pid < 0) {
                    fprintf(stderr, "error forking");
                    exit(2);
                } else if (pid == 0) {/*child */
                    char fileO[10];
                    char fileE[10];
                    sprintf(fileO, "%d.out", getpid());
                    sprintf(fileE, "%d.err", getpid());
                    int fdout = open(fileO, O_RDWR | O_CREAT | O_APPEND, 0777);
                    int fderr = open(fileE, O_RDWR | O_CREAT | O_APPEND, 0777);
                    dup2(fdout, 1);
                    dup2(fderr, 2);

                    char *argument_list[10];
                    int i = 0;
                    fflush(stdout);

                    char *p = strtok(Rcommand, " ");
                    while (p != NULL) {
                        argument_list[i++] = p;
                        p = strtok(NULL, " ");
                    }
                    argument_list[i] = NULL;

                    int result = execvp(argument_list[0], argument_list);

                    //Error process
                    if (result != 0) {
                        exit(2);
                    }
                    exit(0);//End of Restarted Child process
                } else if (pid > 0) {  /* parent */
                    //insert the new pid into the hash table
                    struct nlist *entry_new = insert(Rcommand, pid, index);
                    clock_gettime(CLOCK_MONOTONIC, &entry_new->start);
                    char fileO[10];
                    char fileE[10];
                    sprintf(fileO, "%d.out", pid);
                    sprintf(fileE, "%d.err", pid);
                    int fdout = open(fileO, O_RDWR | O_CREAT | O_APPEND, 0777);
                    int fderr = open(fileE, O_RDWR | O_CREAT | O_APPEND, 0777);
                    dup2(fdout, 1);
                    dup2(fderr, 2);
                    printf("RESTARTING\n");
                    fprintf(stderr, "RESTARTING\n");
                    printf("Starting command INDEX %d: child PID %d of parent PPID %d.\n", entry_new->index, pid, getpid());
                    fflush(NULL);
                } //end of Restarted Parent process
            }//End of Restart process
            else if (elapsed > 0) { //Process do not restart
                fprintf(stderr, "spawning too fast\n");
            } //End of spawning too fast process
        }//End of if(node!=NULL)
    }//End of While(Wait) loop
    free(command);
    free(Rcommand);
    return 0;
}
