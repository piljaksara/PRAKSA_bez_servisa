
//
// Created by sarap on 8/5/2024.
//
#include <sys/sysinfo.h>  // Dodajte ovu liniju
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include<sys/time.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>     // For opendir(), readdir(), closedir()
#include <sys/types.h>  // For pid_t
#include <malloc.h>
#include <sys/resource.h>
#include <android/log.h>
#include "ResourceUsage.cpp"
#include <jni.h>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>


#define MAX_LINE_LENGTH 100
#define BUF_SIZE 1024
#define MAX_BUF_SIZE 256
#define CHUNK_SIZE (1024 * 1024) // Velicina jednog chunk-a je 1 MB
#define PATH_MAX 4096  // Maximum path length
#define LOG_TAG "NativeCode"



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
ResourceUsage*  test(const char* process_name);
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





ResourceUsage* usage= new ResourceUsage();

// Funkcija za prikupljanje i vraćanje podataka o zauzetosti CPU-a i memorije
ResourceUsage* get_resource_usage(pid_t pid) {
    double cpu_usage = get_cpu_usage_for_pid(pid);
    long memory_usage = get_memory_usage_for_pid(pid);
    return new ResourceUsage(cpu_usage, memory_usage);
}



ResourceUsage* test(const char* process_name) {
    pthread_t th1, th2;

    pid_t pid =getpid();
            //get_pid_by_name(process_name);
    /*if (pid == -1) {
        fprintf(stderr, "Error finding process with name %s\n", process_name);
        return nullptr;
    }*/

    //printf("Process: PID = %d\n", pid);

    usage = get_resource_usage(pid);

    printf("Initial Resource Usage:\n");
    printf("CPU usage: %.2f%%\n", usage->getCpuUsage());
    printf("Memory usage (RSS): %ld KB\n", usage->getMemoryUsage());

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
    //delete usage;
    usage = get_resource_usage(pid);

    printf("Final Resource Usage:\n");
    printf("CPU usage: %.2f%%\n", usage->getCpuUsage());
    printf("Memory usage (RSS): %ld KB\n", usage->getMemoryUsage());

    //delete usage; // Oslobađanje memorije
    return usage;
}


double measure_time_elapsed(const struct timespec *last_check_time) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double elapsed = (now.tv_sec - last_check_time->tv_sec) +
                     (now.tv_nsec - last_check_time->tv_nsec) / 1e9;
    return elapsed;
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

// Simulira opterećenje CPU-a na osnovu `load_factor`
void simulate_cpu_load(double load_factor) {
    struct timespec start, active_start, active_end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    double total_time = 15.0; // Ukupno vreme simulacije u sekundama
    double busy_time, idle_time;

    pid_t pid= getpid();

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

        usage = get_resource_usage(pid);
    }
    usage = get_resource_usage(pid);
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
        usage = get_resource_usage(pid);
        usage->set_cpu_usage(cpu_usage);
        usage->set_memory_usage(memory_usage);
    }

    return NULL;
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


///////////////////////////////////////////////////////////////////////////////////

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
    snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);

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

    if (diff_total_time < 0) {
        fprintf(stderr, "Error: negative time difference detected.\n");
        return -1.0;
    }

    // Dobijanje clock ticks po sekundi
    long clk_tck = sysconf(_SC_CLK_TCK);
    if (clk_tck <= 0) {
        perror("Error getting clock ticks per second");
        return -1.0;
    }

    // Izračunavanje CPU opterećenja
    double cpu_usage_percentage = ((double)diff_total_time / (clk_tck * elapsed)) * 100.0;

    prev_utime = utime;
    prev_stime = stime;
    last_check_time = now;

    return cpu_usage_percentage;
}

long get_memory_usage_for_pid(pid_t pid) {
    char status_path[MAX_BUF_SIZE];
    sprintf(status_path, "/proc/%d/status", pid);

    FILE *fp = fopen(status_path, "r");
    if (!fp) {
        perror("Error opening status file");
        return -1;
    }

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



// Funkcija za merenje proteklog vremena u sekundama
double measure_time_elapsed2(struct timespec *last_check_time) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now); // Uzimanje trenutnog vremena

    // Izračunavanje proteklog vremena
    double elapsed = (now.tv_sec - last_check_time->tv_sec) +
                     (now.tv_nsec - last_check_time->tv_nsec) / 1e9;

    // Proverite da li je proteklo više od jedne sekunde
    if (elapsed >= 1.0) {
        // Ažurirajte poslednje vreme
        *last_check_time = now;
    }

    return elapsed; // Vraća proteklo vreme u sekundama
}



