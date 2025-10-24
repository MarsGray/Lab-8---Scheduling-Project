// lottery.cpp
// Include common code
#include <random>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

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

    std::mt19937 gen(42);
    std::vector<std::pair<std::string, int>> gantt;
    int current_time = 0;
    int quantum = 2;
    int remaining_total = 0;
    for (auto& p : processes) remaining_total += p.remaining_time;

    while (remaining_total > 0) {
        std::vector<int> tickets(processes.size(), 0);
        int total_tickets = 0;

        for (int i = 0; i < (int)processes.size(); ++i)
            if (processes[i].remaining_time > 0) {
                int t = std::max(1, 10 / std::max(1, processes[i].priority));
                tickets[i] = t;
                total_tickets += t;
            }

        std::uniform_int_distribution<int> dist(1, total_tickets);
        int draw = dist(gen);
        int winner = -1;
        for (int i = 0, acc = 0; i < (int)processes.size(); ++i) {
            if (tickets[i] == 0) continue;
            acc += tickets[i];
            if (draw <= acc) {
                winner = i;
                break;
            }
        }

        auto& p = processes[winner];
        int slice = std::min(quantum, p.remaining_time);
        gantt.push_back({p.id, slice});
        p.remaining_time -= slice;
        current_time += slice;
        remaining_total -= slice;

        if (p.remaining_time == 0) {
            p.turnaround_time = current_time - p.arrival_time;
            p.waiting_time = p.turnaround_time - p.burst_time;
        }
    }

    calculateMetrics(processes, current_time);
    printGantt(gantt);
    return 0;
}
