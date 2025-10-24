#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <string>
#include <fstream>
#include <random>
#include <chrono>
#include <map>
#include <memory>

struct Process {
    std::string id;
    int arrival_time;
    int burst_time;
    int priority;
    int remaining_time;
    int waiting_time = 0;
    int turnaround_time = 0;
    int deadline = 0;
};

void calculateMetrics(const std::vector<Process>& processes, int total_time, double& avg_wait, double& avg_turn, double& cpu_util, double& throughput) {
    avg_wait = 0;
    avg_turn = 0;
    for (const auto& p : processes) {
        avg_wait += p.waiting_time;
        avg_turn += p.turnaround_time;
    }
    int n = processes.size();
    avg_wait /= n;
    avg_turn /= n;
    cpu_util = (total_time > 0) ? (double)std::accumulate(processes.begin(), processes.end(), 0, [](int sum, const Process& p){return sum + p.burst_time; }) / total_time * 100 : 0;
    throughput = (double)n / total_time;
}

void printGantt(const std::vector<std::pair<std::string, int>>& gantt) {
    std::cout << "Gantt Chart: ";
    for (const auto& entry : gantt) {
        std::cout << entry.first << "(" << entry.second << ") ";
    }
    std::cout << "\n";
}

void printResults(const std::vector<Process>& processes,
                  int total_time,
                  const std::vector<std::pair<std::string, int>>& gantt) {
    double avg_wait, avg_turn, cpu_util, throughput;
    calculateMetrics(processes, total_time, avg_wait, avg_turn, cpu_util, throughput);

    printGantt(gantt);
    std::cout << "Average Waiting Time: " << avg_wait << "\n";
    std::cout << "Average Turnaround Time: " << avg_turn << "\n";
    std::cout << "CPU Utilization: " << cpu_util << "%\n";
    std::cout << "Throughput: " << throughput << " processes/unit time\n";
}

class Scheduler {
public:
    virtual ~Scheduler() = default;
    virtual void schedule(std::vector<Process>& processes,
                          std::vector<std::pair<std::string, int>>& gantt,
                          int& total_time) = 0;
};

class FCFSScheduler : public Scheduler {
public:
    void schedule(std::vector<Process>& procs,
                  std::vector<std::pair<std::string,int>>& gantt,
                  int& total_time) override {
        std::sort(procs.begin(), procs.end(),
                  [](const Process& a, const Process& b){
                      return a.arrival_time < b.arrival_time;
                  });
        int t = 0;
        for (auto& p : procs) {
            if (t < p.arrival_time) { gantt.emplace_back("IDLE", p.arrival_time - t); t = p.arrival_time; }
            gantt.emplace_back(p.id, p.burst_time);
            t += p.burst_time;
            p.turnaround_time = t - p.arrival_time;
            p.waiting_time    = p.turnaround_time - p.burst_time;
        }
        total_time = t;
    }
};

class SJFScheduler : public Scheduler {
public:
    void schedule(std::vector<Process>& procs,
                  std::vector<std::pair<std::string,int>>& gantt,
                  int& total_time) override {
        std::sort(procs.begin(), procs.end(),
                  [](const Process& a, const Process& b){ return a.arrival_time < b.arrival_time; });
        int t = 0, n = (int)procs.size(), done = 0;
        std::vector<int> used(n, 0);
        while (done < n) {
            int idx = -1, best = 1e9;
            for (int i = 0; i < n; ++i)
                if (!used[i] && procs[i].arrival_time <= t && procs[i].burst_time < best)
                    best = procs[i].burst_time, idx = i;
            if (idx == -1) {
                int next_arr = 1e9;
                for (int i = 0; i < n; ++i) if (!used[i]) next_arr = std::min(next_arr, procs[i].arrival_time);
                gantt.emplace_back("IDLE", next_arr - t); t = next_arr; continue;
            }
            auto& p = procs[idx];
            gantt.emplace_back(p.id, p.burst_time);
            t += p.burst_time;
            p.turnaround_time = t - p.arrival_time;
            p.waiting_time    = p.turnaround_time - p.burst_time;
            used[idx] = 1; ++done;
        }
        total_time = t;
    }
};