int get_total_cpu_usage(double *cpu_usage) {
    static unsigned long long prev_user = 0;
    static unsigned long long prev_nice = 0;
    static unsigned long long prev_system = 0;
    static unsigned long long prev_idle = 0;
    static struct timespec last_check_time = {0};

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    // Izračunaj vreme proteklo od poslednjeg merenja
    double elapsed = measure_time_elapsed2(&last_check_time);
    if (elapsed < 1.0) {
        return -1; // Ako je prošlo manje od 1 sekunde, vrati prethodnu vrednost
    }

    // Otvori /proc/stat
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        perror("Error opening /proc/stat");
        return -1;
    }

    unsigned long long user, nice, system, idle;
    // Pročitaj prvi red iz /proc/stat
    fscanf(fp, "cpu %llu %llu %llu %llu", &user, &nice, &system, &idle);
    fclose(fp);

    // Izračunaj ukupno vreme
    unsigned long long total_time = user + nice + system + idle;

    // Izračunaj razlike
    unsigned long long diff_user = user - prev_user;
    unsigned long long diff_nice = nice - prev_nice;
    unsigned long long diff_system = system - prev_system;
    unsigned long long diff_idle = idle - prev_idle;
    unsigned long long diff_total = diff_user + diff_nice + diff_system + diff_idle;

    if (diff_total == 0) {
        *cpu_usage = 0; // Ako nema promene, CPU opterećenje je 0%
        return 0;
    }

    // Izračunaj CPU opterećenje
    double cpu_usage_percentage = 100.0 * (diff_total - diff_idle) / diff_total;

    // Sačuvaj trenutne vrednosti za sledeće merenje
    prev_user = user;
    prev_nice = nice;
    prev_system = system;
    prev_idle = idle;

    *cpu_usage = cpu_usage_percentage; // Postavi vrednost u pokazivač
    return 0; // Uspešno završeno
}



uint get_available_memory() {
    FILE *meminfo = fopen("/proc/meminfo", "r");
    if (meminfo == NULL) {
        printf("Greška u otvaranju fajla\n");
        return 0; // Vraćamo 0 kao indikator greške
    }

    char line[MAX_LINE_LENGTH];
    uint available = 0;

    while (fgets(line, MAX_LINE_LENGTH, meminfo)) {
        if (sscanf(line, "MemAvailable: %u kB", &available) == 1) {
            break; // Pronađena MemAvailable, izlazimo iz petlje
        }
    }

    fclose(meminfo);
    return available; // Vraća dostupnu memoriju u kB
}

void* simulate_cpu_load2(void* arg) {
    while (1) {
        // Ovdje možete dodati kod koji opterećuje CPU
        // Na primer, bespotrebni izračuni:
        double x = 0.0;
        for (long i = 0; i < 1000000; ++i) {
            x += (double)i;
        }
        // Očigledno, ovde ništa ne radimo, ali to opterećuje CPU
    }
    return NULL; // Vratite NULL na kraju
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_prekopiranceokod_MainActivity_getResourceUsage(JNIEnv *env, jobject thiz, jstring pid_jstring) {
    const char *pidCStr = env->GetStringUTFChars(pid_jstring, nullptr);
    std::string pidStr(pidCStr);
    env->ReleaseStringUTFChars(pid_jstring, pidCStr);


    pthread_t thread_id;
    pthread_create(&thread_id, NULL, simulate_cpu_load2, NULL);


    int pid = std::stoi(pidStr);
    double cpu_usage = get_cpu_usage_for_pid(pid);
    long memory_usage = get_memory_usage_for_pid(pid);

    // Dobijanje ukupne zauzetosti CPU i dostupne memorije
    double total_cpu_usage;
    if (get_total_cpu_usage(&total_cpu_usage) != 0) {
        fprintf(stderr, "Neuspešno dobijanje ukupne CPU zauzetosti.\n");
    }

    uint available_memory = get_available_memory();

    // Formatiranje rezultata
    std::string resultStr = "CPU Usage (PID " + std::to_string(pid) + "): " + std::to_string(cpu_usage) +
                            "%, Memory Usage: " + std::to_string(memory_usage) + " KB\n" +
                            "Total CPU Usage: " + std::to_string(total_cpu_usage) +
                            "%, Available Memory: " + std::to_string(available_memory) + " kB\n";

    // Sleep for 1 second before the next measurement
    sleep(1);

    // Vraćanje rezultata kao Java string
    return env->NewStringUTF(resultStr.c_str());
}