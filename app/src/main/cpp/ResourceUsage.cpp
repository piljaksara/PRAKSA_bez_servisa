#include <iostream>

class ResourceUsage {
private:
    double cpu_usage;
    long memory_usage;

public:
    // Konstruktor
    ResourceUsage(double cpu = 0.0, long memory = 0) : cpu_usage(cpu), memory_usage(memory) {}

    // Getteri
    double getCpuUsage() const { return cpu_usage; }
    long getMemoryUsage() const { return memory_usage; }

    // Setteri
    void set_cpu_usage(double cpu) { cpu_usage = cpu; }
    void set_memory_usage(long memory) { memory_usage = memory; }

    // Metod za ispis informacija
    void print_usage() const {
        std::cout << "CPU Usage: " << cpu_usage << "%" << std::endl;
        std::cout << "Memory Usage: " << memory_usage << " KB" << std::endl;
    }
};
