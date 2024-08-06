//
// Created by sarap on 8/5/2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include<sys/time.h>
#include <sys/times.h>
//#include "native-functions.h"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>     // For opendir(), readdir(), closedir()
#include <sys/types.h>  // For pid_t
#include <malloc.h>
#include <sys/resource.h>
#include <android/log.h>


#define MAX_LINE_LENGTH 100
#define BUF_SIZE 1024
#define MAX_BUF_SIZE 256
#define CHUNK_SIZE (1024 * 1024) // Velicina jednog chunk-a je 1 MB
#define PATH_MAX 4096  // Maximum path length
#define LOG_TAG "NativeCode"

typedef struct {
    double cpu_usage;
    long memory_usage;
} ResourceUsage;

// Flag za kontrolu intenzivnog opterećenja
int stop_flag = 0;

extern int (*func_ptr)();
void *worker(void *data);
void *worker1(void *data);
void *worker2(void *data);
uint read_from_file();
int read_cpu_stats(unsigned long long *user, unsigned long long *nice, unsigned long long *system, unsigned long long *idle) ;
int prikaziZauzetostCpu();
void print_process_info(pid_t pid);
int ispisiInfoOProcesu();
double get_cpu_usage(int pid);
extern int malloc_trim(size_t pad);
ResourceUsage  test(const char* process_name);
long get_memory_usage_for_pid(pid_t pid) ;
double get_cpu_usage_for_pid(pid_t pid) ;
void simulate_memory_allocation_cpp(double load_factor) ;
void simulate_memory_allocation_c(double load_factor);
size_t simulate_memory_allocation_and_release(int iteration, size_t *allocated_memory) ;
void simulate_decreasing_cpu_usage(pid_t pid) ;
void simulate_cpu_load_rand(double load_factor);
double get_cpu_usage_for_pid_2(pid_t pid, unsigned long* prev_utime, unsigned long* prev_stime, double* prev_time);
void simulate_decreasing_cpu_load(double load_factor) ;
pid_t get_pid_by_name(const char* process_name);




int FunkcijaUC(int(*PFunkcijaUCppKojaSePozivaIzC)()){
    func_ptr = PFunkcijaUCppKojaSePozivaIzC;
    return 0;
}

int init(){
    pthread_t th1, th2,th3,th4,th5;
    pthread_create(&th1, NULL, worker, NULL);
    sleep(7);

    pthread_create(&th2, NULL, worker, NULL);
    sleep(5);

    pthread_create(&th3, NULL, worker, NULL);
    sleep(2);

    pthread_create(&th4, NULL, worker, NULL);
    sleep(1);

    printf("\nSve niti kreirane\n");
    return 0;
}

void *worker(void *data)
{
    uint avaib0;
    uint avaib1;
    uint avaib2;

    for (int i = 0; i < 1; i++)
    {
        int N = 10;
        int value = rand() % (N + 1);

        usleep(value);
        printf("\n\nHi from thread \n");



        printf("Memorija pre alokacije\n");
        avaib0=read_from_file();

        int *ptr = (int*)malloc(10*1024*1024*N);
        printf("Zauzeta memorija u kB: %ld\n", 10*1024*1024*N*sizeof(int)/1000);
        prikaziZauzetostCpu();
        //ispisiInfoOProcesu();
        //printf("int: %ld\n", sizeof(int));

        if (ptr == NULL) { printf("Memory not allocated.\n"); exit(0);}

        else {
            printf("\nMemory successfully allocated using malloc.\n");

            long long int i=0;
            while(i<1024LL*1024*1024*8){
                i=i+1;
            }


            pid_t pid = getpid();
            printf("My PID is: %d\n", pid);
            double cpu_usage = get_cpu_usage(pid);
            if (cpu_usage < 0.0) {
                fprintf(stderr, "Failed to get CPU usage for PID %d.\n", pid);
            }

            printf("CPU usage for PID %d: %.2f%%\n", pid, cpu_usage);


            for (uint j = 0; j <10*1024*1024 ; ++j) {
                ptr[j] = 1;
            }
            avaib1=read_from_file();
            //printf("\nRazlika Available memorije pre i nakon alokacije %d kB \n", avaib0-avaib1);

        }
        if(!func_ptr){
            printf("Null pointer, napravljena nit ne radi nista\n");
        }
        else (*func_ptr)();

        free(ptr);
        printf("Memorija nakon oslobadjanja\n");
        read_from_file();
    }
    return NULL;
}


