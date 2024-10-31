/*
    COMP3511 Fall 2024
    PA2: Multi-level Feedback Queue

    Your name: Wong Hon Yin
    Your ITSC email: hywongdc@connect.ust.hk

    Declaration:

    I declare that I am not involved in plagiarism
    I understand that both parties (i.e., students providing the codes and students copying the codes) will receive 0 marks.

*/

// Note: Necessary header files are included
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Define MAX_NUM_PROCESS
// For simplicity, assume that we have at most 10 processes
#define MAX_NUM_PROCESS 10

#define MAX_PROCESS_NAME 5
#define MAX_GANTT_CHART 300

// N-level Feedback Queue (N=1,2,3,4)
#define MAX_NUM_QUEUE 4

// Keywords (to be used when parsing the input)
#define KEYWORD_QUEUE_NUMBER "queue_num"
#define KEYWORD_TQ "time_quantum"
#define KEYWORD_PROCESS_TABLE_SIZE "process_table_size"
#define KEYWORD_PROCESS_TABLE "process_table"

// Assume that we only need to support 2 types of space characters:
// " " (space), "\t" (tab)
#define SPACE_CHARS " \t"

// Process data structure
// Helper functions:
//  process_init: initialize a process entry
//  process_table_print: Display the process table
struct Process
{
    char name[MAX_PROCESS_NAME];
    int arrival_time;
    int burst_time;
    int remain_time; // remain_time is needed in the intermediate steps of MLFQ
};
void process_init(struct Process *p, char name[MAX_PROCESS_NAME], int arrival_time, int burst_time)
{
    strcpy(p->name, name);
    p->arrival_time = arrival_time;
    p->burst_time = burst_time;
    p->remain_time = 0;
}
void process_table_print(struct Process *p, int size)
{
    int i;
    printf("Process\tArrival\tBurst\n");
    for (i = 0; i < size; i++)
    {
        printf("%s\t%d\t%d\n", p[i].name, p[i].arrival_time, p[i].burst_time);
    }
}

// A simple GanttChart structure
// Helper functions:
//   gantt_chart_print: display the current chart
struct GanttChartItem
{
    char name[MAX_PROCESS_NAME];
    int duration;
};

void gantt_chart_print(struct GanttChartItem chart[MAX_GANTT_CHART], int n)
{
    int t = 0;
    int i = 0;
    printf("Gantt Chart = ");
    printf("%d ", t);
    for (i = 0; i < n; i++)
    {
        t = t + chart[i].duration;
        printf("%s %d ", chart[i].name, t);
    }
    printf("\n");
}

// Global variables
int queue_num = 0;
int process_table_size = 0;
struct Process process_table[MAX_NUM_PROCESS];
int time_quantum[MAX_NUM_QUEUE];

// Helper function: Check whether the line is a blank line (for input parsing)
int is_blank(char *line)
{
    char *ch = line;
    while (*ch != '\0')
    {
        if (!isspace(*ch))
            return 0;
        ch++;
    }
    return 1;
}
// Helper function: Check whether the input line should be skipped
int is_skip(char *line)
{
    if (is_blank(line))
        return 1;
    char *ch = line;
    while (*ch != '\0')
    {
        if (!isspace(*ch) && *ch == '#')
            return 1;
        ch++;
    }
    return 0;
}
// Helper: parse_tokens function
void parse_tokens(char **argv, char *line, int *numTokens, char *delimiter)
{
    int argc = 0;
    char *token = strtok(line, delimiter);
    while (token != NULL)
    {
        argv[argc++] = token;
        token = strtok(NULL, delimiter);
    }
    *numTokens = argc;
}

