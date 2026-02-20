
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define MAX_PROCESSES 100
#define MAX_QUEUE_SIZE 100
#define MAX_SERVICE_ROLE 100
#define TIME_QUANTUM 4              /* Windows: Longer quantum for efficiency */
#define CONTEXT_SWITCH_PENALTY 0.05 /* Windows: 0.05ms context switch overhead */
#define MAX_GANTT_ENTRIES 1000
#define MAX_ALGORITHMS 5

/* Console colors */
#define CONSOLE_DEFAULT 7
#define CONSOLE_HEADER 11
#define CONSOLE_SUCCESS 10
#define CONSOLE_WARNING 14
#define CONSOLE_ERROR 12
#define CONSOLE_INFO 9
#define CONSOLE_BRIGHT 15

#define CHAR_ARROW "->"
#define CHAR_CHECK "v"
#define CHAR_STAR "*"

/* Process structure with context switching tracking */
typedef struct
{
    char pid[10];
    int arrival_time;
    int burst_time;
    int priority;
    char service_role[MAX_SERVICE_ROLE];
    int remaining_time;
    double completion_time;
    double turnaround_time;
    double waiting_time;
    double response_time;
    double start_time;
    int is_completed;
    int context_switches;
} Process;

typedef struct
{
    char pid[10];
    double start_time;
    double end_time;
} GanttEntry;

typedef struct
{
    GanttEntry entries[MAX_GANTT_ENTRIES];
    int count;
} GanttChart;

typedef struct
{
    Process *processes[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int size;
} Queue;

typedef struct
{
    char algorithm_name[50];
    double avg_turnaround_time;
    double avg_waiting_time;
    double avg_response_time;
    double throughput;
    double cpu_utilization;
    double total_time;
    double computation_time;
    int total_context_switches;
    double total_cs_penalty;
    double effective_cpu_time;
    double cs_overhead_percent;
} PerformanceMetrics;

PerformanceMetrics comparison_table[MAX_ALGORITHMS];
int comparison_count = 0;

/* Function prototypes */
void init_process(Process *p, const char *pid, int arrival, int burst, int priority, const char *role);
void copy_processes(Process src[], Process dest[], int n);
void init_queue(Queue *q);
int is_queue_empty(Queue *q);
void enqueue(Queue *q, Process *p);
Process *dequeue(Queue *q);
double get_time_ms();
void print_separator(int length);
void print_double_separator(int length);
void print_header(const char *algorithm);
void calculate_metrics(Process processes[], int n, int cs_count, PerformanceMetrics *metrics);
void print_metrics(const char *algorithm, Process processes[], int n, int cs_count, double exec_time);
void init_gantt(GanttChart *gc);
void add_gantt_entry(GanttChart *gc, const char *pid, double start, double end);
void print_gantt_chart_windows(GanttChart *gc);
void print_comparison_summary();
void print_windows_header();
int compare_arrival(const void *a, const void *b);
void set_console_color(int color);
void reset_console_color();

/* Scheduling algorithms */
int fcfs_windows(Process processes[], int n, GanttChart *gc);
int sjf_windows(Process processes[], int n, GanttChart *gc); /* Non-preemptive */
int round_robin_windows(Process processes[], int n, GanttChart *gc);
int priority_windows(Process processes[], int n, GanttChart *gc);
int prr_windows(Process processes[], int n, GanttChart *gc);

/* ==================================================================================
 * MAIN FUNCTION
 * ================================================================================== */
int main()
{
    system("cls");

    Process original_processes[5];

    init_process(&original_processes[0], "P1", 0, 6, 2, "Web Request Handler (Nginx)");
    init_process(&original_processes[1], "P2", 1, 4, 1, "Authentication Service");
    init_process(&original_processes[2], "P3", 2, 8, 1, "Database Query Processor");
    init_process(&original_processes[3], "P4", 0, 3, 4, "Logging & Monitoring Agent");
    init_process(&original_processes[4], "P5", 3, 10, 5, "Backup/Batch Analytics");

    int n = 5;

    print_windows_header();

    Process test_procs[MAX_PROCESSES];
    GanttChart gc;
    double start_time, end_time;
    int context_switches;

    /* 1. FCFS */
    copy_processes(original_processes, test_procs, n);
    init_gantt(&gc);
    start_time = get_time_ms();
    context_switches = fcfs_windows(test_procs, n, &gc);
    end_time = get_time_ms();
    print_metrics("FCFS (Windows)", test_procs, n, context_switches, end_time - start_time);
    print_gantt_chart_windows(&gc);

    /* 2. SJF (Non-preemptive) */
    copy_processes(original_processes, test_procs, n);
    init_gantt(&gc);
    start_time = get_time_ms();
    context_switches = sjf_windows(test_procs, n, &gc);
    end_time = get_time_ms();
    print_metrics("SJF - Non-Preemptive (Windows)", test_procs, n, context_switches, end_time - start_time);
    print_gantt_chart_windows(&gc);

    /* 3. Round Robin */
    copy_processes(original_processes, test_procs, n);
    init_gantt(&gc);
    start_time = get_time_ms();
    context_switches = round_robin_windows(test_procs, n, &gc);
    end_time = get_time_ms();
    print_metrics("Round Robin q=4ms (Windows)", test_procs, n, context_switches, end_time - start_time);
    print_gantt_chart_windows(&gc);

    /* 4. Priority */
    copy_processes(original_processes, test_procs, n);
    init_gantt(&gc);
    start_time = get_time_ms();
    context_switches = priority_windows(test_procs, n, &gc);
    end_time = get_time_ms();
    print_metrics("Priority Non-Preemptive (Windows)", test_procs, n, context_switches, end_time - start_time);
    print_gantt_chart_windows(&gc);

    /* 5. PRR */
    copy_processes(original_processes, test_procs, n);
    init_gantt(&gc);
    start_time = get_time_ms();
    context_switches = prr_windows(test_procs, n, &gc);
    end_time = get_time_ms();
    print_metrics("Priority RR q=4ms (Windows)", test_procs, n, context_switches, end_time - start_time);
    print_gantt_chart_windows(&gc);

    print_comparison_summary();

    printf("\nPress Enter to exit...");
    getchar();

    return 0;
}

/* ==================================================================================
 * WINDOWS-SPECIFIC FUNCTIONS
 * ================================================================================== */

double get_time_ms()
{
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / frequency.QuadPart * 1000.0;
}

void set_console_color(int color)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, (WORD)color);
}

