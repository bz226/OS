// must compile with: gcc  -std=c99 -Wall -o oslab oslab.c

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* WE will use the three-state model */
#define READY   0
#define RUNNING 1
#define BLOCKED 2
#define MAXNUM 9999 //pid must be less than this number

/* string of process states */
char states[3][10] = {"ready", "running", "blocked"};

/* Information about each process */
/* Important: Feel free to add any new fields you want. But do not remove any of the cited one. */
struct process_info{
	int pid; // process ID, cannot be negative
	int state; //state of the process: ready, running, blocked
	int arrival_time; // time the process arrives
	int complete_time; //time the process finishes
	int turnaround_time; // complete_time - arrival_time
	int cpu1; //first computing interval
	int io; // io interval
	int cpu2; // second computing interval
	int done; // This flag is initialized to 0 and is set to 1 when the process is done
	int remaining_cpu1;
	int remaining_cpu2;
	int remaining_io;
};

/**** Global Variables ***/

struct process_info * process_list; // array containing all processes info
int num_processes = 0;  // total number of processes

void process_file(FILE *); // We implemented this function for you.

/* TODO: You will implement the following functions */
/* Feel free to add more functions if you need. */

int scheduler(int); // Returns the next process to be scheduled for execution (i.e. its state becomes RUNNING) or -1 if none exists. 
int scheduler(int current_time) {
    int selected_pid = -1;
    int min_pid = MAXNUM;
    
    // Find the process with the lowest PID that is READY
    for (int i = 0; i < num_processes; i++) {
        if (process_list[i].state == READY && 
            process_list[i].done == 0 && 
            process_list[i].arrival_time <= current_time) {
            
            if (process_list[i].pid < min_pid) {
                min_pid = process_list[i].pid;
                selected_pid = i; 
            }
        }
    }
    
    return selected_pid;
}

int update_running_process(int, int); //This function updates the remaining time for cpu1, cpu2, or IO. First input: process PID, second input: current time. Returns 1 if the process completed and 0 otherwise.
int update_running_process(int process_index, int current_time) {
    // Returns 1 if the process completed and 0 otherwise.
    
    if (process_list[process_index].remaining_cpu1 > 0) {
        process_list[process_index].remaining_cpu1--;
        
        if (process_list[process_index].remaining_cpu1 == 0) {
            // If CPU1 is done, move to BLOCKED state for IO
            process_list[process_index].state = BLOCKED;
            return 0;
        }
    }
    else if (process_list[process_index].remaining_cpu2 > 0) {
        process_list[process_index].remaining_cpu2--;
        
        if (process_list[process_index].remaining_cpu2 == 0) {
            // If CPU2 is done, process is complete
            process_list[process_index].done = 1;
            process_list[process_index].complete_time = current_time + 1; // +1 because we complete at the end of this time unit
            process_list[process_index].turnaround_time = 
                process_list[process_index].complete_time - process_list[process_index].arrival_time;
            return 1;
        }
    }
    
    return 0;
}

void update_blocked_processes(int); 
void update_blocked_processes(int current_time) {
    for (int i = 0; i < num_processes; i++) {
        if (process_list[i].state == BLOCKED && 
            process_list[i].done == 0 && 
            process_list[i].arrival_time <= current_time) {
            
            if (process_list[i].remaining_io > 0) {
                process_list[i].remaining_io--;
                
                if (process_list[i].remaining_io == 0) {
                    // When IO is complete, move to READY state for CPU2
                    process_list[i].state = READY;
                }
            }
        }
    }
}

void print_processes(FILE *, int, int); //print blocked/ready processes into the output file> The other two inputs are the current time and the PID of the currently running process.
void print_processes(FILE *output, int current_time, int running_process_index) {
    for (int i = 0; i < num_processes; i++) {
        // Skip the running process, processes that haven't arrived yet, or completed processes
        if (i == running_process_index || process_list[i].arrival_time > current_time || process_list[i].done == 1) {
            continue;
        }
        
        fprintf(output, "process : %d : %s\n", 
                process_list[i].pid, 
                states[process_list[i].state]);
    }
}
/**********************************************************************/

int main(int argc, char *argv[]){
  
FILE * fp; //for creating the output file
FILE * output; // output file
char filename[50]=""; // the output file name
int cpu_active = 0; // Incremented each cycle the CPU is busy
int time; //The clock
int num_completed_processes = 0;
int running_process_index = -1; // index of the process currently on the cpu
int i;



// Check that the command line is correct
if(argc != 2){

  printf("usage:  ./oslab filename\n");
  printf("filename: the processese information file\n");
  exit(1);
}

// Process the input command line. 

//Check that the file specified by the user exists and open it
if( !(fp = fopen(argv[1],"r")))
{
  printf("Cannot open file %s\n", argv[1]);
  exit(1);
}

// process input file
process_file(fp);

// form the output file name: original name + number of processes + .out
strcat(filename,argv[1]); 
sprintf(filename,"%s%d.out",filename, num_processes);
if( !(output = fopen(filename,"w")))
{
  printf("Cannot open file %s\n", filename);
  exit(1);
}

// Main simulation loop
time = 0;
while(num_completed_processes < num_processes){
	
	// Print the current time in the output file
    fprintf(output, "time:%d\n", time);
	
	// Find the process to be running right now
	running_process_index = scheduler(time);
	
	// If there is a process to run
	if (running_process_index != -1) {
        cpu_active++;
        process_list[running_process_index].state = RUNNING;
        
        // Print information about the running process
        fprintf(output, "process : %d : %s\n", 
                process_list[running_process_index].pid, 
                states[process_list[running_process_index].state]);
        
        // Update the running process and check if it completed
        if (update_running_process(running_process_index, time)) {
            num_completed_processes++;
        }
    }
	
	// Print information about non-running processes
	print_processes(output, time, running_process_index);
    
    // Update the status of blocked processes
	update_blocked_processes(time);
	
	// Increment the time
	time++;
}

// Print final statistics
printf("num processes = %d\n", num_processes);
printf("CPU utilization = %f\n", (float)cpu_active/(float)time);
printf("total time = %d\n", time);
for(i = 0; i < num_processes; i++)
	printf("process %d: turnaround time = %d\n", process_list[i].pid, process_list[i].turnaround_time);

// close the processes file
fclose(fp);
fclose(output);
free(process_list);

return 0;
}

/**********************************************************************/
/* The following function does the following:
- Reads the number of process from the input file and saves it in the global variable: num_processes
- Allocates space for the processes list: process_list
- Read the file and fill up the info in the process_info array
- Keeps reading the file and fill up the information in the process_list 
*/
void process_file(FILE * fp){

int i = 0;
fscanf(fp, "%d", &num_processes);	

if((process_list = malloc(num_processes * sizeof(struct process_info) ))== NULL){
	printf("Failure to allocate process list of %d processes\n", num_processes);
	exit(1);
}

while( fscanf(fp,"%d %d %d %d %d", &process_list[i].pid, &process_list[i].cpu1, &process_list[i].io, &process_list[i].cpu2, &process_list[i].arrival_time) == 5){ 
	
	process_list[i].done = 0;
	process_list[i].remaining_cpu1 = process_list[i].cpu1;
	process_list[i].remaining_cpu2 = process_list[i].cpu2;
	process_list[i].remaining_io = process_list[i].io;
	process_list[i].state = READY;
	i++;
   }	
}

/**********************************************************************/