class SRTFScheduler : public Scheduler {
public:
    void schedule(std::vector<Process>& procs,
                  std::vector<std::pair<std::string,int>>& gantt,
                  int& total_time) override {
        for (auto& p : procs) p.remaining_time = p.burst_time;
        std::sort(procs.begin(), procs.end(),
                  [](const Process& a, const Process& b){ return a.arrival_time < b.arrival_time; });
        int t = 0, n = (int)procs.size(), finished = 0;
        std::string last = "IDLE"; int run = 0;
        while (finished < n) {
            int idx = -1, best = 1e9;
            for (int i = 0; i < n; ++i)
                if (procs[i].arrival_time <= t && procs[i].remaining_time > 0 &&
                    procs[i].remaining_time < best) best = procs[i].remaining_time, idx = i;
            if (idx == -1) {
                if (last != "IDLE") { if (run) gantt.emplace_back(last, run); last = "IDLE"; run = 0; }
                ++t; ++run; continue;
            }
            if (last != procs[idx].id) { if (run) gantt.emplace_back(last, run); last = procs[idx].id; run = 0; }
            ++run; --procs[idx].remaining_time; ++t;
            if (procs[idx].remaining_time == 0) {
                procs[idx].turnaround_time = t - procs[idx].arrival_time;
                procs[idx].waiting_time    = procs[idx].turnaround_time - procs[idx].burst_time;
                ++finished;
            }
        }
        if (run) gantt.emplace_back(last, run);
        total_time = t;
    }
};

class PriorityScheduler : public Scheduler {
public:
    void schedule(std::vector<Process>& procs,
                  std::vector<std::pair<std::string,int>>& gantt,
                  int& total_time) override {
        std::sort(procs.begin(), procs.end(),
                  [](const Process& a, const Process& b){ return a.arrival_time < b.arrival_time; });
        int t = 0, n = (int)procs.size(), done = 0;
        std::vector<int> used(n, 0);
        while (done < n) {
            int idx = -1, bestP = 1e9;
            for (int i = 0; i < n; ++i)
                if (!used[i] && procs[i].arrival_time <= t && procs[i].priority < bestP)
                    bestP = procs[i].priority, idx = i;
            if (idx == -1) {
                int next_arr = 1e9;
                for (int i = 0; i < n; ++i) if (!used[i]) next_arr = std::min(next_arr, procs[i].arrival_time);
                gantt.emplace_back("IDLE", next_arr - t); t = next_arr; continue;
            }
            auto& p = procs[idx];
            gantt.emplace_back(p.id, p.burst_time);
            t += p.burst_time;
            p.turnaround_time = t - p.arrival_time;
            p.waiting_time    = p.turnaround_time - p.burst_time;
            used[idx] = 1; ++done;
        }
        total_time = t;
    }
};

class RoundRobinScheduler : public Scheduler {
private:
    int quantum;

public:
    explicit RoundRobinScheduler(int q) : quantum(q) {}

    void schedule(std::vector<Process>& processes,
                  std::vector<std::pair<std::string, int>>& gantt,
                  int& total_time) override {
        // Initialize remaining time
        for (auto& p : processes) p.remaining_time = p.burst_time;

        // Ensure sorted by arrival for streaming arrivals into ready_queue
        std::stable_sort(processes.begin(), processes.end(),
                         [](const Process& a, const Process& b) {
                             return a.arrival_time < b.arrival_time;
                         });

        std::queue<Process*> ready_queue;
        int current_time = 0;
        int idx = 0;  // index for processes (arrivals)

        // Helper: add all arrivals up to current_time
        auto add_arrivals = [&](int t) {
            while (idx < (int)processes.size() && processes[idx].arrival_time <= t) {
                ready_queue.push(&processes[idx++]);
            }
        };

        add_arrivals(0);

        std::string last_id = "";
        int last_start = -1;

        while (!ready_queue.empty() || idx < (int)processes.size()) {
            if (ready_queue.empty()) {
                // Idle time: advance to next arrival
                if (idx < (int)processes.size()) current_time = processes[idx].arrival_time;
                add_arrivals(current_time);
                continue;
            }

            Process* current = ready_queue.front();
            ready_queue.pop();

            int run_time = std::min(quantum, current->remaining_time);

            // Commit previous segment if context switched
            if (last_id != current->id || last_start == -1) {
                if (last_start != -1) {
                    gantt.emplace_back(last_id, current_time - last_start);
                }
                last_id = current->id;
                last_start = current_time;
            }

            current->remaining_time -= run_time;
            current_time += run_time;

            // Add newly arrived during this slice
            add_arrivals(current_time);

            if (current->remaining_time > 0) {
                ready_queue.push(current);
            } else {
                current->turnaround_time = current_time - current->arrival_time;
                current->waiting_time = current->turnaround_time - current->burst_time;
            }
        }

        if (last_start != -1) {
            gantt.emplace_back(last_id, current_time - last_start);
        }
        total_time = current_time;
    }
};

