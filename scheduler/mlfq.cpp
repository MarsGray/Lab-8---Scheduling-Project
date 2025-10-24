// mlfq.cpp
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

    for (auto& p : processes) p.remaining_time = p.burst_time;
    std::vector<std::queue<int>> queues(3);
    std::vector<int> quanta = {2, 4, 8};
    for (int i = 0; i < (int)processes.size(); ++i) queues[0].push(i);

    std::vector<std::pair<std::string, int>> gantt;
    int current_time = 0;
    std::vector<int> level(processes.size(), 0);
    std::vector<int> last_wait(processes.size(), 0);

    auto promote_waiting = [&]() {
        for (int i = 0; i < (int)processes.size(); ++i)
            if (processes[i].remaining_time > 0 && level[i] > 0 &&
                current_time - last_wait[i] >= 10) {
                level[i]--;
                queues[level[i]].push(i);
                last_wait[i] = current_time;
            }
    };

    while (true) {
        int q = -1;
        for (int l = 0; l < 3; ++l)
            if (!queues[l].empty()) {
                q = l;
                break;
            }
        if (q == -1) break;

        int i = queues[q].front();
        queues[q].pop();
        auto& p = processes[i];
        int slice = std::min(quanta[q], p.remaining_time);
        gantt.push_back({p.id, slice});
        p.remaining_time -= slice;
        current_time += slice;
        promote_waiting();

        if (p.remaining_time == 0) {
            p.turnaround_time = current_time - p.arrival_time;
            p.waiting_time = p.turnaround_time - p.burst_time;
        } else {
            int next_lvl = std::min(2, q + 1);
            level[i] = next_lvl;
            last_wait[i] = current_time;
            queues[next_lvl].push(i);
        }
    }

    calculateMetrics(processes, current_time);
    printGantt(gantt);
    return 0;
}