uint read_from_file(){
    FILE *meminfo = fopen("/proc/meminfo", "r");
    if(meminfo == NULL){
        printf("greska u otvaranju fajla") ;
    }

    char line2[MAX_LINE_LENGTH];
    int i=0;
    uint free;
    uint avaib;
    while(fgets(line2, MAX_LINE_LENGTH, meminfo) && i<54){
        //if(i==0)printf("%s", line2);
        if(i==1){
            sscanf(line2, "MemFree: %d kB", &free) ;
            printf("%s", line2);
        }
        if(i==2){
            sscanf(line2, "MemAvailable: %d kB", &avaib) ;
            printf("%s", line2);
        }
        i++;
    }

    fclose(meminfo);
    return avaib;
}



int read_cpu_stats(unsigned long long *user, unsigned long long *nice,
                   unsigned long long *system, unsigned long long *idle) {
    FILE *fp;
    char buffer[BUF_SIZE];
    unsigned long long user_, nice_, system_, idle_;

    fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        perror("Error opening /proc/stat");
        return 0;
    }

    // Read the first line which starts with 'cpu'
    if (fgets(buffer, BUF_SIZE - 1, fp) == NULL) {
        fclose(fp);
        return 0;
    }

    // Parse CPU stats
    sscanf(buffer, "cpu %llu %llu %llu %llu", &user_, &nice_, &system_, &idle_);

    fclose(fp);

    // Return values via pointers
    *user = user_;
    *nice = nice_;
    *system = system_;
    *idle = idle_;

    return 1;
}


int prikaziZauzetostCpu(){

    unsigned long long user, nice, system, idle;
    unsigned long long user_prev, nice_prev, system_prev, idle_prev;
    double usage;

    // Read initial CPU stats
    if (!read_cpu_stats(&user_prev, &nice_prev, &system_prev, &idle_prev)) {
        fprintf(stderr, "Failed to read initial CPU stats.\n");
        return 1;
    }

    // Wait or perform some work
    // For demonstration, let's read again after a delay
    sleep(1);

    // Read updated CPU stats
    if (!read_cpu_stats(&user, &nice, &system, &idle)) {
        fprintf(stderr, "Failed to read updated CPU stats.\n");
        return 1;
    }

    // Calculate total CPU time
    unsigned long long total_prev = user_prev + nice_prev + system_prev + idle_prev;
    unsigned long long total = user + nice + system + idle;

    // Calculate CPU usage percentage
    usage = ((total - total_prev) - (idle - idle_prev)) * 100.0 / (total - total_prev);

    printf("CPU Usage: %.2f%%\n", usage);
    return 0;
}


// Function to read and print process information for a given PID
void print_process_info(pid_t pid) {
    char proc_path[MAX_BUF_SIZE];
    snprintf(proc_path, sizeof(proc_path), "/proc/%d/stat", pid);

    FILE *fp = fopen(proc_path, "r");
    if (fp == NULL) {
        perror("Error opening file");
        return;
    }

    // Read the first line from the stat file
    char buf[MAX_BUF_SIZE];
    if (fgets(buf, sizeof(buf), fp) == NULL) {
        fclose(fp);
        return;
    }

    fclose(fp);

    // Parse the information from the stat file
    char comm[MAX_BUF_SIZE];
    char state;
    int ppid;
    unsigned long utime, stime, vsize, rss;

    sscanf(buf, "%c %d %ld %ld %lu %lu",  &state, &ppid, &utime, &stime, &vsize, &rss);

    // Convert RSS from pages to kilobytes for better readability
    rss *= (unsigned long) sysconf(_SC_PAGESIZE) / 1024;
    printf("\nprocesi kojima fizicka mem nije 0");
    if(rss!=0){
        // Print the parsed information
        printf("PID: %d\n", pid);
        printf("Command: %s\n", comm);
        printf("State: %c\n", state);
        printf("Parent PID: %d\n", ppid);
        printf("User CPU time: %lu\n", utime);
        printf("System CPU time: %lu\n", stime);
        printf("Virtual Memory Size: %lu KB\n", vsize);
        printf("Physical Memory (RSS): %lu KB\n", rss);
        printf("\n");
    }

}

