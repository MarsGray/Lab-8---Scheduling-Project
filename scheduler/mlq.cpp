// mlq.cpp
// Include common code
#include <queue>
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

    std::queue<int> high, low;
    std::vector<std::pair<std::string, int>> gantt;
    int current_time = 0;
    int quantum_high = 4;

    for (int i = 0; i < (int)processes.size(); ++i)
        (processes[i].priority < 3 ? high : low).push(i);

    while (!high.empty() || !low.empty()) {
        if (!high.empty()) {
            int i = high.front();
            high.pop();
            auto& p = processes[i];
            if (p.remaining_time == 0) p.remaining_time = p.burst_time;
            int slice = std::min(quantum_high, p.remaining_time);
            gantt.push_back({p.id, slice});
            p.remaining_time -= slice;
            current_time += slice;
            if (p.remaining_time == 0) {
                p.turnaround_time = current_time - p.arrival_time;
                p.waiting_time = p.turnaround_time - p.burst_time;
            } else
                high.push(i);
        } else {
            int i = low.front();
            low.pop();
            auto& p = processes[i];
            gantt.push_back({p.id, p.burst_time});
            current_time += p.burst_time;
            p.turnaround_time = current_time - p.arrival_time;
            p.waiting_time = p.turnaround_time - p.burst_time;
        }
    }

    calculateMetrics(processes, current_time);
    printGantt(gantt);
    return 0;
}
