// priority.cpp
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

bool comparePriority(const Process& a, const Process& b) {
    return a.priority < b.priority;  // Lower number = higher priority
}

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
    std::vector<std::pair<std::string, int>> gantt;
    int current_time = 0;
    std::vector<int> done(processes.size(), 0);
    int completed = 0;

    while (completed < (int)processes.size()) {
        int idx = -1;
        int bestP = 1e9;
        for (int i = 0; i < (int)processes.size(); ++i)
            if (!done[i] && processes[i].arrival_time <= current_time &&
                processes[i].priority < bestP)
                bestP = processes[i].priority, idx = i;

        if (idx == -1) {
            int next_arr = 1e9;
            for (int i = 0; i < (int)processes.size(); ++i)
                if (!done[i]) next_arr = std::min(next_arr, processes[i].arrival_time);
            gantt.push_back({"IDLE", next_arr - current_time});
            current_time = next_arr;
            continue;
        }

        auto& p = processes[idx];
        gantt.push_back({p.id, p.burst_time});
        current_time += p.burst_time;
        p.turnaround_time = current_time - p.arrival_time;
        p.waiting_time = p.turnaround_time - p.burst_time;
        done[idx] = 1;
        completed++;
    }

    calculateMetrics(processes, current_time);
    printGantt(gantt);
    return 0;
}
