

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_PROCESSES 100
#define MAX_QUEUE_SIZE 100
#define MAX_SERVICE_ROLE 100
#define TIME_QUANTUM 2  /* Linux: Shorter quantum for responsiveness */
#define CONTEXT_SWITCH_PENALTY 0.1  /* Linux: 0.1ms context switch overhead */
#define MAX_GANTT_ENTRIES 1000
#define MAX_ALGORITHMS 5

/* ANSI Colors */
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_BOLD    "\033[1m"

#define CHAR_ARROW "→"
#define CHAR_CHECK "✓"
#define CHAR_STAR  "★"

/* Process structure with context switching tracking */
typedef struct {
    char pid[10];
    int arrival_time;
    int burst_time;
    int priority;
    char service_role[MAX_SERVICE_ROLE];
    int remaining_time;
    double completion_time;  /* Now includes penalties */
    double turnaround_time;
    double waiting_time;
    double response_time;
    double start_time;
    int is_completed;
    int context_switches;  /* Track context switches per process */
} Process;

typedef struct {
    char pid[10];
    double start_time;
    double end_time;
} GanttEntry;

typedef struct {
    GanttEntry entries[MAX_GANTT_ENTRIES];
    int count;
} GanttChart;

typedef struct {
    Process* processes[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int size;
} Queue;

/* Enhanced performance metrics */
typedef struct {
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
void init_process(Process* p, const char* pid, int arrival, int burst, int priority, const char* role);
void copy_processes(Process src[], Process dest[], int n);
void init_queue(Queue* q);
int is_queue_empty(Queue* q);
void enqueue(Queue* q, Process* p);
Process* dequeue(Queue* q);
double get_time_ms();
void print_separator(int length);
void print_double_separator(int length);
void print_header(const char* algorithm);
void calculate_metrics(Process processes[], int n, int cs_count, PerformanceMetrics* metrics);
void print_metrics(const char* algorithm, Process processes[], int n, int cs_count, double exec_time);
void init_gantt(GanttChart* gc);
void add_gantt_entry(GanttChart* gc, const char* pid, double start, double end);
void print_gantt_chart_linux(GanttChart* gc);
void print_comparison_summary();
void print_linux_header();
int compare_arrival(const void* a, const void* b);

/* Scheduling algorithms with context switching tracking */
int fcfs_linux(Process processes[], int n, GanttChart* gc);
int srtf_linux(Process processes[], int n, GanttChart* gc);  /* Preemptive */
int round_robin_linux(Process processes[], int n, GanttChart* gc);
int priority_preemptive_linux(Process processes[], int n, GanttChart* gc);
int prr_linux(Process processes[], int n, GanttChart* gc);

/* ==================================================================================
 * MAIN FUNCTION
 * ================================================================================== */
int main() {
    printf("\033[2J\033[H");
    
    Process original_processes[5];
    
    init_process(&original_processes[0], "P1", 0, 6, 2, "Web Request Handler (Nginx)");
    init_process(&original_processes[1], "P2", 1, 4, 1, "Authentication Service");
    init_process(&original_processes[2], "P3", 2, 8, 1, "Database Query Processor");
    init_process(&original_processes[3], "P4", 0, 3, 4, "Logging & Monitoring Agent");
    init_process(&original_processes[4], "P5", 3, 10, 5, "Backup/Batch Analytics");
    
    int n = 5;
    
    print_linux_header();
    
    Process test_procs[MAX_PROCESSES];
    GanttChart gc;
    double start_time, end_time;
    int context_switches;
    
    /* 1. FCFS */
    copy_processes(original_processes, test_procs, n);
    init_gantt(&gc);
    start_time = get_time_ms();
    context_switches = fcfs_linux(test_procs, n, &gc);
    end_time = get_time_ms();
    print_metrics("FCFS (Linux)", test_procs, n, context_switches, end_time - start_time);
    print_gantt_chart_linux(&gc);
    
    /* 2. SRTF (Preemptive SJF) */
    copy_processes(original_processes, test_procs, n);
    init_gantt(&gc);
    start_time = get_time_ms();
    context_switches = srtf_linux(test_procs, n, &gc);
    end_time = get_time_ms();
    print_metrics("SRTF - Preemptive (Linux)", test_procs, n, context_switches, end_time - start_time);
    print_gantt_chart_linux(&gc);
    
    /* 3. Round Robin */
    copy_processes(original_processes, test_procs, n);
    init_gantt(&gc);
    start_time = get_time_ms();
    context_switches = round_robin_linux(test_procs, n, &gc);
    end_time = get_time_ms();
    print_metrics("Round Robin q=2ms (Linux)", test_procs, n, context_switches, end_time - start_time);
    print_gantt_chart_linux(&gc);
    
    /* 4. Preemptive Priority */
    copy_processes(original_processes, test_procs, n);
    init_gantt(&gc);
    start_time = get_time_ms();
    context_switches = priority_preemptive_linux(test_procs, n, &gc);
    end_time = get_time_ms();
    print_metrics("Priority Preemptive (Linux)", test_procs, n, context_switches, end_time - start_time);
    print_gantt_chart_linux(&gc);
    
    /* 5. PRR */
    copy_processes(original_processes, test_procs, n);
    init_gantt(&gc);
    start_time = get_time_ms();
    context_switches = prr_linux(test_procs, n, &gc);
    end_time = get_time_ms();
    print_metrics("Priority RR q=2ms (Linux)", test_procs, n, context_switches, end_time - start_time);
    print_gantt_chart_linux(&gc);
    
    print_comparison_summary();
    
    return 0;
}

/* ==================================================================================
 * INITIALIZATION FUNCTIONS
 * ================================================================================== */

void init_process(Process* p, const char* pid, int arrival, int burst, int priority, const char* role) {
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

void copy_processes(Process src[], Process dest[], int n) {
    for (int i = 0; i < n; i++) {
        dest[i] = src[i];
    }
}

void init_gantt(GanttChart* gc) {
    gc->count = 0;
}

void add_gantt_entry(GanttChart* gc, const char* pid, double start, double end) {
    if (gc->count >= MAX_GANTT_ENTRIES) return;
    
    if (gc->count > 0 && 
        strcmp(gc->entries[gc->count - 1].pid, pid) == 0 &&
        gc->entries[gc->count - 1].end_time == start) {
        gc->entries[gc->count - 1].end_time = end;
    } else {
        strcpy(gc->entries[gc->count].pid, pid);
        gc->entries[gc->count].start_time = start;
        gc->entries[gc->count].end_time = end;
        gc->count++;
    }
}

double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

void init_queue(Queue* q) {
    q->front = 0;
    q->rear = -1;
    q->size = 0;
}

int is_queue_empty(Queue* q) {
    return q->size == 0;
}

void enqueue(Queue* q, Process* p) {
    if (q->size >= MAX_QUEUE_SIZE) return;
    q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
    q->processes[q->rear] = p;
    q->size++;
}

Process* dequeue(Queue* q) {
    if (is_queue_empty(q)) return NULL;
    Process* p = q->processes[q->front];
    q->front = (q->front + 1) % MAX_QUEUE_SIZE;
    q->size--;
    return p;
}

int compare_arrival(const void* a, const void* b) {
    Process* p1 = (Process*)a;
    Process* p2 = (Process*)b;
    if (p1->arrival_time != p2->arrival_time)
        return p1->arrival_time - p2->arrival_time;
    return strcmp(p1->pid, p2->pid);
}

/* ==================================================================================
 * DISPLAY FUNCTIONS
 * ================================================================================== */

void print_separator(int length) {
    for (int i = 0; i < length; i++) printf("-");
    printf("\n");
}

void print_double_separator(int length) {
    printf("%s", COLOR_CYAN);
    for (int i = 0; i < length; i++) printf("=");
    printf("%s\n", COLOR_RESET);
}

void print_header(const char* algorithm) {
    printf("\n");
    print_double_separator(130);
    printf("%s%s%s %s%s\n", COLOR_BOLD, COLOR_YELLOW, CHAR_STAR, algorithm, COLOR_RESET);
    print_double_separator(130);
}

void print_linux_header() {
    print_double_separator(130);
    printf("%s%s     LINUX CPU SCHEDULER - WITH CONTEXT SWITCHING PENALTY & PERFORMANCE METRICS     %s\n", 
           COLOR_BOLD, COLOR_WHITE, COLOR_RESET);
    printf("%s   Context Switch: 0.1ms | Quantum: 2ms | Preemptive SRTF | Real-time Metrics   %s\n", 
           COLOR_CYAN, COLOR_RESET);
    print_double_separator(130);
    
    printf("\n%s%sTechNova Backend Server Processes:%s\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
    printf("  %sP1:%s Web Handler    - Priority 2, Burst 6ms\n", COLOR_GREEN, COLOR_RESET);
    printf("  %sP2:%s Auth Service   - Priority 1, Burst 4ms %s(CRITICAL)%s\n", COLOR_RED, COLOR_RESET, COLOR_BOLD, COLOR_RESET);
    printf("  %sP3:%s DB Processor   - Priority 1, Burst 8ms %s(CRITICAL)%s\n", COLOR_RED, COLOR_RESET, COLOR_BOLD, COLOR_RESET);
    printf("  %sP4:%s Logging Agent  - Priority 4, Burst 3ms\n", COLOR_BLUE, COLOR_RESET);
    printf("  %sP5:%s Batch Job      - Priority 5, Burst 10ms\n", COLOR_MAGENTA, COLOR_RESET);
    print_double_separator(130);
}

void calculate_metrics(Process processes[], int n, int cs_count, PerformanceMetrics* metrics) {
    double total_tat = 0, total_wt = 0, total_rt = 0;
    double max_completion = 0;
    int total_burst = 0;
    
    for (int i = 0; i < n; i++) {
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

void print_metrics(const char* algorithm, Process processes[], int n, int cs_count, double exec_time) {
    print_header(algorithm);
    
    printf("\n%s%-10s %-30s %-4s %-4s %-4s %-7s %-7s %-7s %-7s %-4s%s\n", 
           COLOR_BOLD, "Process", "Service", "AT", "BT", "Pri", "CT", "TAT", "WT", "RT", "CS", COLOR_RESET);
    print_separator(130);
    
    Process temp[MAX_PROCESSES];
    for (int i = 0; i < n; i++) temp[i] = processes[i];
    qsort(temp, n, sizeof(Process), compare_arrival);
    
    for (int i = 0; i < n; i++) {
        char role[31];
        strncpy(role, temp[i].service_role, 30);
        role[30] = '\0';
        
        const char* color = COLOR_RESET;
        if (temp[i].priority == 1) color = COLOR_RED;
        else if (temp[i].priority == 2) color = COLOR_GREEN;
        else if (temp[i].priority >= 4) color = COLOR_BLUE;
        
        printf("%s%-10s %-30s %-4d %-4d %-4d %-7.2f %-7.2f %-7.2f %-7.2f %-4d%s\n",
               color, temp[i].pid, role,
               temp[i].arrival_time, temp[i].burst_time, temp[i].priority,
               temp[i].completion_time, temp[i].turnaround_time,
               temp[i].waiting_time, temp[i].response_time,
               temp[i].context_switches, COLOR_RESET);
    }
    
    PerformanceMetrics metrics;
    calculate_metrics(processes, n, cs_count, &metrics);
    strcpy(metrics.algorithm_name, algorithm);
    metrics.computation_time = exec_time;
    
    if (comparison_count < MAX_ALGORITHMS) {
        comparison_table[comparison_count++] = metrics;
    }
    
    printf("\n%s%s Performance Metrics with Context Switching Analysis %s%s\n", 
           COLOR_BOLD, COLOR_GREEN, CHAR_STAR, COLOR_RESET);
    print_separator(130);
    printf("Average Turnaround Time:      %s%.2f ms%s\n", COLOR_CYAN, metrics.avg_turnaround_time, COLOR_RESET);
    printf("Average Waiting Time:         %s%.2f ms%s\n", COLOR_CYAN, metrics.avg_waiting_time, COLOR_RESET);
    printf("Average Response Time:        %s%.2f ms%s\n", COLOR_CYAN, metrics.avg_response_time, COLOR_RESET);
    printf("Total Context Switches:       %s%d switches%s\n", COLOR_YELLOW, metrics.total_context_switches, COLOR_RESET);
    printf("Context Switch Penalty:       %s%.2f ms%s (%.1fms × %d)\n", 
           COLOR_RED, metrics.total_cs_penalty, COLOR_RESET, CONTEXT_SWITCH_PENALTY, metrics.total_context_switches);
    printf("Effective CPU Time:           %s%.2f ms%s\n", COLOR_GREEN, metrics.effective_cpu_time, COLOR_RESET);
    printf("Total Execution Time:         %s%.2f ms%s\n", COLOR_CYAN, metrics.total_time, COLOR_RESET);
    printf("CPU Utilization:              %s%.2f%%%s\n", COLOR_CYAN, metrics.cpu_utilization, COLOR_RESET);
    printf("Context Switch Overhead:      %s%.2f%%%s\n", COLOR_MAGENTA, metrics.cs_overhead_percent, COLOR_RESET);
    printf("Throughput:                   %s%.4f processes/ms%s\n", COLOR_CYAN, metrics.throughput, COLOR_RESET);
    printf("Algorithm Computation:        %s%.4f ms%s\n", COLOR_MAGENTA, exec_time, COLOR_RESET);
}

void print_gantt_chart_linux(GanttChart* gc) {
    if (gc->count == 0) return;
    
    printf("\n%s%s Gantt Chart (with CS penalties shown) %s\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
    print_separator(130);
    
    printf("%sExecution Order:%s ", COLOR_BOLD, COLOR_RESET);
    for (int i = 0; i < gc->count; i++) {
        printf("%s%s%s", COLOR_CYAN, gc->entries[i].pid, COLOR_RESET);
        if (i < gc->count - 1) printf(" %s ", CHAR_ARROW);
    }
    printf(" (%s%d context switches%s)\n", COLOR_YELLOW, gc->count - 1, COLOR_RESET);
    print_double_separator(130);
}

void print_comparison_summary() {
    printf("\n\n");
    print_double_separator(145);
    printf("%s%s      LINUX PERFORMANCE COMPARISON - WITH CONTEXT SWITCHING ANALYSIS      %s%s\n", 
           COLOR_BOLD, COLOR_WHITE, CHAR_STAR, COLOR_RESET);
    print_double_separator(145);
    
    printf("\n%s%-30s %-8s %-8s %-8s %-6s %-10s %-10s %-10s%s\n",
           COLOR_BOLD, "Algorithm", "TAT(ms)", "WT(ms)", "RT(ms)", "CS", "CS Pen(ms)", "CPU%", "CS OH%", COLOR_RESET);
    print_separator(145);
    
    for (int i = 0; i < comparison_count; i++) {
        PerformanceMetrics m = comparison_table[i];
        printf("%-30s %-8.2f %-8.2f %-8.2f %-6d %-10.2f %-10.2f %-10.2f\n",
               m.algorithm_name, m.avg_turnaround_time, m.avg_waiting_time,
               m.avg_response_time, m.total_context_switches, m.total_cs_penalty,
               m.cpu_utilization, m.cs_overhead_percent);
    }
    
    print_double_separator(145);
    printf("\n%s%sLinux Scheduling Insights:%s\n", COLOR_BOLD, COLOR_GREEN, COLOR_RESET);
    printf("  %s Preemptive algorithms (SRTF, Priority) have MORE context switches\n", CHAR_ARROW);
    printf("  %s Each context switch adds 0.1ms overhead (Linux typical)\n", CHAR_ARROW);
    printf("  %s Shorter quantum (2ms) = more responsive but more overhead\n", CHAR_ARROW);
    printf("  %s SRTF provides best TAT despite context switch penalty\n", CHAR_ARROW);
    print_double_separator(145);
}

/* ==================================================================================
 * SCHEDULING ALGORITHMS WITH CONTEXT SWITCHING
 * ================================================================================== */

int fcfs_linux(Process processes[], int n, GanttChart* gc) {
    qsort(processes, n, sizeof(Process), compare_arrival);
    double current_time = 0;
    int cs_count = 0;
    
    for (int i = 0; i < n; i++) {
        if (i > 0) {
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

int srtf_linux(Process processes[], int n, GanttChart* gc) {
    double current_time = 0;
    int completed = 0;
    int cs_count = 0;
    int last_process = -1;
    
    while (completed < n) {
        int shortest = -1;
        int min_remaining = 99999;
        
        for (int i = 0; i < n; i++) {
            if (!processes[i].is_completed && processes[i].arrival_time <= current_time &&
                processes[i].remaining_time < min_remaining) {
                min_remaining = processes[i].remaining_time;
                shortest = i;
            }
        }
        
        if (shortest == -1) {
            current_time++;
            continue;
        }
        
        if (last_process != -1 && last_process != shortest) {
            current_time += CONTEXT_SWITCH_PENALTY;
            cs_count++;
            processes[shortest].context_switches++;
        }
        
        if (processes[shortest].response_time == -1) {
            processes[shortest].start_time = current_time;
            processes[shortest].response_time = current_time - processes[shortest].arrival_time;
        }
        
        add_gantt_entry(gc, processes[shortest].pid, current_time, current_time + 1);
        processes[shortest].remaining_time--;
        current_time++;
        
        if (processes[shortest].remaining_time == 0) {
            processes[shortest].completion_time = current_time;
            processes[shortest].turnaround_time = current_time - processes[shortest].arrival_time;
            processes[shortest].waiting_time = processes[shortest].turnaround_time - processes[shortest].burst_time;
            processes[shortest].is_completed = 1;
            completed++;
        }
        
        last_process = shortest;
    }
    
    return cs_count;
}

int round_robin_linux(Process processes[], int n, GanttChart* gc) {
    Queue queue;
    init_queue(&queue);
    qsort(processes, n, sizeof(Process), compare_arrival);
    
    double current_time = 0;
    int completed = 0;
    int index = 0;
    int cs_count = 0;
    int last_process_idx = -1;
    
    while (completed < n) {
        while (index < n && processes[index].arrival_time <= current_time) {
            enqueue(&queue, &processes[index]);
            index++;
        }
        
        if (is_queue_empty(&queue)) {
            if (index < n) current_time = processes[index].arrival_time;
            continue;
        }
        
        Process* p = dequeue(&queue);
        int p_idx = p - processes;
        
        if (last_process_idx != -1 && last_process_idx != p_idx) {
            current_time += CONTEXT_SWITCH_PENALTY;
            cs_count++;
            p->context_switches++;
        }
        
        if (p->response_time == -1) {
            p->start_time = current_time;
            p->response_time = current_time - p->arrival_time;
        }
        
        int exec_time = (TIME_QUANTUM < p->remaining_time) ? TIME_QUANTUM : p->remaining_time;
        add_gantt_entry(gc, p->pid, current_time, current_time + exec_time);
        p->remaining_time -= exec_time;
        current_time += exec_time;
        
        while (index < n && processes[index].arrival_time <= current_time) {
            enqueue(&queue, &processes[index]);
            index++;
        }
        
        if (p->remaining_time > 0) {
            enqueue(&queue, p);
        } else {
            p->completion_time = current_time;
            p->turnaround_time = current_time - p->arrival_time;
            p->waiting_time = p->turnaround_time - p->burst_time;
            completed++;
        }
        
        last_process_idx = p_idx;
    }
    
    return cs_count;
}

int priority_preemptive_linux(Process processes[], int n, GanttChart* gc) {
    double current_time = 0;
    int completed = 0;
    int cs_count = 0;
    int last_process = -1;
    
    while (completed < n) {
        int highest = -1;
        int best_priority = 99999;
        
        for (int i = 0; i < n; i++) {
            if (!processes[i].is_completed && processes[i].arrival_time <= current_time &&
                processes[i].priority < best_priority) {
                best_priority = processes[i].priority;
                highest = i;
            }
        }
        
        if (highest == -1) {
            current_time++;
            continue;
        }
        
        if (last_process != -1 && last_process != highest) {
            current_time += CONTEXT_SWITCH_PENALTY;
            cs_count++;
            processes[highest].context_switches++;
        }
        
        if (processes[highest].response_time == -1) {
            processes[highest].start_time = current_time;
            processes[highest].response_time = current_time - processes[highest].arrival_time;
        }
        
        add_gantt_entry(gc, processes[highest].pid, current_time, current_time + 1);
        processes[highest].remaining_time--;
        current_time++;
        
        if (processes[highest].remaining_time == 0) {
            processes[highest].completion_time = current_time;
            processes[highest].turnaround_time = current_time - processes[highest].arrival_time;
            processes[highest].waiting_time = processes[highest].turnaround_time - processes[highest].burst_time;
            processes[highest].is_completed = 1;
            completed++;
        }
        
        last_process = highest;
    }
    
    return cs_count;
}

int prr_linux(Process processes[], int n, GanttChart* gc) {
    Queue priority_queues[10];
    for (int i = 0; i < 10; i++) init_queue(&priority_queues[i]);
    
    qsort(processes, n, sizeof(Process), compare_arrival);
    double current_time = 0;
    int completed = 0;
    int index = 0;
    int cs_count = 0;
    int last_process_idx = -1;
    
    while (completed < n) {
        while (index < n && processes[index].arrival_time <= current_time) {
            enqueue(&priority_queues[processes[index].priority], &processes[index]);
            index++;
        }
        
        int highest_priority = -1;
        for (int i = 0; i < 10; i++) {
            if (!is_queue_empty(&priority_queues[i])) {
                highest_priority = i;
                break;
            }
        }
        
        if (highest_priority == -1) {
            if (index < n) current_time = processes[index].arrival_time;
            continue;
        }
        
        Process* p = dequeue(&priority_queues[highest_priority]);
        int p_idx = p - processes;
        
        if (last_process_idx != -1 && last_process_idx != p_idx) {
            current_time += CONTEXT_SWITCH_PENALTY;
            cs_count++;
            p->context_switches++;
        }
        
        if (p->response_time == -1) {
            p->start_time = current_time;
            p->response_time = current_time - p->arrival_time;
        }
        
        int exec_time = (TIME_QUANTUM < p->remaining_time) ? TIME_QUANTUM : p->remaining_time;
        add_gantt_entry(gc, p->pid, current_time, current_time + exec_time);
        p->remaining_time -= exec_time;
        current_time += exec_time;
        
        while (index < n && processes[index].arrival_time <= current_time) {
            enqueue(&priority_queues[processes[index].priority], &processes[index]);
            index++;
        }
        
        if (p->remaining_time > 0) {
            enqueue(&priority_queues[p->priority], p);
        } else {
            p->completion_time = current_time;
            p->turnaround_time = current_time - p->arrival_time;
            p->waiting_time = p->turnaround_time - p->burst_time;
            completed++;
        }
        
        last_process_idx = p_idx;
    }
    
    return cs_count;
}