// Helper: parse the input file
void parse_input()
{
    FILE *fp = stdin;
    char *line = NULL;
    ssize_t nread;
    size_t len = 0;

    char *two_tokens[2];               // buffer for 2 tokens
    char *queue_tokens[MAX_NUM_QUEUE]; // buffer for MAX_NUM_QUEUE tokens
    int n;

    int numTokens = 0, i = 0;
    char equal_plus_spaces_delimiters[5] = "";

    char process_name[MAX_PROCESS_NAME];
    int process_arrival_time = 0;
    int process_burst_time = 0;

    strcpy(equal_plus_spaces_delimiters, "=");
    strcat(equal_plus_spaces_delimiters, SPACE_CHARS);

    // Note: MingGW don't have getline, so you are forced to do the coding in Linux/POSIX supported OS
    // In other words, you cannot easily coding in Windows environment

    while ((nread = getline(&line, &len, fp)) != -1)
    {
        if (is_skip(line) == 0)
        {
            line = strtok(line, "\n");

            if (strstr(line, KEYWORD_QUEUE_NUMBER))
            {
                // parse queue_num
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2)
                {
                    sscanf(two_tokens[1], "%d", &queue_num);
                }
            }
            else if (strstr(line, KEYWORD_TQ))
            {
                // parse time_quantum
                parse_tokens(two_tokens, line, &numTokens, "=");
                if (numTokens == 2)
                {
                    // parse the second part using SPACE_CHARS
                    parse_tokens(queue_tokens, two_tokens[1], &n, SPACE_CHARS);
                    for (i = 0; i < n; i++)
                    {
                        sscanf(queue_tokens[i], "%d", &time_quantum[i]);
                    }
                }
            }
            else if (strstr(line, KEYWORD_PROCESS_TABLE_SIZE))
            {
                // parse process_table_size
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2)
                {
                    sscanf(two_tokens[1], "%d", &process_table_size);
                }
            }
            else if (strstr(line, KEYWORD_PROCESS_TABLE))
            {

                // parse process_table
                for (i = 0; i < process_table_size; i++)
                {

                    getline(&line, &len, fp);
                    line = strtok(line, "\n");

                    sscanf(line, "%s %d %d", process_name, &process_arrival_time, &process_burst_time);
                    process_init(&process_table[i], process_name, process_arrival_time, process_burst_time);
                }
            }
        }
    }
}
// Helper: Display the parsed values
void print_parsed_values()
{
    printf("%s = %d\n", KEYWORD_QUEUE_NUMBER, queue_num);
    printf("%s = ", KEYWORD_TQ);
    for (int i = 0; i < queue_num; i++)
        printf("%d ", time_quantum[i]);
    printf("\n");
    printf("%s = \n", KEYWORD_PROCESS_TABLE);
    process_table_print(process_table, process_table_size);
}

/*
int queue_num = 0;
int process_table_size = 0;
struct Process process_table[MAX_NUM_PROCESS];
int time_quantum[MAX_NUM_QUEUE];
*/

// TODO: my own helper functions

// update gantt chart
void updateGanttChart(struct GanttChartItem *chart, int *sz_chart, struct Process *curProcess)
{
    // if chart is NOT empty and the last process is the same as the current process
    if (sz_chart > 0 && strcmp(chart[*sz_chart - 1].name, curProcess->name) == 0)
    {
        // add the 1 duration to the last process
        ++chart[*sz_chart - 1].duration;
    }
    else
    {
        // make a new item in chart
        strcpy(chart[*sz_chart].name, curProcess->name);
        chart[*sz_chart].duration = 1;
        // increase the size of chart
        ++*sz_chart;
    }
}

