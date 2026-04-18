"""
CPSC 351 - Group Presentation: Main Memory Allocation Simulation
Simulates First-Fit, Best-Fit, and Worst-Fit memory allocation algorithms.

Group Members:
  - Rolando Perez   : Intro, memory setup, partition initialization display
  - Leonardo Cristofaro : First-Fit algorithm & demo
  - Albert Tran     : Best-Fit algorithm & demo
  - Joshua Duenas   : Worst-Fit algorithm & comparison
"""

# =============================================================================
# SHARED SETUP — Used by all group members
# =============================================================================

def initialize_partitions(partition_sizes):
    """
    Creates a list of memory partition dictionaries.
    Each partition tracks its total size, how much is used, and which process occupies it.
    """
    partitions = []
    for i, size in enumerate(partition_sizes):
        partitions.append({
            "id": i + 1,
            "size": size,
            "remaining": size,
            "process": None
        })
    return partitions


def display_memory_state(partitions, algorithm_name):
    """
    Prints the current state of all memory partitions.
    Shows partition ID, total size, space used, space remaining, and which process is loaded.
    """
    print(f"\n  Memory State [{algorithm_name}]")
    print(f"  {'Part':>5}  {'Total':>6}  {'Used':>6}  {'Free':>6}  {'Process'}")
    print(f"  {'-'*5}  {'-'*6}  {'-'*6}  {'-'*6}  {'-'*10}")
    for p in partitions:
        used = p["size"] - p["remaining"]
        proc = p["process"] if p["process"] else "---"
        print(f"  {p['id']:>5}  {p['size']:>6}  {used:>6}  {p['remaining']:>6}  {proc}")


def get_fresh_partitions(partition_sizes):
    """Returns a clean copy of partitions (used to reset between algorithms)."""
    return initialize_partitions(partition_sizes)


def print_header(title):
    print("\n" + "=" * 60)
    print(f"  {title}")
    print("=" * 60)


def print_result_summary(satisfied, rejected, algorithm_name):
    total = len(satisfied) + len(rejected)
    print(f"\n  Summary for {algorithm_name}:")
    print(f"    Satisfied : {len(satisfied)}/{total} requests")
    print(f"    Rejected  : {len(rejected)}/{total} requests")
    if rejected:
        print(f"    Rejected processes: {', '.join(rejected)}")


# =============================================================================
# ROLANDO PEREZ — Introduction & Initialization Display
# =============================================================================
#
# ROLANDO: Your section handles:
#   1. Printing the intro/overview of memory management concepts
#   2. Showing the initial partition layout BEFORE any allocation
#   3. Printing the input (partition sizes and process sizes) so the audience
#      understands what data is being fed into the simulation
#
# HOW TO USE IN YOUR DEMO:
#   - Call rolando_intro(partition_sizes, processes) at the start of main()
#   - Walk the audience through the printed partition table
#   - Explain what "remaining" means (all free at start) and what process=None means
#
# =============================================================================

def rolando_intro(partition_sizes, processes):
    print_header("ROLANDO PEREZ — Introduction to Main Memory & Partitioning")

    print("""
  OVERVIEW:
  Main memory (RAM) must be shared among all active processes.
  The OS divides memory into partitions and allocates them to processes.

  PARTITION TYPES:
    Fixed  — partition sizes are set at system startup and never change
    Dynamic — partitions are created on the fly as processes arrive
             (this simulation uses fixed partitions)

  ALGORITHMS WE WILL COMPARE:
    First-Fit  — assign the FIRST partition that is large enough
    Best-Fit   — assign the SMALLEST partition that is large enough
    Worst-Fit  — assign the LARGEST partition available
    """)

    # Show the initial memory layout before any allocation
    partitions = get_fresh_partitions(partition_sizes)
    print("  Initial Memory Partitions (nothing allocated yet):")
    display_memory_state(partitions, "Initial State")

    print("\n  Processes to Allocate:")
    print(f"  {'Process':>10}  {'Size':>6}")
    print(f"  {'-'*10}  {'-'*6}")
    for name, size in processes:
        print(f"  {name:>10}  {size:>6}")

    print("""
  INPUT FORMAT:
    - Partition sizes represent the fixed memory blocks available
    - Process sizes represent how much memory each process needs
    - A process is REJECTED if no eligible partition is large enough
    """)


