/*
 * CPSC 351 - Group Presentation: Main Memory Allocation Simulation
 * Simulates First-Fit, Best-Fit, and Worst-Fit memory allocation algorithms.
 *
 * Group Members:
 *   Rolando Perez        - Intro, memory setup, partition initialization display
 *   Leonardo Cristofaro  - First-Fit algorithm & demo
 *   Albert Tran          - Best-Fit algorithm & demo
 *   Joshua Duenas        - Worst-Fit algorithm & comparison
 *
 * Compile: g++ -o memory_allocation memory_allocation.cpp
 * Run:     ./memory_allocation
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <climits>

using namespace std;

// =============================================================================
// SHARED DATA STRUCTURES & UTILITIES — Used by all group members
// =============================================================================

struct Partition {
    int    id;
    int    size;
    int    remaining;
    string process;   // empty string = unoccupied
};

struct Process {
    string name;
    int    size;
};

vector<Partition> initializePartitions(const vector<int>& sizes) {
    vector<Partition> partitions;
    for (int i = 0; i < (int)sizes.size(); i++) {
        Partition p;
        p.id        = i + 1;
        p.size      = sizes[i];
        p.remaining = sizes[i];
        p.process   = "";
        partitions.push_back(p);
    }
    return partitions;
}

void displayMemoryState(const vector<Partition>& partitions, const string& label) {
    cout << "\n  Memory State [" << label << "]\n";
    cout << "  " << setw(5)  << right << "Part"
         << "  " << setw(6)  << right << "Total"
         << "  " << setw(6)  << right << "Used"
         << "  " << setw(6)  << right << "Free"
         << "  " << "Process" << "\n";
    cout << "  " << string(5,'-')
         << "  " << string(6,'-')
         << "  " << string(6,'-')
         << "  " << string(6,'-')
         << "  " << string(10,'-') << "\n";

    for (const Partition& p : partitions) {
        int used = p.size - p.remaining;
        string proc = p.process.empty() ? "---" : p.process;
        cout << "  " << setw(5)  << right << p.id
             << "  " << setw(6)  << right << p.size
             << "  " << setw(6)  << right << used
             << "  " << setw(6)  << right << p.remaining
             << "  " << proc << "\n";
    }
}

void printHeader(const string& title) {
    cout << "\n" << string(60, '=') << "\n";
    cout << "  " << title << "\n";
    cout << string(60, '=') << "\n";
}

void printResultSummary(const vector<string>& satisfied,
                        const vector<string>& rejected,
                        const string& algorithm) {
    int total = (int)(satisfied.size() + rejected.size());
    cout << "\n  Summary for " << algorithm << ":\n";
    cout << "    Satisfied : " << satisfied.size() << "/" << total << " requests\n";
    cout << "    Rejected  : " << rejected.size()  << "/" << total << " requests\n";
    if (!rejected.empty()) {
        cout << "    Rejected processes: ";
        for (int i = 0; i < (int)rejected.size(); i++) {
            cout << rejected[i];
            if (i + 1 < (int)rejected.size()) cout << ", ";
        }
        cout << "\n";
    }
}

void calculateFragmentation(const vector<Partition>& partitions, const string& label) {
    int internalFrag = 0, freeSpace = 0;
    for (const Partition& p : partitions) {
        if (!p.process.empty())
            internalFrag += p.remaining;
        else
            freeSpace += p.remaining;
    }
    cout << "\n  Fragmentation Report [" << label << "]:\n";
    cout << "    Internal fragmentation (wasted space in used partitions) : "
         << internalFrag << " units\n";
    cout << "    Free space in unallocated partitions                     : "
         << freeSpace << " units\n";
}


// =============================================================================
// ROLANDO PEREZ — Introduction & Initialization Display
// =============================================================================
//
// ROLANDO: Your section handles:
//   1. Printing the intro/overview of memory management concepts
//   2. Showing the initial partition layout BEFORE any allocation
//   3. Printing the input (partition sizes and process sizes) so the audience
//      understands what data is being fed into the simulation
//
// HOW TO USE IN YOUR DEMO:
//   - Walk the audience through the printed partition table
//   - Explain what "remaining" means (all free at start) and what "---" means
//     (no process loaded yet)
//   - Briefly describe each algorithm before handing off to Leonardo
//
// =============================================================================

void rolandoIntro(const vector<int>& partitionSizes, const vector<Process>& processes) {
    printHeader("ROLANDO PEREZ - Introduction to Main Memory & Partitioning");

    cout << R"(
  OVERVIEW:
  Main memory (RAM) must be shared among all active processes.
  The OS divides memory into partitions and allocates them to processes.

  PARTITION TYPES:
    Fixed   -- partition sizes are set at system startup and never change
    Dynamic -- partitions are created on the fly as processes arrive
               (this simulation uses fixed partitions)

  ALGORITHMS WE WILL COMPARE:
    First-Fit  -- assign the FIRST partition that is large enough
    Best-Fit   -- assign the SMALLEST partition that is large enough
    Worst-Fit  -- assign the LARGEST partition available
)";

    vector<Partition> partitions = initializePartitions(partitionSizes);
    cout << "\n  Initial Memory Partitions (nothing allocated yet):";
    displayMemoryState(partitions, "Initial State");

    cout << "\n  Processes to Allocate:\n";
    cout << "  " << setw(10) << right << "Process"
         << "  " << setw(6)  << right << "Size" << "\n";
    cout << "  " << string(10,'-') << "  " << string(6,'-') << "\n";
    for (const Process& p : processes)
        cout << "  " << setw(10) << right << p.name
             << "  " << setw(6)  << right << p.size << "\n";

    cout << R"(
  INPUT FORMAT:
    - Partition sizes represent the fixed memory blocks available
    - Process sizes represent how much memory each process needs
    - A process is REJECTED if no eligible partition is large enough
)";
}


// =============================================================================
// LEONARDO CRISTOFARO — First-Fit Algorithm
// =============================================================================
//
// LEONARDO: Your section handles:
//   1. Running the First-Fit allocation algorithm
//   2. Printing which requests are satisfied vs rejected after each step
//   3. Displaying memory state after each allocation
//   4. Pointing out fragmentation forming near the front partitions
//
// HOW TO USE IN YOUR DEMO:
//   - For each process explain: "First-Fit scans from partition 1 forward
//     and picks the very first one big enough -- no further searching."
//   - After a few allocations, point to the front partitions filling up
//     and note that later large processes may not fit there even though
//     total free memory exists (external fragmentation)
//
// TALKING POINTS:
//   - Speed advantage: stops scanning as soon as it finds a fit
//   - Fragmentation risk: small leftover holes accumulate at the front
//
// =============================================================================

bool firstFitAllocate(vector<Partition>& partitions,
                      const string& name, int size) {
    // Scan from the beginning; pick the FIRST partition with enough space
    for (Partition& p : partitions) {
        if (p.process.empty() && p.remaining >= size) {
            p.process    = name;
            p.remaining -= size;
            return true;
        }
    }
    return false;
}

vector<Partition> leonardoFirstFit(const vector<int>& partitionSizes,
                                   const vector<Process>& processes) {
    printHeader("LEONARDO CRISTOFARO - First-Fit Memory Allocation");

    cout << R"(
  FIRST-FIT ALGORITHM:
    Scans memory partitions from the beginning (partition 1 onward).
    Allocates the process to the FIRST partition with enough free space.
    Fast -- stops searching as soon as a fit is found.
    Downside -- small leftover fragments tend to pile up near the front.
)";

    vector<Partition> partitions = initializePartitions(partitionSizes);
    vector<string> satisfied, rejected;

    for (const Process& proc : processes) {
        cout << "\n  >> Allocating " << proc.name
             << " (size=" << proc.size << ")\n";

        if (firstFitAllocate(partitions, proc.name, proc.size)) {
            satisfied.push_back(proc.name);
            cout << "     SATISFIED -- " << proc.name << " placed using First-Fit\n";
        } else {
            rejected.push_back(proc.name);
            cout << "     REJECTED  -- no partition has enough space for "
                 << proc.name << " (size=" << proc.size << ")\n";
        }
        displayMemoryState(partitions, "First-Fit");
    }

    printResultSummary(satisfied, rejected, "First-Fit");
    calculateFragmentation(partitions, "First-Fit");
    return partitions;
}


// =============================================================================
// ALBERT TRAN — Best-Fit Algorithm
// =============================================================================
//
// ALBERT: Your section handles:
//   1. Running the Best-Fit allocation algorithm
//   2. Comparing results to First-Fit using the same inputs
//   3. Highlighting the smaller leftover fragments after each allocation
//
// HOW TO USE IN YOUR DEMO:
//   - Use the SAME partition sizes and processes as Leonardo's demo
//   - After each allocation compare the "Free" column to what First-Fit
//     left behind -- Best-Fit leaves less wasted space per step
//   - Point out tiny leftover fragments (remaining=1 or 2) and explain
//     these become unusable -- the "small hole" problem
//
// TALKING POINTS:
//   - Less wasted space per allocation than First-Fit
//   - Slower -- must scan ALL partitions every time
//   - Creates many tiny leftover fragments that are too small to reuse
//
// =============================================================================

bool bestFitAllocate(vector<Partition>& partitions,
                     const string& name, int size) {
    // Scan ALL partitions; pick the SMALLEST one that still fits
    int bestIndex     = -1;
    int bestRemaining = INT_MAX;

    for (int i = 0; i < (int)partitions.size(); i++) {
        Partition& p = partitions[i];
        if (p.process.empty() && p.remaining >= size) {
            if (p.remaining < bestRemaining) {
                bestRemaining = p.remaining;
                bestIndex     = i;
            }
        }
    }

    if (bestIndex != -1) {
        partitions[bestIndex].process    = name;
        partitions[bestIndex].remaining -= size;
        return true;
    }
    return false;
}

vector<Partition> albertBestFit(const vector<int>& partitionSizes,
                                const vector<Process>& processes) {
    printHeader("ALBERT TRAN - Best-Fit Memory Allocation");

    cout << R"(
  BEST-FIT ALGORITHM:
    Scans ALL memory partitions before deciding.
    Allocates the process to the SMALLEST partition that still fits.
    Trade-off: wastes less space per allocation, but leaves tiny fragments
    that are often too small to be useful for future processes.
)";

    vector<Partition> partitions = initializePartitions(partitionSizes);
    vector<string> satisfied, rejected;

    for (const Process& proc : processes) {
        cout << "\n  >> Allocating " << proc.name
             << " (size=" << proc.size << ")\n";

        if (bestFitAllocate(partitions, proc.name, proc.size)) {
            satisfied.push_back(proc.name);
            cout << "     SATISFIED -- " << proc.name << " placed using Best-Fit\n";
        } else {
            rejected.push_back(proc.name);
            cout << "     REJECTED  -- no partition has enough space for "
                 << proc.name << " (size=" << proc.size << ")\n";
        }
        displayMemoryState(partitions, "Best-Fit");
    }

    printResultSummary(satisfied, rejected, "Best-Fit");
    calculateFragmentation(partitions, "Best-Fit");
    return partitions;
}


// =============================================================================
// JOSHUA DUENAS — Worst-Fit Algorithm & Comparison
// =============================================================================
//
// JOSHUA: Your section handles:
//   1. Running the Worst-Fit allocation algorithm
//   2. Explaining the rationale for choosing the LARGEST partition
//   3. Running the full side-by-side comparison across all three algorithms
//
// HOW TO USE IN YOUR DEMO:
//   - After Worst-Fit runs, the comparison table prints automatically
//   - Point out that Worst-Fit leaves big leftovers -- useful when future
//     processes are expected to be large
//   - In the comparison highlight fragmentation differences visually
//
// TALKING POINTS -- Worst-Fit rationale:
//   - Intentionally picks the LARGEST partition
//   - Leaves a large remainder that can still fit future big processes
//   - Better than Best-Fit when process sizes vary widely
//   - Performs poorly when partition sizes are uniform
//
// COMPARISON TALKING POINTS:
//   - First-Fit : fastest, moderate fragmentation at the front
//   - Best-Fit  : smallest holes, but most tiny unusable fragments
//   - Worst-Fit : large holes remain, good for varied workloads
//
// =============================================================================

bool worstFitAllocate(vector<Partition>& partitions,
                      const string& name, int size) {
    // Scan ALL partitions; pick the LARGEST one available
    int worstIndex     = -1;
    int worstRemaining = -1;

    for (int i = 0; i < (int)partitions.size(); i++) {
        Partition& p = partitions[i];
        if (p.process.empty() && p.remaining >= size) {
            if (p.remaining > worstRemaining) {
                worstRemaining = p.remaining;
                worstIndex     = i;
            }
        }
    }

    if (worstIndex != -1) {
        partitions[worstIndex].process    = name;
        partitions[worstIndex].remaining -= size;
        return true;
    }
    return false;
}

vector<Partition> joshuaWorstFit(const vector<int>& partitionSizes,
                                 const vector<Process>& processes) {
    printHeader("JOSHUA DUENAS - Worst-Fit Memory Allocation");

    cout << R"(
  WORST-FIT ALGORITHM:
    Scans ALL memory partitions before deciding.
    Allocates the process to the LARGEST available partition.
    Intentionally leaves the biggest possible remainder.
    Useful when future processes are expected to be large --
    larger leftovers are more likely to accommodate them.
)";

    vector<Partition> partitions = initializePartitions(partitionSizes);
    vector<string> satisfied, rejected;

    for (const Process& proc : processes) {
        cout << "\n  >> Allocating " << proc.name
             << " (size=" << proc.size << ")\n";

        if (worstFitAllocate(partitions, proc.name, proc.size)) {
            satisfied.push_back(proc.name);
            cout << "     SATISFIED -- " << proc.name << " placed using Worst-Fit\n";
        } else {
            rejected.push_back(proc.name);
            cout << "     REJECTED  -- no partition has enough space for "
                 << proc.name << " (size=" << proc.size << ")\n";
        }
        displayMemoryState(partitions, "Worst-Fit");
    }

    printResultSummary(satisfied, rejected, "Worst-Fit");
    calculateFragmentation(partitions, "Worst-Fit");
    return partitions;
}

void joshuaComparison(const vector<Partition>& ffPartitions,
                      const vector<Partition>& bfPartitions,
                      const vector<Partition>& wfPartitions) {
    printHeader("JOSHUA DUENAS - Algorithm Comparison & Fragmentation Analysis");

    cout << R"(
  SIDE-BY-SIDE COMPARISON
  Internal fragmentation = wasted space inside allocated (used) partitions
  Free space             = space in partitions that were never allocated
)";

    auto fragStats = [](const vector<Partition>& parts,
                        int& internalFrag, int& freeSpace, int& allocated) {
        internalFrag = 0; freeSpace = 0; allocated = 0;
        for (const Partition& p : parts) {
            if (!p.process.empty()) { internalFrag += p.remaining; allocated++; }
            else                      freeSpace    += p.remaining;
        }
    };

    int ffInt, ffFree, ffAlloc;
    int bfInt, bfFree, bfAlloc;
    int wfInt, wfFree, wfAlloc;

    fragStats(ffPartitions, ffInt, ffFree, ffAlloc);
    fragStats(bfPartitions, bfInt, bfFree, bfAlloc);
    fragStats(wfPartitions, wfInt, wfFree, wfAlloc);

    cout << "\n  " << setw(12) << right << "Algorithm"
         << "  " << setw(9)  << right << "Allocated"
         << "  " << setw(9)  << right << "Int. Frag"
         << "  " << setw(10) << right << "Free Space" << "\n";
    cout << "  " << string(12,'-')
         << "  " << string(9,'-')
         << "  " << string(9,'-')
         << "  " << string(10,'-') << "\n";
    cout << "  " << setw(12) << right << "First-Fit"
         << "  " << setw(9)  << right << ffAlloc
         << "  " << setw(9)  << right << ffInt
         << "  " << setw(10) << right << ffFree << "\n";
    cout << "  " << setw(12) << right << "Best-Fit"
         << "  " << setw(9)  << right << bfAlloc
         << "  " << setw(9)  << right << bfInt
         << "  " << setw(10) << right << bfFree << "\n";
    cout << "  " << setw(12) << right << "Worst-Fit"
         << "  " << setw(9)  << right << wfAlloc
         << "  " << setw(9)  << right << wfInt
         << "  " << setw(10) << right << wfFree << "\n";

    cout << R"(
  ANALYSIS:
    First-Fit  -- Fast allocation, moderate fragmentation near the front.
                  Good general-purpose choice for most workloads.

    Best-Fit   -- Minimizes wasted space per allocation.
                  Produces the most tiny unusable fragments over time.
                  Slower due to full scan on every allocation.

    Worst-Fit  -- Leaves the largest remainders after allocation.
                  Best when incoming processes tend to be large and varied.
                  Performs poorly on uniform workloads.

  CONCLUSION:
    No single algorithm is universally best.
    The right choice depends on process size distribution and system needs.
)";
}


// =============================================================================
// MAIN — Entry Point
// Runs all four sections in order: Rolando -> Leonardo -> Albert -> Joshua
// =============================================================================

int main() {
    // -------------------------------------------------------------------------
    // INPUT: Modify these values to change the demo scenario
    // -------------------------------------------------------------------------
    vector<int> partitionSizes = {100, 500, 200, 300, 600};

    vector<Process> processes = {
        {"P1", 212},
        {"P2", 417},
        {"P3", 112},
        {"P4", 426},
        {"P5",  98},
        {"P6", 500},
        {"P7", 300},
    };
    // -------------------------------------------------------------------------

    // ROLANDO — Introduction & initial state display
    rolandoIntro(partitionSizes, processes);

    // LEONARDO — First-Fit demo
    vector<Partition> ffResult = leonardoFirstFit(partitionSizes, processes);

    // ALBERT — Best-Fit demo (same inputs)
    vector<Partition> bfResult = albertBestFit(partitionSizes, processes);

    // JOSHUA — Worst-Fit demo + final comparison
    vector<Partition> wfResult = joshuaWorstFit(partitionSizes, processes);
    joshuaComparison(ffResult, bfResult, wfResult);

    return 0;
}
