// srtf.cpp
// Include common code

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

struct Process {
    std::string id;
    int arrival_time;
    int burst_time;
    int priority;
    int remaining_time;
    int waiting_time;
    int turnaround_time;
};

void calculateMetrics(std::vector<Process>& processes, int total_time) {
    double avg_wait = 0, avg_turn = 0;
    for (auto& p : processes) {
        avg_wait += p.waiting_time;
        avg_turn += p.turnaround_time;
    }
    avg_wait /= processes.size();
    avg_turn /= processes.size();
    std::cout << "Avg Waiting Time: " << avg_wait << "\n";
    std::cout << "Avg Turnaround Time: " << avg_turn << "\n";
}

void printGantt(const std::vector<std::pair<std::string, int>>& gantt) {
    std::cout << "Gantt Chart: ";
    for (auto& entry : gantt) std::cout << entry.first << "(" << entry.second << ") ";
    std::cout << "\n";
}

int main() {
    std::vector<Process> processes = {
        {"P1", 0, 8, 2, 8, 0, 0},
        {"P2", 1, 4, 1, 4, 0, 0},
        {"P3", 2, 9, 3, 9, 0, 0},
        {"P4", 3, 5, 4, 5, 0, 0},
    };
    for (auto& p : processes) p.remaining_time = p.burst_time;
    std::vector<std::pair<std::string, int>> gantt;
    int current_time = 0;
    std::string last = "IDLE";
    int last_len = 0;
    int finished = 0;

    while (finished < (int)processes.size()) {
        int idx = -1, best = 1e9;
        for (int i = 0; i < (int)processes.size(); ++i)
            if (processes[i].arrival_time <= current_time && processes[i].remaining_time > 0 &&
                processes[i].remaining_time < best)
                best = processes[i].remaining_time, idx = i;

        if (idx == -1) {
            if (last != "IDLE") {
                if (last_len) gantt.push_back({last, last_len});
                last_len = 0;
                last = "IDLE";
            }
            last_len++;
            current_time++;
            continue;
        }

        if (last != processes[idx].id) {
            if (last_len) gantt.push_back({last, last_len});
            last = processes[idx].id;
            last_len = 0;
        }
        last_len++;
        processes[idx].remaining_time--;
        current_time++;
        if (processes[idx].remaining_time == 0) {
            processes[idx].turnaround_time = current_time - processes[idx].arrival_time;
            processes[idx].waiting_time = processes[idx].turnaround_time - processes[idx].burst_time;
            finished++;
        }
    }
    if (last_len) gantt.push_back({last, last_len});

    calculateMetrics(processes, current_time);
    printGantt(gantt);
    return 0;
}