void reset_console_color()
{
    set_console_color(CONSOLE_DEFAULT);
}

void init_process(Process *p, const char *pid, int arrival, int burst, int priority, const char *role)
{
    strcpy(p->pid, pid);
    p->arrival_time = arrival;
    p->burst_time = burst;
    p->priority = priority;
    strncpy(p->service_role, role, MAX_SERVICE_ROLE - 1);
    p->service_role[MAX_SERVICE_ROLE - 1] = '\0';
    p->remaining_time = burst;
    p->completion_time = 0;
    p->turnaround_time = 0;
    p->waiting_time = 0;
    p->response_time = -1;
    p->start_time = -1;
    p->is_completed = 0;
    p->context_switches = 0;
}

void copy_processes(Process src[], Process dest[], int n)
{
    for (int i = 0; i < n; i++)
    {
        dest[i] = src[i];
    }
}

void init_gantt(GanttChart *gc)
{
    gc->count = 0;
}

void add_gantt_entry(GanttChart *gc, const char *pid, double start, double end)
{
    if (gc->count >= MAX_GANTT_ENTRIES)
        return;

    if (gc->count > 0 &&
        strcmp(gc->entries[gc->count - 1].pid, pid) == 0 &&
        gc->entries[gc->count - 1].end_time == start)
    {
        gc->entries[gc->count - 1].end_time = end;
    }
    else
    {
        strcpy(gc->entries[gc->count].pid, pid);
        gc->entries[gc->count].start_time = start;
        gc->entries[gc->count].end_time = end;
        gc->count++;
    }
}

void init_queue(Queue *q)
{
    q->front = 0;
    q->rear = -1;
    q->size = 0;
}

int is_queue_empty(Queue *q)
{
    return q->size == 0;
}

void enqueue(Queue *q, Process *p)
{
    if (q->size >= MAX_QUEUE_SIZE)
        return;
    q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
    q->processes[q->rear] = p;
    q->size++;
}

Process *dequeue(Queue *q)
{
    if (is_queue_empty(q))
        return NULL;
    Process *p = q->processes[q->front];
    q->front = (q->front + 1) % MAX_QUEUE_SIZE;
    q->size--;
    return p;
}

