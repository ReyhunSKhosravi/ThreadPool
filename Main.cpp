#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <fstream>
#include <sstream>
#include <chrono>
#include <atomic>

using namespace std;

struct Task {
    int id;
    int arrival_time;
    int execution_time;
};

class SimpleMutex {
private:
    atomic_flag flag = ATOMIC_FLAG_INIT;

public:
    void lock() {
        while (flag.test_and_set(memory_order_acquire)) {
            this_thread::sleep_for(chrono::milliseconds(1)); // Reduce CPU usage
        }
    }

    void unlock() {
        flag.clear(memory_order_release);
    }
};

class ThreadPool {
private:
    vector<thread> workers;
    queue<Task> taskQueue;
    bool stop;
    size_t maxQueueSize;
    ofstream logFile;
    SimpleMutex queueMutex;
    atomic<int> totalProcessedTasks{0};
    chrono::steady_clock::time_point startTime, endTime;

public:
    ThreadPool(size_t num_threads, size_t maxQueue, const string& logFileName) : stop(false), maxQueueSize(maxQueue) {
        logFile.open(logFileName, ios::out);
        if (!logFile.is_open()) {
            cerr << "Error opening log file: " << logFileName << endl;
            exit(1);
        }

        startTime = chrono::steady_clock::now();

        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    Task task;
                    queueMutex.lock();
                    if (!taskQueue.empty()) {
                        task = taskQueue.front();
                        taskQueue.pop();
                    } else {
                        queueMutex.unlock();
                        if (stop) return;
                        this_thread::sleep_for(chrono::milliseconds(1));
                        continue;
                    }
                    queueMutex.unlock();
                    
                    logFile << "Thread " << this_thread::get_id() << " executing Task " << task.id << "\n";
                    cout << "Thread " << this_thread::get_id() << " executing Task " << task.id << "\n";
                    this_thread::sleep_for(chrono::milliseconds(task.execution_time));
                    logFile << "Task " << task.id << " completed.\n";
                    cout << "Task " << task.id << " completed.\n";
                    totalProcessedTasks++;
                }
            });
        }
    }

    void enqueueTask(const Task& task) {
        while (true) {
            queueMutex.lock();
            if (taskQueue.size() < maxQueueSize) {
                taskQueue.push(task);
                queueMutex.unlock();
                break;
            }
            queueMutex.unlock();
            this_thread::sleep_for(chrono::milliseconds(1));
        }
    }

    void generateFinalReport() {
        endTime = chrono::steady_clock::now();
        chrono::duration<double> elapsedTime = endTime - startTime;

        queueMutex.lock();
        int remainingTasks = taskQueue.size();
        queueMutex.unlock();

        logFile << "\n--- Final Report ---\n";
        logFile << "Total tasks processed: " << totalProcessedTasks.load() << "\n";
        logFile << "Remaining tasks in queue: " << remainingTasks << "\n";
        logFile << "Total execution time: " << elapsedTime.count() << " seconds\n";
        logFile << "Pool shutdown completed successfully.\n";
    }

    ~ThreadPool() {
        stop = true;
        for (thread &worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        generateFinalReport();
        if (logFile.is_open()) {
            logFile.close();
        }
    }
};

void readConfig(int &num_threads, size_t &maxQueueSize) {
    ifstream configFile("config.txt");
    if (!configFile.is_open()) {
        cerr << "Error opening config.txt\n";
        exit(1);
    }

    string line;
    while (getline(configFile, line)) {
        if (line.find("num_threads=") == 0) {
            num_threads = stoi(line.substr(12));
        } else if (line.find("queue_size=") == 0) {
            maxQueueSize = stoi(line.substr(11));
        }
    }
    configFile.close();
}

void simulateExecution(const string& inputFile, int num_threads, size_t maxQueueSize) {
    ifstream file(inputFile);
    if (!file.is_open()) {
        cerr << "Error opening file: " << inputFile << endl;
        return;
    }

    string logFileName = "log_" + inputFile;
    ThreadPool pool(num_threads, maxQueueSize, logFileName);
    string line;
    vector<Task> tasks;
    
    while (getline(file, line)) {
        stringstream ss(line);
        Task task;
        ss >> task.id >> task.arrival_time >> task.execution_time;
        tasks.push_back(task);
    }
    file.close();

    for (const auto& task : tasks) {
        this_thread::sleep_for(chrono::milliseconds(task.arrival_time));
        pool.enqueueTask(task);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <task_file>\n";
        return 1;
    }

    string taskFile = argv[1];
    int numThreads;
    size_t maxQueueSize;

    readConfig(numThreads, maxQueueSize);

    simulateExecution(taskFile, numThreads, maxQueueSize);
    return 0;
}