int ispisiInfoOProcesu() {

    printf("\n informacije o procesu");
    DIR *dir;
    struct dirent *ent;
    pid_t my_pid = getpid();  // Get current process PID

    // Open the /proc directory
    if ((dir = opendir("/proc")) != NULL) {
        // Iterate over all entries in the /proc directory
        while ((ent = readdir(dir)) != NULL) {
            // Check if the entry is a directory and its name consists only of digits (process ID)
            if (ent->d_type == DT_DIR && atoi(ent->d_name) != 0) {
                pid_t pid = atoi(ent->d_name);
                if (pid != my_pid) {  // Skip current process
                    print_process_info(pid);
                }
            }
        }
        closedir(dir);
    } else {
        perror("Error opening /proc directory");
        return 1;
    }

    return 0;
}


// Function to get system uptime in seconds
long uptime() {
    FILE *fp;
    char path[PATH_MAX];
    long uptime_seconds = 0;

    // Open the uptime file
    snprintf(path, PATH_MAX, "/proc/uptime");
    fp = fopen(path, "r");
    if (fp == NULL) {
        perror("fopen");
        return -1;
    }

    // Read uptime value
    if (fscanf(fp, "%ld", &uptime_seconds) != 1) {
        perror("fscanf");
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return uptime_seconds;
}


// Function to calculate CPU usage for a given process ID (PID)
double get_cpu_usage(int pid) {
    FILE *fp;
    char path[PATH_MAX];
    char line[PATH_MAX];
    char *token;
    long utime_ticks, stime_ticks, cutime_ticks, cstime_ticks;
    long total_ticks, seconds;
    double cpu_usage;

    // Open the stat file for the given process ID
    snprintf(path, PATH_MAX, "/proc/%d/stat", pid);
    fp = fopen(path, "r");
    if (fp == NULL) {
        perror("fopen");
        return -1.0;
    }

    // Read the line from stat file
    if (fgets(line, PATH_MAX, fp) == NULL) {
        perror("fgets");
        fclose(fp);
        return -1.0;
    }

    fclose(fp);

    // Extract CPU time information from the line
    token = strtok(line, " ");  // Skip PID
    for (int i = 0; i < 13; ++i) {
        token = strtok(NULL, " ");  // Move to the 14th field (utime)
    }

    // Read CPU time in clock ticks
    utime_ticks = atol(token);
    token = strtok(NULL, " ");  // stime
    stime_ticks = atol(token);
    token = strtok(NULL, " ");  // cutime
    cutime_ticks = atol(token);
    token = strtok(NULL, " ");  // cstime
    cstime_ticks = atol(token);

    // Calculate total CPU time in clock ticks
    total_ticks = utime_ticks + stime_ticks + cutime_ticks + cstime_ticks;

    // Calculate seconds since the process started
    seconds = 10.0;

    // Calculate CPU usage percentage
    cpu_usage = 100 * ((double)total_ticks / sysconf(_SC_CLK_TCK)) / seconds;

    return cpu_usage;
}




// Funkcija za prikupljanje i vraćanje podataka o zauzetosti CPU-a i memorije
ResourceUsage get_resource_usage(pid_t pid) {
    ResourceUsage usage;

    // Prikupljanje podataka o CPU zauzetosti
    usage.cpu_usage = get_cpu_usage_for_pid(pid);

    // Prikupljanje podataka o zauzetosti memorije
    usage.memory_usage = get_memory_usage_for_pid(pid);

    return usage;
}





ResourceUsage  test(const char* process_name) {
    pthread_t th1, th2;

    pid_t pid = get_pid_by_name(process_name);
    if (pid == -1) {
        fprintf(stderr, "Error finding process with name %s\n", process_name);
        return {0.0,0};
    }

    printf("Process: PID = %d\n", pid);

    ResourceUsage usage = get_resource_usage(pid);

    printf("Initial Resource Usage:\n");
    printf("CPU usage: %.2f%%\n", usage.cpu_usage);
    printf("Memory usage (RSS): %ld KB\n", usage.memory_usage);

    int value = pid % 10;

    if (value < 5) {
        pthread_create(&th2, NULL, worker2, (void*)&pid);
        sleep(1);
        pthread_join(th2, NULL);
        printf("\nReached thread 2\n");
    } else {
        pthread_create(&th1, NULL, worker1, (void*)&pid);
        sleep(3);
        pthread_join(th1, NULL);
        printf("\nReached thread 1\n");
    }

    // Ponovo prikupi podatke nakon rada sa nitima
    usage = get_resource_usage(pid);

    printf("Final Resource Usage:\n");
    printf("CPU usage: %.2f%%\n", usage.cpu_usage);
    printf("Memory usage (RSS): %ld KB\n", usage.memory_usage);
    return usage; // Vraćamo strukturu ResourceUsage
}


// Funkcija za pronalaženje PID-a procesa sa zadatim imenom
pid_t get_pid_by_name(const char* name) {
    char command[MAX_BUF_SIZE];
    snprintf(command, sizeof(command), "pgrep -f %s", name);

    FILE* fp = popen(command, "r");
    if (fp == NULL) {
        perror("Error opening pipe");
        return -1;
    }

    pid_t pid;
    if (fscanf(fp, "%d", &pid) != 1) {
        perror("Error reading PID");
        pclose(fp);
        return -1;
    }

    pclose(fp);
    return pid;
}


// Funkcija za merenje proteklog vremena
double measure_time_elapsed(struct timespec* start) {
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    return (end.tv_sec - start->tv_sec) + (end.tv_nsec - start->tv_nsec) / 1e9;
}

// Simulira opterećenje CPU-a na osnovu `load_factor`
void simulate_cpu_load(double load_factor) {
    struct timespec start, active_start, active_end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    double total_time = 2.0; // Ukupno vreme simulacije u sekundama
    double busy_time, idle_time;

    while (measure_time_elapsed(&start) < total_time) {
        busy_time = load_factor; // Aktivno vreme u sekundi
        idle_time = 1.0 - load_factor; // Idle vreme u sekundi

        clock_gettime(CLOCK_MONOTONIC, &active_start);

        // Aktivni rad
        while (1) {
            clock_gettime(CLOCK_MONOTONIC, &active_end);
            double elapsed = measure_time_elapsed(&active_start);
            if (elapsed >= busy_time) {
                break;
            }
            // Intenzivna operacija
            volatile double x = 1.0;
            for (volatile long i = 0; i < 1000000; ++i) {
                x *= 1.0001; // Malo složenija operacija
            }
            sched_yield(); // Poboljšanje raspodele CPU vremena
        }

        // Pauza za smanjenje opterećenja CPU-a
        if (idle_time > 0) {
            struct timespec req = {0, (long)(idle_time * 1e9)};
            nanosleep(&req, NULL);
        }

        // Postepeno povećanje opterećenja
        load_factor += 0.1;
        if (load_factor > 1.0) {
            load_factor = 1.0;
        }

        // Ispisivanje trenutnog opterećenja
        printf("Current CPU load factor: %.2f\n", load_factor);

        // Pauza između iteracija kako bi se omogućilo stabilizovanje opterećenja
        usleep(100000); // 0.1 sekundi
    }
}


// Funkcija za simulaciju zauzimanja memorije pomocu malloc
void simulate_memory_allocation_c(double load_factor) {
    const size_t chunk_size = 1024 * 1024; // Velicina jednog chunk-a je 1 MB
    char** memory_chunks = NULL; // Niz pokazivaca na char, koji cemo kasnije osloboditi

    long long iterations = (long long)(load_factor * 10); // Broj iteracija zavisi od load_factor-a
    long long total_allocated = 0;

    memory_chunks = (char**)malloc(iterations * sizeof(char*));

    if (memory_chunks == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }

    // Alociranje memorije za svaki chunk i zauzimanje memorije
    for (long long i = 0; i < iterations; ++i) {
        memory_chunks[i] = (char*)malloc(chunk_size); // Alociramo chunk velicine chunk_size
        if (memory_chunks[i] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            // Oslobodimo dosadasnje alocirane chunk-ove
            for (long long j = 0; j < i; ++j) {
                free(memory_chunks[j]);
            }
            free(memory_chunks);
            return;
        }

        // Zauzimanje memorije
        memset(memory_chunks[i], 'A', chunk_size); // Popunjavamo memoriju sa 'A' karakterima
        total_allocated += chunk_size;
    }

    printf("Total allocated memory: %lld bytes\n", total_allocated);

    // Ispisivanje sadrzaja alocirane memorije
    for (long long i = 0; i < iterations; ++i) {
        // Ispisivanje nije obavezno, ali može pomoći u proveri
        // printf("Memory chunk %lld:\n%s\n", i, memory_chunks[i]);
    }

    // Oslobadjamo alociranu memoriju
    /*for (long long i = 0; i < iterations; ++i) {
        free(memory_chunks[i]);
    }
    free(memory_chunks);*/
}

// Funkcija za merenje zauzetosti CPU-a za dati PID
double get_cpu_usage_for_pid(pid_t pid) {
    static unsigned long prev_utime = 0;
    static unsigned long prev_stime = 0;
    static struct timespec last_check_time = {0};

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    // Izračunaj vreme proteklo od poslednjeg merenja
    double elapsed = measure_time_elapsed(&last_check_time);
    if (elapsed < 1.0) {
        // Ako je prošlo manje od 1 sekunde, vrati prethodnu vrednost
        return -1.0;
    }

    char stat_path[MAX_BUF_SIZE];
    sprintf(stat_path, "/proc/%d/stat", pid);

    FILE *fp = fopen(stat_path, "r");
    if (!fp) {
        perror("Error opening stat file");
        return -1.0;
    }

    // Čitanje potrebnih vrednosti iz /proc/[PID]/stat
    unsigned long utime, stime, total_time;
    if (fscanf(fp, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu", &utime, &stime) != 2) {
        perror("Error reading values from stat file");
        fclose(fp);
        return -1.0;
    }
    fclose(fp);

    // Izračunavanje ukupnog CPU vremena u clock ticks (jiffies)
    total_time = utime + stime;
    unsigned long diff_utime = utime - prev_utime;
    unsigned long diff_stime = stime - prev_stime;
    unsigned long diff_total_time = diff_utime + diff_stime;

    // Dobijanje clock ticks po sekundi
    long clk_tck = sysconf(_SC_CLK_TCK);

    // Izračunavanje CPU opterećenja
    double cpu_usage_percentage = ((double)diff_total_time / (clk_tck * elapsed)) * 100.0;

    prev_utime = utime;
    prev_stime = stime;
    last_check_time = now;

    return cpu_usage_percentage;
}




void *worker1(void *data) {
    printf("\n\nHi from thread 1 \n");
    //__android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Hi from thread 1");
    pid_t pid = getpid();

    double initial_load = 0.1;  // Početni faktor opterećenja
    double increment = 0.1;     // Inkrement za povećanje faktora opterećenja

    // Simulirajte postupno povećanje opterećenja
    for (double load_factor = initial_load; load_factor <= 1.0; load_factor += increment) {
        // Simulirajte CPU opterećenje i alociranje memorije
        simulate_cpu_load(load_factor);
        simulate_memory_allocation_c(load_factor);

        // Merenje CPU opterećenja
        usleep(100000); // Čekanje 0.1 sekundi da bi se omogućilo stabilizovanje opterećenja
        double cpu_usage = get_cpu_usage_for_pid(pid);
        if (cpu_usage < 0.0) {
            fprintf(stderr, "Failed to retrieve CPU usage\n");
        } else {
            printf("CPU usage for process with PID %d: %.2f%%\n", pid, cpu_usage);
        }

        // Merenje memorijske potrošnje
        long memory_usage = get_memory_usage_for_pid(pid);
        if (memory_usage < 0) {
            fprintf(stderr, "Failed to retrieve memory usage\n");
        } else {
            printf("Memory usage (RSS) for process with PID %d: %ld KB\n", pid, memory_usage);
        }

        // Pauza između iteracija kako bi se uočila promena u opterećenju
        usleep(100000); // 0.1 sekundi
    }

    return NULL;
}
// Function to get memory usage (RSS) for a given PID
long get_memory_usage_for_pid(pid_t pid) {
    char status_path[MAX_BUF_SIZE];
    sprintf(status_path, "/proc/%d/status", pid);

    FILE *fp = fopen(status_path, "r");
    if (!fp) {
        perror("Error opening status file");
        return -1;
    }

    // Read the RSS (Resident Set Size) value from /proc/[PID]/status
    long rss = -1;
    char line[MAX_BUF_SIZE];
    const char *search = "VmRSS:";
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, search, strlen(search)) == 0) {
            if (sscanf(line, "%*s %ld", &rss) == 1) {
                break;
            }
        }
    }
    fclose(fp);

    return rss;
}