int compare_arrival(const void *a, const void *b)
{
    Process *p1 = (Process *)a;
    Process *p2 = (Process *)b;
    if (p1->arrival_time != p2->arrival_time)
        return p1->arrival_time - p2->arrival_time;
    return strcmp(p1->pid, p2->pid);
}

/* ==================================================================================
 * DISPLAY FUNCTIONS
 * ================================================================================== */

void print_separator(int length)
{
    for (int i = 0; i < length; i++)
        printf("-");
    printf("\n");
}

void print_double_separator(int length)
{
    for (int i = 0; i < length; i++)
        printf("=");
    printf("\n");
}

void print_header(const char *algorithm)
{
    printf("\n");
    set_console_color(CONSOLE_HEADER);
    print_double_separator(130);
    set_console_color(CONSOLE_BRIGHT);
    printf("%s %s %s\n", CHAR_STAR, algorithm, CHAR_STAR);
    set_console_color(CONSOLE_HEADER);
    print_double_separator(130);
    reset_console_color();
}

void print_windows_header()
{
    set_console_color(CONSOLE_HEADER);
    print_double_separator(130);
    set_console_color(CONSOLE_BRIGHT);
    printf("    WINDOWS CPU SCHEDULER - WITH CONTEXT SWITCHING PENALTY & PERFORMANCE METRICS     \n");
    set_console_color(CONSOLE_INFO);
    printf("   Context Switch: 0.05ms | Quantum: 4ms | Non-Preemptive SJF | Real-time Metrics   \n");
    set_console_color(CONSOLE_HEADER);
    print_double_separator(130);
    reset_console_color();

    printf("\n");
    set_console_color(CONSOLE_WARNING);
    printf("TechNova Backend Server Processes:\n");
    reset_console_color();
    printf("  P1: Web Handler    - Priority 2, Burst 6ms\n");
    printf("  P2: Auth Service   - Priority 1, Burst 4ms (CRITICAL)\n");
    printf("  P3: DB Processor   - Priority 1, Burst 8ms (CRITICAL)\n");
    printf("  P4: Logging Agent  - Priority 4, Burst 3ms\n");
    printf("  P5: Batch Job      - Priority 5, Burst 10ms\n");
    set_console_color(CONSOLE_HEADER);
    print_double_separator(130);
    reset_console_color();
}

void calculate_metrics(Process processes[], int n, int cs_count, PerformanceMetrics *metrics)
{
    double total_tat = 0, total_wt = 0, total_rt = 0;
    double max_completion = 0;
    int total_burst = 0;

    for (int i = 0; i < n; i++)
    {
        total_tat += processes[i].turnaround_time;
        total_wt += processes[i].waiting_time;
        total_rt += processes[i].response_time;
        total_burst += processes[i].burst_time;
        if (processes[i].completion_time > max_completion)
            max_completion = processes[i].completion_time;
    }

    metrics->avg_turnaround_time = total_tat / n;
    metrics->avg_waiting_time = total_wt / n;
    metrics->avg_response_time = total_rt / n;
    metrics->total_time = max_completion;
    metrics->total_context_switches = cs_count;
    metrics->total_cs_penalty = cs_count * CONTEXT_SWITCH_PENALTY;
    metrics->effective_cpu_time = total_burst;
    metrics->throughput = max_completion > 0 ? (double)n / max_completion : 0;
    metrics->cpu_utilization = max_completion > 0 ? (total_burst / max_completion) * 100 : 0;
    metrics->cs_overhead_percent = max_completion > 0 ? (metrics->total_cs_penalty / max_completion) * 100 : 0;
}

