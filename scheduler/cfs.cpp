// cfs.cpp
// Include common code
#include <queue>
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

// Add vruntime field to Process via a wrapper
struct CProc : Process {
    double vruntime = 0.0;
};

struct CFSCompare {
    bool operator()(const CProc* a, const CProc* b) const {
        return a->vruntime > b->vruntime; // Min-heap on vruntime
    }
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
    std::vector<CProc> procs = {
        CProc{{"P1", 0, 8, 2, 8, 0, 0}},
        CProc{{"P2", 1, 4, 1, 4, 0, 0}},
        CProc{{"P3", 2, 9, 3, 9, 0, 0}},
        CProc{{"P4", 3, 5, 4, 5, 0, 0}},
    };

    std::priority_queue<CProc*, std::vector<CProc*>, CFSCompare> ready_queue;
    std::vector<std::pair<std::string, int>> gantt;
    int current_time = 0;
    int time_slice = 2;

    auto weight = [](int prio) { return 1.0 / std::max(1, prio); }; // lower prio num = higher weight
    for (auto& p : procs) if (p.arrival_time == 0) ready_queue.push(&p);

    while (true) {
        if (ready_queue.empty()) {
            int next_arr = 1e9;
            for (auto& p : procs)
                if (p.remaining_time > 0)
                    next_arr = std::min(next_arr, p.arrival_time);
            if (next_arr == 1e9) break;
            gantt.push_back({"IDLE", next_arr - current_time});
            current_time = next_arr;
            for (auto& p : procs)
                if (p.arrival_time <= current_time && p.remaining_time > 0)
                    ready_queue.push(&p);
            continue;
        }

        CProc* cp = ready_queue.top();
        ready_queue.pop();
        int run = std::min(time_slice, cp->remaining_time);
        gantt.push_back({cp->id, run});
        cp->remaining_time -= run;
        cp->vruntime += run / weight(cp->priority);
        current_time += run;

        for (auto& p : procs)
            if (p.arrival_time <= current_time && p.remaining_time > 0 && &p != cp)
                ready_queue.push(&p);

        if (cp->remaining_time > 0)
            ready_queue.push(cp);
        else {
            cp->turnaround_time = current_time - cp->arrival_time;
            cp->waiting_time = cp->turnaround_time - cp->burst_time;
        }
    }

    std::vector<Process> casted;
    for (auto& p : procs) casted.push_back(p);
    calculateMetrics(casted, current_time);
    printGantt(gantt);
    return 0;
}