# =============================================================================
# LEONARDO CRISTOFARO — First-Fit Algorithm
# =============================================================================
#
# LEONARDO: Your section handles:
#   1. Running the First-Fit allocation algorithm
#   2. Printing which requests are satisfied vs rejected after each step
#   3. Displaying memory state after each allocation
#   4. Pointing out fragmentation forming near the front partitions
#
# HOW TO USE IN YOUR DEMO:
#   - Call leonardo_first_fit(partition_sizes, processes) in main()
#   - For each process, explain: "First-Fit scans from partition 1 forward
#     and picks the very first one big enough — no further searching."
#   - After a few allocations, point to the front partitions filling up
#     and note that later, large processes may not fit there anymore
#     even though total free memory exists (external fragmentation)
#
# TALKING POINTS:
#   - Speed advantage: stops scanning as soon as it finds a fit
#   - Fragmentation risk: small leftover holes accumulate at the front
#
# =============================================================================

def first_fit_allocate(partitions, process_name, process_size):
    """
    First-Fit: scan partitions from the beginning, pick the FIRST one
    that has enough remaining space.
    Returns True if allocated, False if rejected.
    """
    for p in partitions:
        if p["remaining"] >= process_size and p["process"] is None:
            p["process"] = process_name
            p["remaining"] -= process_size
            return True
    return False


def leonardo_first_fit(partition_sizes, processes):
    print_header("LEONARDO CRISTOFARO — First-Fit Memory Allocation")

    print("""
  FIRST-FIT ALGORITHM:
    Scans memory partitions from the beginning (partition 1 onward).
    Allocates the process to the FIRST partition with enough free space.
    Fast — stops searching as soon as a fit is found.
    Downside — small leftover fragments tend to pile up near the front.
    """)

    partitions = get_fresh_partitions(partition_sizes)
    satisfied = []
    rejected = []

    for name, size in processes:
        print(f"\n  >> Allocating {name} (size={size})")
        allocated = first_fit_allocate(partitions, name, size)

        if allocated:
            satisfied.append(name)
            print(f"     SATISFIED — {name} placed using First-Fit")
        else:
            rejected.append(name)
            print(f"     REJECTED  — no partition has enough space for {name} (size={size})")

        display_memory_state(partitions, "First-Fit")

    print_result_summary(satisfied, rejected, "First-Fit")
    calculate_fragmentation(partitions, "First-Fit")
    return partitions


# =============================================================================
# ALBERT TRAN — Best-Fit Algorithm
# =============================================================================
#
# ALBERT: Your section handles:
#   1. Running the Best-Fit allocation algorithm
#   2. Comparing results to First-Fit using the same inputs
#   3. Highlighting the smaller leftover fragments after each allocation
#
# HOW TO USE IN YOUR DEMO:
#   - Call albert_best_fit(partition_sizes, processes) in main()
#   - Use the SAME partition_sizes and processes as Leonardo's demo
#   - After each allocation, compare the "remaining" column to what
#     First-Fit left behind — Best-Fit leaves less wasted space per step
#   - Point out tiny leftover fragments (e.g., remaining=1 or 2)
#     and explain these become unusable — the "small hole" problem
#
# TALKING POINTS:
#   - Less wasted space per allocation than First-Fit
#   - Slower — must scan ALL partitions every time
#   - Creates many tiny leftover fragments that are too small to use
#
# =============================================================================

def best_fit_allocate(partitions, process_name, process_size):
    """
    Best-Fit: scan ALL partitions and pick the SMALLEST one that still fits.
    Minimizes wasted space per allocation but creates tiny unusable fragments.
    Returns True if allocated, False if rejected.
    """
    best_index = None
    best_remaining = float("inf")

    for i, p in enumerate(partitions):
        if p["remaining"] >= process_size and p["process"] is None:
            if p["remaining"] < best_remaining:
                best_remaining = p["remaining"]
                best_index = i

    if best_index is not None:
        partitions[best_index]["process"] = process_name
        partitions[best_index]["remaining"] -= process_size
        return True
    return False