void print_metrics(const char *algorithm, Process processes[], int n, int cs_count, double exec_time)
{
    print_header(algorithm);

    printf("\nProcess    Service                        AT   BT   Pri  CT      TAT     WT      RT      CS  \n");
    print_separator(130);

    Process temp[MAX_PROCESSES];
    for (int i = 0; i < n; i++)
        temp[i] = processes[i];
    qsort(temp, n, sizeof(Process), compare_arrival);

    for (int i = 0; i < n; i++)
    {
        char role[31];
        strncpy(role, temp[i].service_role, 30);
        role[30] = '\0';

        printf("%-10s %-30s %-4d %-4d %-4d %-7.2f %-7.2f %-7.2f %-7.2f %-4d\n",
               temp[i].pid, role,
               temp[i].arrival_time, temp[i].burst_time, temp[i].priority,
               temp[i].completion_time, temp[i].turnaround_time,
               temp[i].waiting_time, temp[i].response_time,
               temp[i].context_switches);
    }

    PerformanceMetrics metrics;
    calculate_metrics(processes, n, cs_count, &metrics);
    strcpy(metrics.algorithm_name, algorithm);
    metrics.computation_time = exec_time;

    if (comparison_count < MAX_ALGORITHMS)
    {
        comparison_table[comparison_count++] = metrics;
    }

    printf("\n");
    set_console_color(CONSOLE_SUCCESS);
    printf("%s Performance Metrics with Context Switching Analysis %s\n", CHAR_STAR, CHAR_STAR);
    reset_console_color();
    print_separator(130);
    printf("Average Turnaround Time:      %.2f ms\n", metrics.avg_turnaround_time);
    printf("Average Waiting Time:         %.2f ms\n", metrics.avg_waiting_time);
    printf("Average Response Time:        %.2f ms\n", metrics.avg_response_time);
    set_console_color(CONSOLE_WARNING);
    printf("Total Context Switches:       %d switches\n", metrics.total_context_switches);
    printf("Context Switch Penalty:       %.2f ms (%.2fms x %d)\n",
           metrics.total_cs_penalty, CONTEXT_SWITCH_PENALTY, metrics.total_context_switches);
    reset_console_color();
    set_console_color(CONSOLE_SUCCESS);
    printf("Effective CPU Time:           %.2f ms\n", metrics.effective_cpu_time);
    reset_console_color();
    printf("Total Execution Time:         %.2f ms\n", metrics.total_time);
    printf("CPU Utilization:              %.2f%%\n", metrics.cpu_utilization);
    set_console_color(CONSOLE_INFO);
    printf("Context Switch Overhead:      %.2f%%\n", metrics.cs_overhead_percent);
    reset_console_color();
    printf("Throughput:                   %.4f processes/ms\n", metrics.throughput);
    set_console_color(CONSOLE_INFO);
    printf("Algorithm Computation:        %.4f ms\n", exec_time);
    reset_console_color();
}

void print_gantt_chart_windows(GanttChart *gc)
{
    if (gc->count == 0)
        return;

    printf("\n");
    set_console_color(CONSOLE_WARNING);
    printf("%s Gantt Chart (with CS penalties shown) %s\n", CHAR_STAR, CHAR_STAR);
    reset_console_color();
    print_separator(130);

    printf("Execution Order: ");
    for (int i = 0; i < gc->count; i++)
    {
        set_console_color(CONSOLE_INFO);
        printf("%s", gc->entries[i].pid);
        reset_console_color();
        if (i < gc->count - 1)
            printf(" %s ", CHAR_ARROW);
    }
    set_console_color(CONSOLE_WARNING);
    printf(" (%d context switches)\n", gc->count - 1);
    reset_console_color();
    print_double_separator(130);
}

void print_comparison_summary()
{
    printf("\n\n");
    set_console_color(CONSOLE_HEADER);
    print_double_separator(145);
    set_console_color(CONSOLE_BRIGHT);
    printf("     WINDOWS PERFORMANCE COMPARISON - WITH CONTEXT SWITCHING ANALYSIS      \n");
    set_console_color(CONSOLE_HEADER);
    print_double_separator(145);
    reset_console_color();

    printf("\n%-30s %-8s %-8s %-8s %-6s %-10s %-10s %-10s\n",
           "Algorithm", "TAT(ms)", "WT(ms)", "RT(ms)", "CS", "CS Pen(ms)", "CPU%", "CS OH%");
    print_separator(145);

    for (int i = 0; i < comparison_count; i++)
    {
        PerformanceMetrics m = comparison_table[i];
        printf("%-30s %-8.2f %-8.2f %-8.2f %-6d %-10.2f %-10.2f %-10.2f\n",
               m.algorithm_name, m.avg_turnaround_time, m.avg_waiting_time,
               m.avg_response_time, m.total_context_switches, m.total_cs_penalty,
               m.cpu_utilization, m.cs_overhead_percent);
    }

    set_console_color(CONSOLE_HEADER);
    print_double_separator(145);
    reset_console_color();
    printf("\n");
    set_console_color(CONSOLE_SUCCESS);
    printf("Windows Scheduling Insights:\n");
    reset_console_color();
    printf("  %s Non-preemptive algorithms (SJF, Priority) have FEWER context switches\n", CHAR_ARROW);
    printf("  %s Each context switch adds only 0.05ms overhead (Windows efficient)\n", CHAR_ARROW);
    printf("  %s Longer quantum (4ms) = more efficient, less overhead\n", CHAR_ARROW);
    printf("  %s Lower context switch penalty = better overall performance\n", CHAR_ARROW);
    set_console_color(CONSOLE_HEADER);
    print_double_separator(145);
    reset_console_color();
}

