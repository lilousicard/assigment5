//---------------------------
//put commands in a data structure (e.g. hash table), which is used for recording for each exec
// the start time, finish time, index (line number in the commands file), pid and command.
//see the read_parse_file.c I provided for parsing each command, then
//save each cmd in an entry of the hashtable.

while ((linesize = getline(&command, &len, fp)) >= 0) {
    .....
    index++; // this index is the line number in the commands file.


    //---------------------------
    //fork the commands and record the start times

    //save the startime!
    // starttime = current time

pid = fork();

  if (pid < 0) { 
         fprintf(stderr, "error forking");
       exit(2);
    } else if (pid == 0) { /*child */
//See shell1_execvp.c for execvp usage
 execvp(.... command);  /*executes the command in the specific node */

} else if (pid > 0) {  /* parent goes to the next node */ 

        //insert the new pid into the hash table
        // entry_new = call hash insert(pid);
        //record the new starttime!
        // entry_new.starttime = starttime
        // entry_new.command = line
        // entry_new.index = index

int fdout = open("%d.out", pid);
int fderr = open("%d.err", pid);
fprintf(fdout, "Starting command INDEX %d: child PID %d of parent PPID %d.\n", entry.index, pid, getpid() ); 

}
} /*end of while loop*/


//---------------------------
//Final while loop: waits until anything has completed,
//this will exit (wait returns -1) 
//when there is no more child process. Then your parent process exits.

while((pid = wait(&status)) >= 0) {
    if(pid > 0) {
        //finishtime = get the finish time current time
        //search your hashtable for the entry that corresponds to pid
        //The function lookup was provided in the hashtable code
        //entry=hash lookup(pid)

        //signal handling
        int fdout = open("%d.out", pid);
        int fderr = open("%d.err", pid);

        fprintf(stderr, "Process with PID %d terminated.\n", pid);
        if (WIFEXITED(status)) {
            fprintf(fderr, "Child %d terminated normally with exit code: %d\n",
                   pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            fprintf(fderr, "Child %d terminated abnormally with signal number: %d\n",
                   pid, WTERMSIG(status));   }

        //
        //to compute the elapsed time you subtract
        //elapsedtime = finishtime - entry.start_time
        //decide if you will restart
        //if (elapsedtime > 2) {
        //save the startime!
        // starttime = current time
        // pid = fork();
        // if (pid < 0) //error
        // else if (pid == 0) { //child
        // See shell1_execvp.c for execvp usage
        // execvp(entry.command);
        // } else if (pid > 0) {
         //insert the new pid into the hash table
        // entry_new = call hash insert(pid);
        //record the new starttime!
        // entry_new.starttime = new starttime
        // entry_new.index = entry.index
        // entry_new.command = entry.command
        // }

    }
}