def albert_best_fit(partition_sizes, processes):
    print_header("ALBERT TRAN — Best-Fit Memory Allocation")

    print("""
  BEST-FIT ALGORITHM:
    Scans ALL memory partitions before deciding.
    Allocates the process to the SMALLEST partition that still fits.
    Trade-off: wastes less space per allocation, but leaves tiny fragments
    that are often too small to be useful for future processes.
    """)

    partitions = get_fresh_partitions(partition_sizes)
    satisfied = []
    rejected = []

    for name, size in processes:
        print(f"\n  >> Allocating {name} (size={size})")
        allocated = best_fit_allocate(partitions, name, size)

        if allocated:
            satisfied.append(name)
            print(f"     SATISFIED — {name} placed using Best-Fit")
        else:
            rejected.append(name)
            print(f"     REJECTED  — no partition has enough space for {name} (size={size})")

        display_memory_state(partitions, "Best-Fit")

    print_result_summary(satisfied, rejected, "Best-Fit")
    calculate_fragmentation(partitions, "Best-Fit")
    return partitions


# =============================================================================
# JOSHUA DUENAS — Worst-Fit Algorithm & Comparison
# =============================================================================
#
# JOSHUA: Your section handles:
#   1. Running the Worst-Fit allocation algorithm
#   2. Explaining the rationale for choosing the LARGEST partition
#   3. Running the full comparison across all three algorithms
#
# HOW TO USE IN YOUR DEMO:
#   - Call joshua_worst_fit(partition_sizes, processes) in main()
#   - After Worst-Fit runs, call joshua_comparison(ff, bf, wf) passing
#     the partition states returned by each algorithm
#   - Point out that Worst-Fit leaves big leftovers — useful when
#     future processes are expected to be large
#   - In the comparison, highlight fragmentation differences visually
#
# TALKING POINTS — Worst-Fit rationale:
#   - Intentionally picks the LARGEST partition
#   - Leaves a large remainder that can still fit future big processes
#   - Better than Best-Fit when process sizes vary widely
#   - Worst performer when partition sizes are uniform
#
# COMPARISON TALKING POINTS:
#   - First-Fit: fastest, moderate fragmentation at the front
#   - Best-Fit: smallest holes, but most tiny unusable fragments
#   - Worst-Fit: large holes remain, good for varied workloads
#
# =============================================================================

def worst_fit_allocate(partitions, process_name, process_size):
    """
    Worst-Fit: scan ALL partitions and pick the LARGEST one available.
    Leaves the biggest possible remainder, which may fit future large processes.
    Returns True if allocated, False if rejected.
    """
    worst_index = None
    worst_remaining = -1

    for i, p in enumerate(partitions):
        if p["remaining"] >= process_size and p["process"] is None:
            if p["remaining"] > worst_remaining:
                worst_remaining = p["remaining"]
                worst_index = i

    if worst_index is not None:
        partitions[worst_index]["process"] = process_name
        partitions[worst_index]["remaining"] -= process_size
        return True
    return False


def joshua_worst_fit(partition_sizes, processes):
    print_header("JOSHUA DUENAS — Worst-Fit Memory Allocation")

    print("""
  WORST-FIT ALGORITHM:
    Scans ALL memory partitions before deciding.
    Allocates the process to the LARGEST available partition.
    Intentionally leaves the biggest possible remainder.
    Useful when future processes are expected to be large —
    larger leftovers are more likely to accommodate them.
    """)

    partitions = get_fresh_partitions(partition_sizes)
    satisfied = []
    rejected = []

    for name, size in processes:
        print(f"\n  >> Allocating {name} (size={size})")
        allocated = worst_fit_allocate(partitions, name, size)

        if allocated:
            satisfied.append(name)
            print(f"     SATISFIED — {name} placed using Worst-Fit")
        else:
            rejected.append(name)
            print(f"     REJECTED  — no partition has enough space for {name} (size={size})")

        display_memory_state(partitions, "Worst-Fit")

    print_result_summary(satisfied, rejected, "Worst-Fit")
    calculate_fragmentation(partitions, "Worst-Fit")
    return partitions


