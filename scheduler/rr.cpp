// rr.cpp
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
    for (auto& entry : gantt)
        std::cout << entry.first << "(" << entry.second << ") ";
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

    std::queue<int> ready_queue;
    std::vector<std::pair<std::string, int>> gantt;
    int current_time = 0;
    int quantum = 4;

    std::stable_sort(processes.begin(), processes.end(),
                     [](const Process& a, const Process& b) { return a.arrival_time < b.arrival_time; });

    int arrived = 0;
    auto add_arrivals = [&](int t) {
        for (; arrived < (int)processes.size(); ++arrived)
            if (processes[arrived].arrival_time <= t)
                ready_queue.push(arrived);
            else
                break;
    };

    add_arrivals(0);
    std::string last = "IDLE";
    int last_len = 0;
    int finished = 0;

    while (finished < (int)processes.size()) {
        if (ready_queue.empty()) {
            int next_arr = 1e9;
            for (auto& p : processes)
                if (p.remaining_time > 0)
                    next_arr = std::min(next_arr, p.arrival_time);
            if (last != "IDLE") {
                if (last_len) gantt.push_back({last, last_len});
                last = "IDLE";
                last_len = 0;
            }
            last_len += (next_arr - current_time);
            current_time = next_arr;
            add_arrivals(current_time);
            continue;
        }

        int i = ready_queue.front();
        ready_queue.pop();
        auto& p = processes[i];
        if (last != p.id) {
            if (last_len) gantt.push_back({last, last_len});
            last = p.id;
            last_len = 0;
        }

        int slice = std::min(quantum, p.remaining_time);
        p.remaining_time -= slice;
        last_len += slice;
        current_time += slice;
        add_arrivals(current_time);

        if (p.remaining_time == 0) {
            p.turnaround_time = current_time - p.arrival_time;
            p.waiting_time = p.turnaround_time - p.burst_time;
            finished++;
        } else {
            ready_queue.push(i);
        }
    }
    if (last_len) gantt.push_back({last, last_len});

    calculateMetrics(processes, current_time);
    printGantt(gantt);
    return 0;
}