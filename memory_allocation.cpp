/*
 * CPSC 351 - Group Presentation: Main Memory Allocation Simulation
 * Simulates First-Fit, Best-Fit, and Worst-Fit memory allocation algorithms.
 *
 * Group Members & Responsibilities:
 *   1. Rolando Perez       - Introduction, memory concepts, partition setup display
 *   2. Leonardo Cristofaro - First-Fit algorithm demo
 *   3. Albert Tran         - Best-Fit algorithm demo
 *   4. Joshua Duenas       - Worst-Fit algorithm demo + full comparison
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
// SHARED DATA STRUCTURES & UTILITIES
// These are used by every section — do not modify unless everyone agrees.
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

// Builds a fresh list of unoccupied partitions from the given sizes.
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

// Prints the current state of every partition as a formatted table.
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
        int    used = p.size - p.remaining;
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

// Prints internal fragmentation (waste inside used partitions) and total free space.
void calculateFragmentation(const vector<Partition>& partitions, const string& label) {
    int internalFrag = 0, freeSpace = 0;
    for (const Partition& p : partitions) {
        if (!p.process.empty()) internalFrag += p.remaining;
        else                    freeSpace    += p.remaining;
    }
    cout << "\n  Fragmentation Report [" << label << "]:\n";
    cout << "    Internal fragmentation (wasted space in used partitions) : "
         << internalFrag << " units\n";
    cout << "    Free space in unallocated partitions                     : "
         << freeSpace    << " units\n";
}

// Pauses output until the presenter presses Enter — use between steps during live demo.
void pressEnterToContinue() {
    cout << "\n  [Press ENTER to continue...]\n";
    cin.ignore();
    cin.get();
}


// =============================================================================
// SECTION 1 — ROLANDO PEREZ
// Topic: Introduction to Main Memory & Memory Partitioning
// =============================================================================
//
// WHAT YOU ARE RESPONSIBLE FOR PRESENTING:
//
//   A) VERBAL OVERVIEW (say this before running the program):
//      - Explain that main memory (RAM) is a shared resource. When multiple
//        processes are running, the OS must divide memory and hand out pieces
//        to each process — this is called memory allocation.
//      - Define the two partition types:
//          Fixed partitions  — block sizes decided at boot, never change.
//                              Simple but can waste space if a process is small.
//          Dynamic partitions — created on demand per process size.
//                              More flexible but harder to manage.
//      - Tell the audience this simulation uses FIXED partitions.
//      - Briefly name all three algorithms (First-Fit, Best-Fit, Worst-Fit)
//        and say your teammates will each run one.
//
//   B) RUNNING THE DEMO (select option 1 from the menu):
//      - The program will print the partition table with all slots empty.
//      - Point to the "Free" column — every partition shows its full size as
//        free because nothing has been allocated yet.
//      - Point to "Process = ---" meaning no process is loaded.
//      - Show the process list and explain: each process needs a certain amount
//        of memory; the algorithms decide WHICH partition gets each process.
//      - End by saying: "Leonardo will now show how First-Fit decides."
//
//   WHAT TO KNOW IF ASKED A QUESTION:
//      - Why fixed partitions? Simpler to implement and reason about for class.
//      - What happens if a process is smaller than its partition?
//        The leftover space is wasted — that is internal fragmentation.
//      - What happens if no partition is large enough?
//        The process is rejected (we will see this happen in the demos).
//
// =============================================================================

void rolandoIntro(const vector<int>& partitionSizes, const vector<Process>& processes) {
    printHeader("SECTION 1 - ROLANDO PEREZ: Introduction to Main Memory & Partitioning");

    cout << R"(
  OVERVIEW:
  Main memory (RAM) must be shared among all active processes.
  The OS divides memory into partitions and allocates them to processes.

  PARTITION TYPES:
    Fixed   -- sizes are set at system startup and never change.
               Simple to implement; may waste space (internal fragmentation).
    Dynamic -- created on the fly to match each process's exact size.
               More flexible; this simulation uses FIXED partitions.

  ALGORITHMS WE WILL COMPARE:
    First-Fit  -- scan from the start, pick the FIRST partition big enough
    Best-Fit   -- scan all partitions, pick the SMALLEST one that fits
    Worst-Fit  -- scan all partitions, pick the LARGEST one available
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
    - Partition sizes = the fixed memory blocks available in the system
    - Process sizes   = how much memory each process requests
    - REJECTED        = printed when no partition can fit the process
)";
}


// =============================================================================
// SECTION 2 — LEONARDO CRISTOFARO
// Topic: First-Fit Memory Allocation
// =============================================================================
//
// WHAT YOU ARE RESPONSIBLE FOR PRESENTING:
//
//   A) VERBAL EXPLANATION (say this before running the simulation):
//      - Explain the First-Fit strategy in plain English:
//          "First-Fit scans the partition list from the very beginning —
//           partition 1, then 2, then 3 — and stops the moment it finds
//           a partition with enough free space. It places the process there
//           without looking any further."
//      - Why is this fast? Because it does the minimum work — as soon as a
//        fit is found, the search ends. No need to evaluate every partition.
//      - What is the downside? The front partitions fill up first and develop
//        small leftover gaps. Over time these small holes near the front are
//        too small to hold new processes — this is called fragmentation.
//
//   B) RUNNING THE DEMO (select option 2 from the menu):
//      - As each process is allocated, narrate what the algorithm did:
//          "P1 needs 212 units. Partition 1 only has 100 — too small.
//           Partition 2 has 500 — that fits, so First-Fit stops here."
//      - When a process is REJECTED, explain:
//          "No single partition has enough remaining space, even though
//           there may be enough total free memory spread across partitions.
//           First-Fit cannot combine fragments — this is fragmentation."
//      - After all processes, point to the fragmentation report and explain
//        what internal fragmentation means: space inside a used partition
//        that is allocated but going unused.
//
//   WHAT TO KNOW IF ASKED A QUESTION:
//      - Why does First-Fit leave fragmentation at the front?
//        Because it always starts scanning from partition 1, so the first
//        few partitions get used most often and develop the most holes.
//      - Is First-Fit ever better than Best-Fit?
//        Yes — it is faster and in practice performs similarly to Best-Fit
//        for many workloads, making it the most commonly used algorithm.
//
// =============================================================================

bool firstFitAllocate(vector<Partition>& partitions,
                      const string& name, int size) {
    // Scan from the beginning; stop at the FIRST partition with enough space.
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
    printHeader("SECTION 2 - LEONARDO CRISTOFARO: First-Fit Memory Allocation");

    cout << R"(
  FIRST-FIT ALGORITHM:
    Scans partitions from the beginning (partition 1 onward).
    Allocates the process to the FIRST partition with enough free space.
    Stops searching immediately once a fit is found -- makes it the fastest.
    Downside: small leftover fragments accumulate near the front over time.
)";

    vector<Partition> partitions = initializePartitions(partitionSizes);
    vector<string> satisfied, rejected;

    for (const Process& proc : processes) {
        cout << "\n  >> Allocating " << proc.name
             << " (needs " << proc.size << " units)\n";
        cout << "     Scanning from partition 1...\n";

        if (firstFitAllocate(partitions, proc.name, proc.size)) {
            satisfied.push_back(proc.name);
            cout << "     SATISFIED -- " << proc.name
                 << " placed in first available partition (First-Fit)\n";
        } else {
            rejected.push_back(proc.name);
            cout << "     REJECTED  -- scanned all partitions, none have "
                 << proc.size << " units free for " << proc.name << "\n";
        }
        displayMemoryState(partitions, "First-Fit");
    }

    printResultSummary(satisfied, rejected, "First-Fit");
    calculateFragmentation(partitions, "First-Fit");
    return partitions;
}


// =============================================================================
// SECTION 3 — ALBERT TRAN
// Topic: Best-Fit Memory Allocation
// =============================================================================
//
// WHAT YOU ARE RESPONSIBLE FOR PRESENTING:
//
//   A) VERBAL EXPLANATION (say this before running the simulation):
//      - Explain the Best-Fit strategy:
//          "Best-Fit does NOT stop at the first partition that fits.
//           Instead it scans EVERY partition, records which ones are
//           large enough, and then picks the one with the SMALLEST
//           remaining space that still fits the process."
//      - Why? The goal is to waste as little space as possible per allocation.
//        If a process needs 200 units and we have a 210-unit partition and a
//        500-unit partition, Best-Fit picks the 210 because it wastes only 10
//        instead of 300.
//      - What is the downside? Two things:
//          1. Slower — must always scan every partition.
//          2. Tiny leftover fragments — the "small hole" problem. After many
//             allocations, you are left with lots of 5-, 10-, 20-unit holes
//             that are too small for any real process.
//
//   B) RUNNING THE DEMO (select option 3 from the menu):
//      - Use the SAME partition sizes and processes as Leonardo used.
//        This allows a direct, fair comparison.
//      - As each process is allocated, narrate the decision:
//          "P1 needs 212 units. Partitions large enough are: 300 (88 leftover),
//           500 (288 leftover), 600 (388 leftover). Best-Fit picks 300
//           because it leaves the smallest remainder of 88."
//      - Compare the "Free" column to what First-Fit left behind — point out
//        the smaller remainders in Best-Fit's table.
//      - Point out any very small leftover values (like 2 or 83) and say:
//          "That 2-unit hole will never be used again — that is the small
//           fragment problem Best-Fit is known for."
//
//   WHAT TO KNOW IF ASKED A QUESTION:
//      - Does Best-Fit always satisfy more requests than First-Fit?
//        Not necessarily — it depends on the input. In this demo they tie.
//        But Best-Fit's tiny fragments can actually cause MORE rejections
//        over a longer sequence of processes.
//      - When is Best-Fit preferred?
//        When process sizes are predictable and similar, so you can plan
//        partition usage tightly with minimal waste per slot.
//
// =============================================================================

bool bestFitAllocate(vector<Partition>& partitions,
                     const string& name, int size) {
    // Scan ALL partitions; pick the SMALLEST one that still fits the process.
    int bestIndex     = -1;
    int bestRemaining = INT_MAX;

    for (int i = 0; i < (int)partitions.size(); i++) {
        if (partitions[i].process.empty() && partitions[i].remaining >= size) {
            if (partitions[i].remaining < bestRemaining) {
                bestRemaining = partitions[i].remaining;
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
    printHeader("SECTION 3 - ALBERT TRAN: Best-Fit Memory Allocation");

    cout << R"(
  BEST-FIT ALGORITHM:
    Scans ALL partitions before making a decision.
    Allocates the process to the SMALLEST partition that still fits.
    Minimizes wasted space per allocation -- but requires a full scan each time.
    Known problem: leaves tiny leftover fragments that are too small to reuse.
)";

    vector<Partition> partitions = initializePartitions(partitionSizes);
    vector<string> satisfied, rejected;

    for (const Process& proc : processes) {
        cout << "\n  >> Allocating " << proc.name
             << " (needs " << proc.size << " units)\n";
        cout << "     Scanning all partitions for the smallest fit...\n";

        if (bestFitAllocate(partitions, proc.name, proc.size)) {
            satisfied.push_back(proc.name);
            cout << "     SATISFIED -- " << proc.name
                 << " placed in the smallest fitting partition (Best-Fit)\n";
        } else {
            rejected.push_back(proc.name);
            cout << "     REJECTED  -- no partition has " << proc.size
                 << " units free for " << proc.name << "\n";
        }
        displayMemoryState(partitions, "Best-Fit");
    }

    printResultSummary(satisfied, rejected, "Best-Fit");
    calculateFragmentation(partitions, "Best-Fit");
    return partitions;
}


// =============================================================================
// SECTION 4 — JOSHUA DUENAS
// Topic: Worst-Fit Allocation & Algorithm Comparison
// =============================================================================
//
// WHAT YOU ARE RESPONSIBLE FOR PRESENTING:
//
//   A) VERBAL EXPLANATION — Worst-Fit (say this before the simulation):
//      - Explain the Worst-Fit strategy:
//          "Worst-Fit is the opposite of Best-Fit. It also scans every
//           partition, but instead of picking the smallest fit, it picks
//           the LARGEST available partition every time."
//      - Why would anyone do this? The reasoning is:
//          "If we always use the biggest partition, we leave behind the
//           biggest possible remainder. A big remainder is more likely to
//           fit a future large process than a tiny leftover fragment."
//      - When does this logic hold? When incoming processes are large and
//        varied in size. Worst-Fit avoids the tiny-fragment problem of Best-Fit.
//      - When does it fail? On uniform or small-process workloads, using the
//        biggest partition every time can exhaust large partitions quickly,
//        leaving no room for genuinely large future processes.
//
//   B) RUNNING THE DEMO (select option 4 from the menu):
//      - Use the SAME inputs as the other two demos.
//      - Narrate each decision:
//          "P1 needs 212 units. The largest available partition is 600.
//           Worst-Fit picks it and leaves 388 units — a big remainder."
//      - After each step, note the "Free" values are larger than Best-Fit's.
//      - When REJECTED appears, explain:
//          "Even though large partitions exist, they have been partially used
//           and no single partition has enough contiguous space left."
//
//   C) COMPARISON (runs automatically after Worst-Fit):
//      - Walk through the comparison table line by line:
//          - "Allocated" shows how many processes each algorithm placed.
//          - "Int. Frag" is the total wasted space inside used partitions.
//            Lower is better — Best-Fit wins here.
//          - "Free Space" is space in partitions that were never touched.
//      - Summarize: "No algorithm is universally best. First-Fit is fastest,
//        Best-Fit wastes the least per slot, and Worst-Fit preserves large
//        blocks for future big processes. The right choice depends on the
//        expected workload."
//
//   WHAT TO KNOW IF ASKED A QUESTION:
//      - Why does Worst-Fit sometimes satisfy fewer requests?
//        It burns through the large partitions early, leaving only small
//        ones that cannot fit medium or large processes later.
//      - What is external vs internal fragmentation?
//        Internal — wasted space INSIDE an allocated partition (shown here).
//        External — free space spread across many partitions that cannot be
//        combined to serve a large request (also visible in these demos).
//
// =============================================================================

bool worstFitAllocate(vector<Partition>& partitions,
                      const string& name, int size) {
    // Scan ALL partitions; pick the LARGEST one available.
    int worstIndex     = -1;
    int worstRemaining = -1;

    for (int i = 0; i < (int)partitions.size(); i++) {
        if (partitions[i].process.empty() && partitions[i].remaining >= size) {
            if (partitions[i].remaining > worstRemaining) {
                worstRemaining = partitions[i].remaining;
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
    printHeader("SECTION 4 - JOSHUA DUENAS: Worst-Fit Memory Allocation");

    cout << R"(
  WORST-FIT ALGORITHM:
    Scans ALL partitions before making a decision.
    Allocates the process to the LARGEST available partition.
    Intentionally leaves the biggest possible remainder for future processes.
    Useful when future requests are expected to be large and varied.
)";

    vector<Partition> partitions = initializePartitions(partitionSizes);
    vector<string> satisfied, rejected;

    for (const Process& proc : processes) {
        cout << "\n  >> Allocating " << proc.name
             << " (needs " << proc.size << " units)\n";
        cout << "     Scanning all partitions for the largest available...\n";

        if (worstFitAllocate(partitions, proc.name, proc.size)) {
            satisfied.push_back(proc.name);
            cout << "     SATISFIED -- " << proc.name
                 << " placed in the largest available partition (Worst-Fit)\n";
        } else {
            rejected.push_back(proc.name);
            cout << "     REJECTED  -- no partition has " << proc.size
                 << " units free for " << proc.name << "\n";
        }
        displayMemoryState(partitions, "Worst-Fit");
    }

    printResultSummary(satisfied, rejected, "Worst-Fit");
    calculateFragmentation(partitions, "Worst-Fit");
    return partitions;
}

// Comparison table — called by Joshua after Worst-Fit completes.
void joshuaComparison(const vector<Partition>& ffPartitions,
                      const vector<Partition>& bfPartitions,
                      const vector<Partition>& wfPartitions) {
    printHeader("SECTION 4 (cont.) - JOSHUA DUENAS: Algorithm Comparison");

    cout << R"(
  SIDE-BY-SIDE COMPARISON
  Internal fragmentation = wasted space inside allocated (used) partitions
  Free space             = space remaining in partitions never assigned
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
    First-Fit  -- Fastest (stops at first fit). Moderate fragmentation
                  concentrated near the front of memory.

    Best-Fit   -- Least wasted space per allocation. Slowest (full scan
                  every time). Produces the most tiny unusable fragments.

    Worst-Fit  -- Preserves large contiguous blocks for future requests.
                  Performs poorly when all partitions are similar in size.

  CONCLUSION:
    No algorithm is universally best. The right choice depends on whether
    speed, low fragmentation, or handling large future requests matters most.
)";
}


// =============================================================================
// MENU
// =============================================================================

void printMenu() {
    cout << "\n" << string(60, '=') << "\n";
    cout << "  CPSC 351 -- Memory Allocation Simulation\n";
    cout << string(60, '=') << "\n";
    cout << "  Select a section to run:\n\n";
    cout << "    1  (Rolando Perez)       -- Introduction & Memory Setup\n";
    cout << "    2  (Leonardo Cristofaro) -- First-Fit Algorithm\n";
    cout << "    3  (Albert Tran)         -- Best-Fit Algorithm\n";
    cout << "    4  (Joshua Duenas)       -- Worst-Fit + Full Comparison\n";
    cout << "    5                        -- Run All Sections in Order\n";
    cout << "    0                        -- Exit\n\n";
    cout << "  Enter choice: ";
}


// =============================================================================
// MAIN
// =============================================================================

int main() {
    // -------------------------------------------------------------------------
    // INPUT: Modify these values to change the demo scenario.
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

    // Cache results so the comparison table in option 4 can use them even if
    // only Worst-Fit is run fresh; fall back to running all three when needed.
    vector<Partition> ffResult, bfResult, wfResult;
    bool ffRan = false, bfRan = false, wfRan = false;

    int choice = -1;
    while (choice != 0) {
        printMenu();
        cin >> choice;

        switch (choice) {
            case 1:
                rolandoIntro(partitionSizes, processes);
                break;

            case 2:
                ffResult = leonardoFirstFit(partitionSizes, processes);
                ffRan    = true;
                break;

            case 3:
                bfResult = albertBestFit(partitionSizes, processes);
                bfRan    = true;
                break;

            case 4:
                // Run all three algorithms if any haven't been run yet,
                // so the comparison table always has complete data.
                if (!ffRan) { ffResult = leonardoFirstFit(partitionSizes, processes); ffRan = true; }
                if (!bfRan) { bfResult = albertBestFit(partitionSizes, processes);    bfRan = true; }
                wfResult = joshuaWorstFit(partitionSizes, processes);
                wfRan    = true;
                joshuaComparison(ffResult, bfResult, wfResult);
                break;

            case 5:
                rolandoIntro(partitionSizes, processes);
                ffResult = leonardoFirstFit(partitionSizes, processes); ffRan = true;
                bfResult = albertBestFit(partitionSizes, processes);    bfRan = true;
                wfResult = joshuaWorstFit(partitionSizes, processes);   wfRan = true;
                joshuaComparison(ffResult, bfResult, wfResult);
                break;

            case 0:
                cout << "\n  Exiting simulation. Good luck on the presentation!\n\n";
                break;

            default:
                cout << "\n  Invalid choice. Please enter 0-5.\n";
                break;
        }
    }

    return 0;
}