def calculate_fragmentation(partitions, label):
    """Calculates and prints internal fragmentation (wasted space in used partitions)."""
    total_free = sum(p["remaining"] for p in partitions if p["process"] is None)
    internal_frag = sum(p["remaining"] for p in partitions if p["process"] is not None)
    print(f"\n  Fragmentation Report [{label}]:")
    print(f"    Internal fragmentation (wasted space in used partitions) : {internal_frag} units")
    print(f"    Free space in unallocated partitions                     : {total_free} units")


def joshua_comparison(ff_partitions, bf_partitions, wf_partitions):
    """
    JOSHUA: Call this last. Prints a side-by-side comparison of all three algorithms.
    Shows internal fragmentation and free space to highlight efficiency differences.
    """
    print_header("JOSHUA DUENAS — Algorithm Comparison & Fragmentation Analysis")

    print("""
  SIDE-BY-SIDE COMPARISON
  Internal fragmentation = wasted space inside allocated (used) partitions
  Free space             = space in partitions that were never allocated
  """)

    def frag_stats(partitions):
        internal = sum(p["remaining"] for p in partitions if p["process"] is not None)
        free = sum(p["remaining"] for p in partitions if p["process"] is None)
        allocated = sum(1 for p in partitions if p["process"] is not None)
        return internal, free, allocated

    ff_int, ff_free, ff_alloc = frag_stats(ff_partitions)
    bf_int, bf_free, bf_alloc = frag_stats(bf_partitions)
    wf_int, wf_free, wf_alloc = frag_stats(wf_partitions)

    print(f"  {'Algorithm':>12}  {'Allocated':>9}  {'Int. Frag':>9}  {'Free Space':>10}")
    print(f"  {'-'*12}  {'-'*9}  {'-'*9}  {'-'*10}")
    print(f"  {'First-Fit':>12}  {ff_alloc:>9}  {ff_int:>9}  {ff_free:>10}")
    print(f"  {'Best-Fit':>12}  {bf_alloc:>9}  {bf_int:>9}  {bf_free:>10}")
    print(f"  {'Worst-Fit':>12}  {wf_alloc:>9}  {wf_int:>9}  {wf_free:>10}")

    print("""
  ANALYSIS:
    First-Fit  — Fast allocation, moderate fragmentation near the front.
                 Good general-purpose choice for most workloads.

    Best-Fit   — Minimizes wasted space per allocation.
                 Produces the most tiny unusable fragments over time.
                 Slower due to full scan every allocation.

    Worst-Fit  — Leaves the largest remainders after allocation.
                 Best when incoming processes tend to be large and varied.
                 Performs poorly on uniform workloads.

  CONCLUSION:
    No single algorithm is universally best.
    The right choice depends on process size distribution and system needs.
    """)


# =============================================================================
# MAIN — Entry Point
# Runs all four sections in order: Rolando → Leonardo → Albert → Joshua
# =============================================================================

if __name__ == "__main__":

    # -------------------------------------------------------------------------
    # INPUT: Modify these values to change the demo scenario
    # -------------------------------------------------------------------------
    partition_sizes = [100, 500, 200, 300, 600]   # sizes of fixed memory partitions (in units)

    processes = [
        ("P1",  212),
        ("P2",  417),
        ("P3",  112),
        ("P4",  426),
        ("P5",   98),
        ("P6",  500),
        ("P7",  300),
    ]
    # -------------------------------------------------------------------------

    # ROLANDO — Introduction & initial state display
    rolando_intro(partition_sizes, processes)

    # LEONARDO — First-Fit demo
    ff_result = leonardo_first_fit(partition_sizes, processes)

    # ALBERT — Best-Fit demo (same inputs)
    bf_result = albert_best_fit(partition_sizes, processes)

    # JOSHUA — Worst-Fit demo + final comparison
    wf_result = joshua_worst_fit(partition_sizes, processes)
    joshua_comparison(ff_result, bf_result, wf_result)
