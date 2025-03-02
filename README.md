# ThreadPool
Efficient C++ thread pool for concurrent task execution. Reads tasks, schedules them across multiple threads, logs execution, and generates a final report. Supports configurable pool size. 🚀 Ideal for OS &amp; concurrency projects.

## Overview
This project implements a **thread pool** in C++ to efficiently manage concurrent task execution. It takes a **task file** as input, reads configuration from `config.txt`, and executes tasks across multiple threads while logging execution details and generating a final report.

## Features
-  **Multi-threaded task execution** using a thread pool.
-  **Configurable thread pool size and queue capacity** via `config.txt`.
-  **Custom thread-safe task queue** implementation.
-  **Manual mutex implementation (`SimpleMutex`)** instead of using `std::mutex`.
-  **Dynamic log file generation** (`log_<task_file>.txt`).
-  **Final report generation** summarizing execution statistics.

## Requirements
- **C++11 or later**
- **GCC or Clang compiler**
- **Any operating system with a C++11-compatible compiler (Windows/Linux/macOS)**

## Compilation
To compile the program, use:
```bash
g++ -std=c++11 -o thread_pool thread_pool.cpp -pthread
```

## Running the Program
```bash
./thread_pool <task_file>
```
### Example:
```bash
./thread_pool input1.txt
```
This generates an output log file named `log_input1.txt` containing execution details and a final report.

## Input Format
The **task file** should contain tasks in the format:
```
<task_id> <arrival_time> <execution_time>
```
### Example `input1.txt`:
```
1 100 300
2 500 400
3 1000 200
4 1500 500
```

## Configuration File (`config.txt`)
Defines thread pool settings:
```
num_threads=4
queue_size=10
```

## Output Log File
The program generates a log file (`log_<task_file>.txt`), containing:
1. Execution logs of each task.
2. Task completion details.
3. **Final summary report** with performance metrics.

### Example Log Output (`log_input1.txt`):
```
Thread 140372654835456 executing Task 1
Task 1 completed.
Thread 140372646442752 executing Task 2
Task 2 completed.
Thread 140372638050048 executing Task 3
Task 3 completed.
Thread 140372629657344 executing Task 4
Task 4 completed.

--- Final Report ---
Total tasks processed: 4
Remaining tasks in queue: 0
Total execution time: 3.56 seconds
Pool shutdown completed successfully.
```

---

## **Algorithm Explanation**
This program uses **First Come First Serve (FCFS)** scheduling, where tasks are processed in the order they arrive.

### **Thread Pool Implementation**
- A **fixed number of worker threads** are created at startup.
- Tasks are stored in a **FIFO queue (`std::queue<Task>`)**.
- Each worker thread continuously fetches tasks and executes them.

### **Mutex Implementation (Without `std::mutex`)**
- Instead of using `std::mutex`, we implement **`SimpleMutex`** using `atomic_flag`.
- `SimpleMutex` provides **thread-safe locking**:
```cpp
class SimpleMutex {
private:
    atomic_flag flag = ATOMIC_FLAG_INIT;
public:
    void lock() {
        while (flag.test_and_set(memory_order_acquire)) {
            this_thread::sleep_for(chrono::milliseconds(1)); // Prevent CPU overload
        }
    }
    void unlock() {
        flag.clear(memory_order_release);
    }
};
```
- This ensures that only **one thread accesses the queue at a time**, preventing race conditions.

### **Queue Management**
- A **FIFO queue (`std::queue<Task>`)** stores incoming tasks.
- Tasks are added using `enqueueTask()`, which waits if the queue is full.
- Worker threads fetch tasks in `while(true)`, ensuring execution continues until all tasks are completed.

### **Final Report Generation**
At shutdown, `generateFinalReport()` calculates execution time and records:
- **Total tasks processed**
- **Remaining tasks in queue**
- **Execution duration**
- **Shutdown confirmation**

---

## **Notes**
- Ensure `config.txt` exists before running the program.
- The program **automatically determines the log file name** based on the task file input.
- Uses a **custom manual mutex implementation (`SimpleMutex`)** instead of `std::mutex`.

## **Author**
Developed for **Operating Systems Project**.
Contact: reyhunskhosravi@gmail.com