/* ==================================================================================
 * SCHEDULING ALGORITHMS
 * ================================================================================== */

int fcfs_windows(Process processes[], int n, GanttChart *gc)
{
    qsort(processes, n, sizeof(Process), compare_arrival);
    double current_time = 0;
    int cs_count = 0;

    for (int i = 0; i < n; i++)
    {
        if (i > 0)
        {
            current_time += CONTEXT_SWITCH_PENALTY;
            cs_count++;
        }

        if (current_time < processes[i].arrival_time)
            current_time = processes[i].arrival_time;

        processes[i].start_time = current_time;
        processes[i].response_time = current_time - processes[i].arrival_time;
        processes[i].context_switches = (i > 0) ? 1 : 0;

        add_gantt_entry(gc, processes[i].pid, current_time, current_time + processes[i].burst_time);
        current_time += processes[i].burst_time;
        processes[i].completion_time = current_time;
        processes[i].turnaround_time = current_time - processes[i].arrival_time;
        processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;
    }

    return cs_count;
}

int sjf_windows(Process processes[], int n, GanttChart *gc)
{
    int current_time = 0;
    int completed = 0;
    int cs_count = 0;

    while (completed < n)
    {
        int shortest = -1;
        int min_burst = 99999;

        for (int i = 0; i < n; i++)
        {
            if (!processes[i].is_completed && processes[i].arrival_time <= current_time &&
                processes[i].burst_time < min_burst)
            {
                min_burst = processes[i].burst_time;
                shortest = i;
            }
        }

        if (shortest == -1)
        {
            int next_arrival = 99999;
            for (int i = 0; i < n; i++)
            {
                if (!processes[i].is_completed && processes[i].arrival_time > current_time &&
                    processes[i].arrival_time < next_arrival)
                    next_arrival = processes[i].arrival_time;
            }
            current_time = next_arrival;
            continue;
        }

        if (completed > 0)
        {
            current_time += CONTEXT_SWITCH_PENALTY;
            cs_count++;
            processes[shortest].context_switches = 1;
        }

        processes[shortest].start_time = current_time;
        processes[shortest].response_time = current_time - processes[shortest].arrival_time;
        add_gantt_entry(gc, processes[shortest].pid, current_time, current_time + processes[shortest].burst_time);
        current_time += processes[shortest].burst_time;
        processes[shortest].completion_time = current_time;
        processes[shortest].turnaround_time = current_time - processes[shortest].arrival_time;
        processes[shortest].waiting_time = processes[shortest].turnaround_time - processes[shortest].burst_time;
        processes[shortest].is_completed = 1;
        completed++;
    }

    return cs_count;
}

int round_robin_windows(Process processes[], int n, GanttChart *gc)
{
    Queue queue;
    init_queue(&queue);
    qsort(processes, n, sizeof(Process), compare_arrival);

    double current_time = 0;
    int completed = 0;
    int index = 0;
    int cs_count = 0;
    int last_process_idx = -1;

    while (completed < n)
    {
        while (index < n && processes[index].arrival_time <= current_time)
        {
            enqueue(&queue, &processes[index]);
            index++;
        }

        if (is_queue_empty(&queue))
        {
            if (index < n)
                current_time = processes[index].arrival_time;
            continue;
        }

        Process *p = dequeue(&queue);
        int p_idx = (int)(p - processes);

        if (last_process_idx != -1 && last_process_idx != p_idx)
        {
            current_time += CONTEXT_SWITCH_PENALTY;
            cs_count++;
            p->context_switches++;
        }

        if (p->response_time == -1)
        {
            p->start_time = current_time;
            p->response_time = current_time - p->arrival_time;
        }

        int exec_time = (TIME_QUANTUM < p->remaining_time) ? TIME_QUANTUM : p->remaining_time;
        add_gantt_entry(gc, p->pid, current_time, current_time + exec_time);
        p->remaining_time -= exec_time;
        current_time += exec_time;

        while (index < n && processes[index].arrival_time <= current_time)
        {
            enqueue(&queue, &processes[index]);
            index++;
        }

        if (p->remaining_time > 0)
        {
            enqueue(&queue, p);
        }
        else
        {
            p->completion_time = current_time;
            p->turnaround_time = current_time - p->arrival_time;
            p->waiting_time = p->turnaround_time - p->burst_time;
            completed++;
        }

        last_process_idx = p_idx;
    }

    return cs_count;
}