/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




// Funkcija koja prikuplja CPU vreme iz /proc/[PID]/stat
void get_cpu_times(pid_t pid, unsigned long* utime, unsigned long* stime) {
    char stat_path[MAX_BUF_SIZE];
    sprintf(stat_path, "/proc/%d/stat", pid);

    FILE *fp = fopen(stat_path, "r");
    if (!fp) {
        perror("Error opening stat file");
        return;
    }

    if (fscanf(fp, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu",
               utime, stime) != 2) {
        perror("Error reading values from stat file");
    }
    fclose(fp);
}

// Funkcija koja izračunava CPU opterećenje
double get_cpu_usage_for_pid_2(pid_t pid, unsigned long* prev_utime, unsigned long* prev_stime, double* prev_time) {
    unsigned long utime, stime;
    double current_time, delta_time;
    double cpu_usage;

    // Preuzmi trenutno CPU vreme
    get_cpu_times(pid, &utime, &stime);

    // Preuzmi trenutno vreme
    struct timeval tv;
    gettimeofday(&tv, NULL);
    current_time = tv.tv_sec + tv.tv_usec / 1e6;

    // Izračunaj prošlo vreme
    delta_time = current_time - *prev_time;

    if (delta_time < 0.1) {
        // Izbegavaj prečesto merenje
        return -1.0;
    }

    // Izračunaj CPU opterećenje kao procenat
    if (delta_time > 0) {
        unsigned long delta_utime = utime - *prev_utime;
        unsigned long delta_stime = stime - *prev_stime;
        unsigned long total_time = delta_utime + delta_stime;
        cpu_usage = (double)total_time / sysconf(_SC_CLK_TCK) / delta_time * 100.0;
        // Ograniči CPU opterećenje na maksimum 100%
        if (cpu_usage > 100.0) {
            cpu_usage = 100.0;
        }
    } else {
        cpu_usage = 0.0;
    }

    // Spremi trenutne vrednosti za sledeće merenje
    *prev_utime = utime;
    *prev_stime = stime;
    *prev_time = current_time;

    return cpu_usage;
}

