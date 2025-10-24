# Lab-8---Scheduling-Project
This simulator models 10 CPU scheduling algorithms in C++ using object-oriented design. It computes Gantt charts, waiting/turnaround times, CPU utilization, and throughput. Enhancements include priority aging, EDF deadlines, I/O simulation, and random task generation.

Implemented Schedulers
FCFS: First Come, First Serve
SJF / SRTF: Shortest Job / Remaining Time First
Priority: Supports aging
Round Robin (RR): Time quantum-based, supports I/O blocking
MLQ / MLFQ: Multi-Level Queue and Feedback Queue
Lottery: Randomized fair scheduling
CFS: Simplified Linux fair-share scheduler
EDF: Earliest Deadline First with deadline misses tracking