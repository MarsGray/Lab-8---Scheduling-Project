// edf.cpp
#include <iostream>
#include <queue>
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

struct EProc : Process {
    int deadline; // We’ll set: arrival + 2*burst
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
    for (auto& e : gantt) std::cout << e.first << "(" << e.second << ") ";
    std::cout << "\n";
}

int main() {
    // Initialize remaining_time = burst_time. Last value is deadline (temp 0).
    std::vector<EProc> procs = {
        {"P1", 0, 8, 2, 8, 0, 0, 0},
        {"P2", 1, 4, 1, 4, 0, 0, 0},
        {"P3", 2, 9, 3, 9, 0, 0, 0},
        {"P4", 3, 5, 4, 5, 0, 0, 0}
    };
    for (auto& p : procs) p.deadline = p.arrival_time + 2 * p.burst_time;

    // Sort once by arrival; we’ll stream arrivals with a pointer (can’t miss any).
    std::sort(procs.begin(), procs.end(),
              [](const EProc& a, const EProc& b){
                  if (a.arrival_time != b.arrival_time) return a.arrival_time < b.arrival_time;
                  return a.id < b.id;
              });

    // Min-heap by (deadline, arrival, index)
    using Key = std::tuple<int,int,int>; // (deadline, arrival, idx)
    struct Cmp {
        bool operator()(const Key& x, const Key& y) const { return x > y; } // min-heap
    };
    std::priority_queue<Key, std::vector<Key>, Cmp> pq;

    std::vector<std::pair<std::string,int>> gantt;
    int n = (int)procs.size();
    int finished = 0;
    int t = 0;
    int ap = 0; // arrival pointer

    auto push_arrivals_up_to = [&](int time){
        while (ap < n && procs[ap].arrival_time <= time) {
            if (procs[ap].remaining_time > 0) {
                pq.emplace(procs[ap].deadline, procs[ap].arrival_time, ap);
            }
            ++ap;
        }
    };

    // Start: bring arrivals at t=0 (or jump to first arrival)
    if (ap < n && procs[ap].arrival_time > 0) {
        gantt.push_back({"IDLE", procs[ap].arrival_time - t});
        t = procs[ap].arrival_time;
    }
    push_arrivals_up_to(t);

    // Run preemptively in 1-unit quanta (simple and correct)
    std::string last = "IDLE";
    int runlen = 0;

    while (finished < n) {
        if (pq.empty()) {
            // Jump time to next arrival (if any)
            if (ap < n) {
                int next_t = procs[ap].arrival_time;
                if (last != "IDLE") {
                    if (runlen) gantt.push_back({last, runlen});
                    last = "IDLE";
                    runlen = 0;
                }
                gantt.push_back({"IDLE", next_t - t});
                t = next_t;
                push_arrivals_up_to(t);
                continue;
            } else {
                // No arrivals left and nothing to run: done
                break;
            }
        }

        auto [dl, arr, i] = pq.top(); pq.pop();

        // Context switch in Gantt if needed
        if (last != procs[i].id) {
            if (runlen) gantt.push_back({last, runlen});
            last = procs[i].id;
            runlen = 0;
        }

        // Run for 1 unit (preemptive EDF)
        procs[i].remaining_time -= 1;
        runlen += 1;
        t += 1;

        // New arrivals during this tick
        push_arrivals_up_to(t);

        if (procs[i].remaining_time == 0) {
            procs[i].turnaround_time = t - procs[i].arrival_time;
            procs[i].waiting_time = procs[i].turnaround_time - procs[i].burst_time;
            finished += 1;
        } else {
            // Recompute (same deadline), re-enqueue
            pq.emplace(procs[i].deadline, procs[i].arrival_time, i);
        }
    }
    if (runlen) gantt.push_back({last, runlen});

    // Print results
    std::vector<Process> cast; cast.reserve(n);
    for (auto& p : procs) cast.push_back(p);
    calculateMetrics(cast, t);
    printGantt(gantt);
    return 0;
}