// Funkcija koja simulira intenzivno CPU opterećenje za određeni period
void* simulate_heavy_cpu_load(void* arg) {
    int duration = *(int*)arg;
    struct timeval start_time, current_time;
    gettimeofday(&start_time, NULL);

    while (!stop_flag) {
        gettimeofday(&current_time, NULL);
        double elapsed = (current_time.tv_sec - start_time.tv_sec) +
                         (current_time.tv_usec - start_time.tv_usec) / 1e6;
        if (elapsed >= duration) {
            break;
        }
        // Aktivna petlja za visoko opterećenje
        volatile unsigned long i;
        for (i = 0; i < 100000000UL; ++i) {
            // Busy wait
        }
    }

    return NULL;
}

// Simulacija postepeno smanjenje opterećenja procesora i rada sa memorijom
void simulate_decreasing_cpu_usage(pid_t pid) {
    unsigned long prev_utime = 0, prev_stime = 0;
    double prev_time = 0.0;

    int measurement_interval = 2; // interval merenja u sekundama (duplo veći)

    pthread_t load_thread;
    int load_duration = 10;  // Trajanje visokog opterećenja u sekundama
    pthread_create(&load_thread, NULL, simulate_heavy_cpu_load, &load_duration);

    double initial_load = 1.0;
    double decrement = 0.1;
    size_t total_used_memory = CHUNK_SIZE * 10;
    size_t total_released_memory = 0;

    for (double load_factor = initial_load; load_factor >= 0.1; load_factor -= decrement) {
        usleep(1000000); // Pauza od 1 sekunde (duplo veća nego pre)

        // Izmeri CPU opterećenje
        double cpu_usage = get_cpu_usage_for_pid_2(pid, &prev_utime, &prev_stime, &prev_time);

        // Simuliši alokaciju i oslobađanje memorije
        size_t allocated_memory = 0;
        size_t released_memory = simulate_memory_allocation_and_release((int)((initial_load - load_factor) / decrement), &allocated_memory);
        total_used_memory -= released_memory;
        total_released_memory += released_memory;

        // Ispis informacija
        if (cpu_usage >= 0) {
            printf("CPU opterećenje za proces sa PID %d: %.2f%%\n", pid, cpu_usage);
        } else {
            printf("CPU opterećenje nije dostupno\n");
        }
        printf("Ukupno zauzeta memorija u ovoj iteraciji: %zu bajta\n", total_used_memory);
        printf("Oslobođena memorija u ovoj iteraciji: %zu bajta\n", released_memory);

        sleep(measurement_interval);
    }

    // Prestanite sa opterećenjem
    stop_flag = 1;
    pthread_join(load_thread, NULL);
}