class MLQScheduler : public Scheduler {
public:
    void schedule(std::vector<Process>& procs,
                  std::vector<std::pair<std::string,int>>& gantt,
                  int& total_time) override {
        int t = 0;
        std::queue<int> high, low;
        for (int i = 0; i < (int)procs.size(); ++i) {
            if (procs[i].priority < 3) high.push(i); else low.push(i);
        }
        for (auto& p : procs) p.remaining_time = p.burst_time;
        while (!high.empty() || !low.empty()) {
            if (!high.empty()) {
                int i = high.front(); high.pop();
                auto& p = procs[i];
                int slice = std::min(4, p.remaining_time);
                gantt.emplace_back(p.id, slice);
                t += slice; p.remaining_time -= slice;
                if (p.remaining_time > 0) high.push(i);
                else { p.turnaround_time = t - p.arrival_time; p.waiting_time = p.turnaround_time - p.burst_time; }
            } else {
                int i = low.front(); low.pop();
                auto& p = procs[i];
                gantt.emplace_back(p.id, p.burst_time);
                t += p.burst_time;
                p.turnaround_time = t - p.arrival_time;
                p.waiting_time    = p.turnaround_time - p.burst_time;
            }
        }
        total_time = t;
    }
};

class MLFQScheduler : public Scheduler {
public:
    void schedule(std::vector<Process>& procs,
                  std::vector<std::pair<std::string,int>>& gantt,
                  int& total_time) override {
        std::vector<std::queue<int>> qs(3);
        std::vector<int> quanta = {2,4,8};
        for (int i = 0; i < (int)procs.size(); ++i) { qs[0].push(i); procs[i].remaining_time = procs[i].burst_time; }
        int t = 0;
        while (true) {
            int lvl = -1;
            for (int l=0;l<3;++l) if (!qs[l].empty()) { lvl = l; break; }
            if (lvl == -1) break;
            int i = qs[lvl].front(); qs[lvl].pop();
            auto& p = procs[i];
            int slice = std::min(quanta[lvl], p.remaining_time);
            gantt.emplace_back(p.id, slice);
            p.remaining_time -= slice; t += slice;
            if (p.remaining_time == 0) {
                p.turnaround_time = t - p.arrival_time;
                p.waiting_time    = p.turnaround_time - p.burst_time;
            } else {
                int nl = std::min(2, lvl+1);
                qs[nl].push(i);
            }
        }
        total_time = t;
    }
};

class LotteryScheduler : public Scheduler {
public:
    void schedule(std::vector<Process>& procs,
                  std::vector<std::pair<std::string,int>>& gantt,
                  int& total_time) override {
        for (auto& p : procs) p.remaining_time = p.burst_time;
        std::mt19937 gen(42);
        int t = 0;
        int left = 0; for (auto& p : procs) left += p.remaining_time;
        while (left > 0) {
            std::vector<int> tickets(procs.size(),0);
            int total = 0;
            for (int i=0;i<(int)procs.size();++i) if (procs[i].remaining_time>0) {
                int tk = std::max(1, 10 / std::max(1, procs[i].priority));
                tickets[i] = tk; total += tk;
            }
            std::uniform_int_distribution<int> dist(1,total);
            int draw = dist(gen), winner=-1, acc=0;
            for (int i=0;i<(int)procs.size();++i) { if (!tickets[i]) continue; acc+=tickets[i]; if (draw<=acc){ winner=i; break; } }
            auto& p = procs[winner];
            int slice = std::min(2, p.remaining_time);
            gantt.emplace_back(p.id, slice);
            p.remaining_time -= slice; t += slice; left -= slice;
            if (p.remaining_time == 0) {
                p.turnaround_time = t - p.arrival_time;
                p.waiting_time    = p.turnaround_time - p.burst_time;
            }
        }
        total_time = t;
    }
};