// TODO: Implementation of MLFQ algorithm
void mlfq()
{

    struct GanttChartItem chart[MAX_GANTT_CHART];
    int sz_chart = 0;

    // TODO: Write your code here to implement MLFQ
    // Tips: A simple array is good enough to implement a queue

    //! PROBLEM / CONFUSION: do i need to implement the FCFS? it said will input a large number and suppose it as FCFS

    // initialize the queue
    struct Process queue1[MAX_NUM_PROCESS];
    struct Process queue2[MAX_NUM_PROCESS];
    struct Process queue3[MAX_NUM_PROCESS];
    struct Process queue4[MAX_NUM_PROCESS];

    // index for each queue end
    int q1Index = 0;
    int q2Index = 0;
    int q3Index = 0;
    int q4Index = 0;

    // index for each queue start
    int q1Start = 0;
    int q2Start = 0;
    int q3Start = 0;
    int q4Start = 0;

    // burst time for each queue's process
    int q1Time = 0;
    int q2Time = 0;
    int q3Time = 0;
    int q4Time = 0;

    // total burst time add all burst time tgt
    int totalBurstTime = 0;
    for (int i = 0; i < process_table_size; i++)
    {
        totalBurstTime += process_table[i].burst_time;
    }

    // current time set to 0
    int curTime = 0;

    // Scheduleing loop each loop 1 unit time
    while (curTime < totalBurstTime)
    {

        // check if there is any process arrived at the current time
        // loop through all process
        for (int i = 0; i < process_table_size; i++)
        {
            // if the process arrived at the current time
            // add the process to the queue
            if (process_table[i].arrival_time == curTime)
            {
                // add the process to first queue
                queue1[q1Index] = process_table[i];
                ++q1Index;
            }
        }

        // queue1
        // if the queue1 time is greater than the time quantum of queue1
        if (q1Time >= time_quantum[0])
        {
            // add the process to the end of queue2
            queue2[q2Index] = queue1[q1Start];
            ++q2Index;
            // update the start of queue1
            q1Start++;
            // reset the burst time of queue1
            q1Time = 0;
        }

        // queue2
        if (q2Time >= time_quantum[1])
        {
            // add the process to the end of queue3
            queue3[q3Index] = queue2[q2Start];
            ++q3Index;
            // update the start of queue2
            q2Start++;
            // reset the burst time of queue2
            q2Time = 0;
        }

        // queue3
        if (q3Time >= time_quantum[2])
        {
            // add the process to the end of queue4
            queue4[q4Index] = queue3[q3Start];
            ++q4Index;
            // update the start of queue3
            q3Start++;
            // reset the burst time of queue3
            q3Time = 0;
        }

        // execute the process for 1 unit time each loop
        //  -> use if else if... to check which queue to execute
        //  check if the queue is empty or not
        if (q1Start < q1Index)
        {
            // queue1 is not empty
            // update the first process in queue1
            queue1[q1Start].remain_time -= 1;
            // update the burst time of queue1
            q1Time += 1;
            // update the current time
            curTime += 1;
            // update chart
            updateGanttChart(chart, &sz_chart, &queue1[q1Start]);
        }
        else if (q2Start < q2Index)
        {
            // queue2 is not empty
            // update the first process in queue2
            queue2[q2Start].remain_time -= 1;
            // update the burst time of queue2
            q2Time += 1;
            // update the current time
            curTime += 1;
            // update chart
            updateGanttChart(chart, &sz_chart, &queue2[q2Start]);
        }
        else if (q3Start < q3Index)
        {
            // queue3 is not empty
            // update the first process in queue3
            queue3[q3Start].remain_time -= 1;
            // update the burst time of queue3
            q3Time += 1;
            // update the current time
            curTime += 1;
            // update chart
            updateGanttChart(chart, &sz_chart, &queue3[q3Start]);
        }
        else if (q4Start < q4Index)
        {
            // queue4 is not empty
            // update the first process in queue4
            queue4[q4Start].remain_time -= 1;
            // update the burst time of queue4
            q4Time += 1;
            // update the current time
            curTime += 1;
            // update chart
            updateGanttChart(chart, &sz_chart, &queue4[q4Start]);
        }
        else
        {
            // no process can be executed
            // increase the current time
            curTime += 1;
        }
    }

    // At the end, display the final Gantt chart
    gantt_chart_print(chart, sz_chart);
}

int main()
{
    parse_input();
    print_parsed_values();
    mlfq();
    return 0;
}