// Simulacija oslobađanja memorije
size_t simulate_memory_allocation_and_release(int iteration, size_t *allocated_memory) {
    // Generišemo slučajan broj između 1 i CHUNK_SIZE / (iteration + 1)
    srand(time(NULL));
    int random_release_size = rand() % (CHUNK_SIZE / (iteration + 1)) + 1;

    // Alociramo memoriju
    char* memory_chunk = (char*)malloc(CHUNK_SIZE);

    if (memory_chunk == NULL) {
        fprintf(stderr, "Greška pri alokaciji memorije\n");
        return 0;  // Vraćamo 0 ako alokacija ne uspe
    }

    // Popunjavamo memoriju 'A' karakterima
    memset(memory_chunk, 'A', CHUNK_SIZE);

    // Simulacija rada sa alociranom memorijom
    usleep(500000);  // Primer: pauziramo 0.5 sekundi

    // Oslobađamo deo alocirane memorije
    free(memory_chunk);

    // Optimizacija memorije (možda se ne oslobodi odmah)
    //malloc_trim(random_release_size);

    // Vraćamo veličinu oslobođene memorije
    *allocated_memory = random_release_size;
    return random_release_size;
}

// Worker thread function
void *worker2(void *data) {
    printf("\n\nHi from thread 2 \n");
    pid_t pid = getpid();

    simulate_decreasing_cpu_usage(pid);

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