int priority_windows(Process processes[], int n, GanttChart *gc)
{
    int current_time = 0;
    int completed = 0;
    int cs_count = 0;

    while (completed < n)
    {
        int highest = -1;
        int best_priority = 99999;

        for (int i = 0; i < n; i++)
        {
            if (!processes[i].is_completed && processes[i].arrival_time <= current_time &&
                processes[i].priority < best_priority)
            {
                best_priority = processes[i].priority;
                highest = i;
            }
        }

        if (highest == -1)
        {
            int next_arrival = 99999;
            for (int i = 0; i < n; i++)
            {
                if (!processes[i].is_completed && processes[i].arrival_time > current_time &&
                    processes[i].arrival_time < next_arrival)
                    next_arrival = processes[i].arrival_time;
            }
            current_time = next_arrival;
            continue;
        }

        if (completed > 0)
        {
            current_time += CONTEXT_SWITCH_PENALTY;
            cs_count++;
            processes[highest].context_switches = 1;
        }

        processes[highest].start_time = current_time;
        processes[highest].response_time = current_time - processes[highest].arrival_time;
        add_gantt_entry(gc, processes[highest].pid, current_time, current_time + processes[highest].burst_time);
        current_time += processes[highest].burst_time;
        processes[highest].completion_time = current_time;
        processes[highest].turnaround_time = current_time - processes[highest].arrival_time;
        processes[highest].waiting_time = processes[highest].turnaround_time - processes[highest].burst_time;
        processes[highest].is_completed = 1;
        completed++;
    }

    return cs_count;
}

int prr_windows(Process processes[], int n, GanttChart *gc)
{
    Queue priority_queues[10];
    for (int i = 0; i < 10; i++)
        init_queue(&priority_queues[i]);

    qsort(processes, n, sizeof(Process), compare_arrival);
    double current_time = 0;
    int completed = 0;
    int index = 0;
    int cs_count = 0;
    int last_process_idx = -1;

    while (completed < n)
    {
        while (index < n && processes[index].arrival_time <= current_time)
        {
            enqueue(&priority_queues[processes[index].priority], &processes[index]);
            index++;
        }

        int highest_priority = -1;
        for (int i = 0; i < 10; i++)
        {
            if (!is_queue_empty(&priority_queues[i]))
            {
                highest_priority = i;
                break;
            }
        }

        if (highest_priority == -1)
        {
            if (index < n)
                current_time = processes[index].arrival_time;
            continue;
        }

        Process *p = dequeue(&priority_queues[highest_priority]);
        int p_idx = (int)(p - processes);

        if (last_process_idx != -1 && last_process_idx != p_idx)
        {
            current_time += CONTEXT_SWITCH_PENALTY;
            cs_count++;
            p->context_switches++;
        }

        if (p->response_time == -1)
        {
            p->start_time = current_time;
            p->response_time = current_time - p->arrival_time;
        }

        int exec_time = (TIME_QUANTUM < p->remaining_time) ? TIME_QUANTUM : p->remaining_time;
        add_gantt_entry(gc, p->pid, current_time, current_time + exec_time);
        p->remaining_time -= exec_time;
        current_time += exec_time;

        while (index < n && processes[index].arrival_time <= current_time)
        {
            enqueue(&priority_queues[processes[index].priority], &processes[index]);
            index++;
        }

        if (p->remaining_time > 0)
        {
            enqueue(&priority_queues[p->priority], p);
        }
        else
        {
            p->completion_time = current_time;
            p->turnaround_time = current_time - p->arrival_time;
            p->waiting_time = p->turnaround_time - p->burst_time;
            completed++;
        }

        last_process_idx = p_idx;
    }

    return cs_count;
}