class CFSScheduler : public Scheduler {
    struct CProc { int idx; double vruntime=0.0; };
    struct Cmp { bool operator()(const CProc& a, const CProc& b) const { return a.vruntime > b.vruntime; } };
public:
    void schedule(std::vector<Process>& procs,
                  std::vector<std::pair<std::string,int>>& gantt,
                  int& total_time) override {
        for (auto& p : procs) p.remaining_time = p.burst_time;
        std::priority_queue<CProc,std::vector<CProc>,Cmp> pq;
        for (int i=0;i<(int)procs.size();++i) pq.push({i,0.0});
        int t = 0;
        auto weight = [&](int pr){ return 1.0 / std::max(1, pr); };
        while (!pq.empty()) {
            auto cp = pq.top(); pq.pop();
            int i = cp.idx; auto& p = procs[i];
            if (p.remaining_time==0) continue;
            int slice = std::min(2, p.remaining_time);
            gantt.emplace_back(p.id, slice);
            p.remaining_time -= slice; t += slice;
            cp.vruntime += slice / weight(p.priority);
            if (p.remaining_time > 0) pq.push(cp);
            else { p.turnaround_time = t - p.arrival_time; p.waiting_time = p.turnaround_time - p.burst_time; }
        }
        total_time = t;
    }
};

class EDFScheduler : public Scheduler {
public:
    void schedule(std::vector<Process>& procs,
                  std::vector<std::pair<std::string,int>>& gantt,
                  int& total_time) override {
        for (auto& p : procs) { p.remaining_time = p.burst_time; if (p.deadline==0) p.deadline = p.arrival_time + 2*p.burst_time; }
        std::sort(procs.begin(), procs.end(),
                  [](const Process& a, const Process& b){
                      if (a.arrival_time != b.arrival_time) return a.arrival_time < b.arrival_time;
                      return a.id < b.id;
                  });
        using Key = std::tuple<int,int,int>; // (deadline, arrival, index)
        struct Cmp { bool operator()(const Key& x, const Key& y) const { return x > y; } };
        std::priority_queue<Key,std::vector<Key>,Cmp> pq;
        int n = (int)procs.size(), finished = 0, t = 0, ap = 0;
        auto push_arrivals_up_to = [&](int time){
            while (ap < n && procs[ap].arrival_time <= time) {
                if (procs[ap].remaining_time > 0) pq.emplace(procs[ap].deadline, procs[ap].arrival_time, ap);
                ++ap;
            }
        };
        if (ap < n && procs[ap].arrival_time > 0) { gantt.emplace_back("IDLE", procs[ap].arrival_time - t); t = procs[ap].arrival_time; }
        push_arrivals_up_to(t);

        std::string last = "IDLE"; int run = 0;

        while (finished < n) {
            if (pq.empty()) {
                if (ap < n) {
                    int next_t = procs[ap].arrival_time;
                    if (last != "IDLE") { if (run) gantt.emplace_back(last, run); last="IDLE"; run=0; }
                    gantt.emplace_back("IDLE", next_t - t);
                    t = next_t;
                    push_arrivals_up_to(t);
                    continue;
                } else break;
            }
            auto [dl, arr, i] = pq.top(); pq.pop();
            if (last != procs[i].id) { if (run) gantt.emplace_back(last, run); last = procs[i].id; run = 0; }
            // run 1 unit
            --procs[i].remaining_time; ++run; ++t;
            push_arrivals_up_to(t);
            if (procs[i].remaining_time == 0) {
                procs[i].turnaround_time = t - procs[i].arrival_time;
                procs[i].waiting_time    = procs[i].turnaround_time - procs[i].burst_time;
                ++finished;
            } else {
                pq.emplace(procs[i].deadline, procs[i].arrival_time, i);
            }
        }
        if (run) gantt.emplace_back(last, run);
        total_time = t;
    }
};

std::vector<Process> loadProcesses(const std::string& filename) {
    std::vector<Process> procs;
    std::ifstream file(filename);
    if (!file) { std::cerr << "Error opening file: " << filename << "\n"; return procs; }
    std::string id; int at, bt, pri, dl = 0;
    // Accept 4 or 5 fields per line: ID arrival burst priority [deadline]
    std::string line;
    while (true) {
        if (!(file >> id >> at >> bt >> pri)) break;
        if (file.peek()=='\n' || file.peek()==EOF) dl = 0;
        else {
            // try to read optional deadline
            std::streampos pos = file.tellg();
            if (file >> dl) { /* ok */ }
            else { file.clear(); file.seekg(pos); dl = 0; }
        }
        procs.push_back({id, at, bt, pri, bt, 0, 0, dl});
    }
    std::sort(procs.begin(), procs.end(),
              [](const Process& a, const Process& b){ return a.arrival_time < b.arrival_time; });
    return procs;
}

std::vector<Process> generateRandomProcesses(int num) {
    std::vector<Process> procs;
    std::mt19937 gen((unsigned)std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> A(0, 20), B(1, 10), P(1, 5);
    for (int i=0;i<num;++i) {
        std::string id = "P" + std::to_string(i+1);
        int at = A(gen), bt = B(gen), pri = P(gen);
        procs.push_back({id, at, bt, pri, bt, 0, 0, 0});
    }
    std::sort(procs.begin(), procs.end(),
              [](const Process& a, const Process& b){ return a.arrival_time < b.arrival_time; });
    return procs;
}

int main(int argc, char* argv[]) {
    // Parse very simply: --flag value
    std::map<std::string,std::string> args;
    for (int i=1;i<argc;i+=2) args[argv[i]] = (i+1<argc)? argv[i+1] : "";

    std::string scheduler_type = args["--scheduler"];
    std::string input_file     = args["--input"];
    int  quantum               = args["--quantum"].empty()? 4 : std::stoi(args["--quantum"]);
    bool random                = (args["--random"]=="--random" || args["--random"]=="true");
    int  num_rand              = args["--num"].empty()? 10 : std::stoi(args["--num"]);
    std::string output_path    = args["--output"];

    // Load processes
    std::vector<Process> processes;
    if (random) processes = generateRandomProcesses(num_rand);
    else if (!input_file.empty()) processes = loadProcesses(input_file);
    else {
        processes = {
            {"P1", 0, 8, 2, 8, 0, 0, 0},
            {"P2", 1, 4, 1, 4, 0, 0, 0},
            {"P3", 2, 9, 3, 9, 0, 0, 0},
            {"P4", 3, 5, 4, 5, 0, 0, 0},
        };
        std::sort(processes.begin(), processes.end(),
                  [](const Process& a, const Process& b){ return a.arrival_time < b.arrival_time; });
    }
    if (processes.empty()) { std::cerr << "No processes loaded.\n"; return 1; }

    // Instantiate chosen scheduler
    std::unique_ptr<Scheduler> scheduler;
    if      (scheduler_type == "rr")      scheduler = std::make_unique<RoundRobinScheduler>(quantum);
    else if (scheduler_type == "fcfs")    scheduler = std::make_unique<FCFSScheduler>();
    else if (scheduler_type == "sjf")     scheduler = std::make_unique<SJFScheduler>();
    else if (scheduler_type == "srtf")    scheduler = std::make_unique<SRTFScheduler>();
    else if (scheduler_type == "priority")scheduler = std::make_unique<PriorityScheduler>();
    else if (scheduler_type == "mlq")     scheduler = std::make_unique<MLQScheduler>();
    else if (scheduler_type == "mlfq")    scheduler = std::make_unique<MLFQScheduler>();
    else if (scheduler_type == "lottery") scheduler = std::make_unique<LotteryScheduler>();
    else if (scheduler_type == "cfs")     scheduler = std::make_unique<CFSScheduler>();
    else if (scheduler_type == "edf")     scheduler = std::make_unique<EDFScheduler>();
    else { std::cerr << "Unknown scheduler: " << scheduler_type << "\n"; return 1; }

    // Run simulation
    std::vector<std::pair<std::string,int>> gantt;
    int total_time = 0;
    scheduler->schedule(processes, gantt, total_time);
    printResults(processes, total_time, gantt);

    // Optional log
    if (!output_path.empty()) {
        std::ofstream log(output_path);
        if (log) {
            double avg_wait, avg_turn, cpu_util, throughput;
            calculateMetrics(processes, total_time, avg_wait, avg_turn, cpu_util, throughput);
            log << "Scheduler: " << scheduler_type << "\n";
            log << "Gantt: "; for (auto& e: gantt) log << e.first << "("<<e.second<<") ";
            log << "\n";
            log << "Avg Waiting: " << avg_wait << "\n";
            log << "Avg Turnaround: " << avg_turn << "\n";
            log << "CPU Utilization: " << cpu_util << "%\n";
            log << "Throughput: " << throughput << " processes/unit time\n";
        } else std::cerr << "Could not open output file: " << output_path << "\n";
    }
    return 0;
}