*This project has been created as part of the 42 curriculum by valrakot.*

# Codexion

## Description

**Codexion** is a concurrency and synchronization project written in C.

The simulation represents a group of coders working around a shared quantum compiler. Each coder is represented by a POSIX thread and alternates between three activities:

1. Compiling
2. Debugging
3. Refactoring

Compiling requires a coder to hold two USB dongles simultaneously. There is one dongle between each pair of coders, creating a circular resource-sharing system.

The main challenge is to coordinate concurrent access to these shared dongles while preventing race conditions, deadlocks, starvation, and coder burnout.

The project uses POSIX threads (`pthread`), mutexes, condition variables, and a custom priority queue implemented as a min-heap.

Two scheduling policies are supported:

- **FIFO (First In, First Out):** a dongle is granted to the coder whose request arrived first.
- **EDF (Earliest Deadline First):** a dongle is granted to the coder with the earliest burnout deadline.

## Features

- One POSIX thread per coder
- Circular arrangement of coders and dongles
- Two dongles required simultaneously for compilation
- Mutex protection for shared dongles
- Condition variables for waiting coders
- Dongle cooldown
- FIFO scheduling
- EDF scheduling
- Custom min-heap priority queue
- Separate monitor thread for burnout detection
- Thread-safe logging
- Millisecond-precision timestamps
- Deadlock prevention
- Starvation prevention
- Proper resource cleanup
- No global variables

## Instructions

### Compilation

Build the project with:

```bash
make
```

The project must compile with:

```text
-Wall -Wextra -Werror -pthread
```

Available Makefile targets:

```text
all
clean
fclean
re
```

### Usage

```text
./codexion number_of_coders time_to_burnout time_to_compile \
time_to_debug time_to_refactor number_of_compiles_required \
dongle_cooldown scheduler
```

The scheduler must be either:

```text
fifo
```

or:

```text
edf
```

Example:
```bash
./codexion
    number_of_coders
    time_to_burnout
    time_to_compile
    time_to_debug
    time_to_refactor
    number_of_compiles_required
    dongle_cooldown
    scheduler
```
```bash
./codexion 4 800 200 200 200 3 50 fifo
```

EDF example:

```bash
./codexion 4 800 200 200 200 3 50 edf
```

### Arguments

| Argument | Description |
|---|---|
| `number_of_coders` | Number of coders and dongles |
| `time_to_burnout` | Maximum allowed time before a coder must start compiling again |
| `time_to_compile` | Duration of the compilation phase |
| `time_to_debug` | Duration of the debugging phase |
| `time_to_refactor` | Duration of the refactoring phase |
| `number_of_compiles_required` | Number of compilations required for every coder |
| `dongle_cooldown` | Time a dongle remains unavailable after release |
| `scheduler` | Scheduling policy: `fifo` or `edf` |

## Simulation Lifecycle

Each coder follows this cycle:

```text
Request two dongles
        |
        v
   Take both dongles
        |
        v
      Compile
        |
        v
   Release dongles
        |
        v
      Debug
        |
        v
    Refactor
        |
        +--------> repeat
```

A coder must hold both dongles during the entire compilation phase.

After compilation, both dongles are released and become subject to the configured cooldown.

The simulation ends when either:

1. Every coder has completed the required number of compilations.
2. A coder burns out.

## Scheduling

### FIFO

FIFO means **First In, First Out**.

The coder whose request arrived first receives priority.

Example:

```text
Coder 1 -> request at 1000 ms
Coder 2 -> request at 1050 ms
Coder 3 -> request at 1100 ms
```

Priority:

```text
Coder 1 -> Coder 2 -> Coder 3
```

### EDF

EDF means **Earliest Deadline First**.

The coder with the earliest burnout deadline receives priority.

A deadline is based on the coder's last compilation start time and the configured burnout time.

Example:

```text
Coder 1 -> deadline 5000
Coder 2 -> deadline 3500
Coder 3 -> deadline 7000
```

Priority:

```text
Coder 2 -> Coder 1 -> Coder 3
```

When EDF deadlines are equal, a deterministic tie-breaker is used.

## Priority Queue

Each dongle maintains a custom priority queue implemented as a **min-heap**.

A heap node contains:

```c
typedef struct s_heap_node
{
    t_coder     *coder;
    long long   key;
}   t_heap_node;
```

The key represents the scheduling priority:

```text
FIFO -> request arrival time
EDF  -> burnout deadline
```

The heap provides operations such as:

```text
heap_push()
heap_pop()
```

The smallest key has the highest priority.

## Thread Synchronization Mechanisms

### POSIX Threads

Each coder is represented by its own POSIX thread.

```text
Simulation
    |
    +-- Coder 1 -> Thread 1
    +-- Coder 2 -> Thread 2
    +-- Coder 3 -> Thread 3
    +-- Coder 4 -> Thread 4
```

### Mutexes

Each dongle has a mutex protecting its shared state.

```c
pthread_mutex_t mutex;
```

The simulation also uses mutexes to protect shared simulation state and serialize log output.

The basic critical-section pattern is:

```c
pthread_mutex_lock(&mutex);

/* critical section */

pthread_mutex_unlock(&mutex);
```

### Condition Variables

Each dongle has a condition variable:

```c
pthread_cond_t cond;
```

A coder that cannot currently obtain a dongle can wait until the resource state changes instead of continuously polling.

Relevant operations include:

```text
pthread_cond_wait()
pthread_cond_signal()
pthread_cond_broadcast()
```

### Thread Joining

Threads are joined before the simulation resources are destroyed.

```c
pthread_join(thread, NULL);
```

This ensures that threads have finished before cleanup.

## Blocking Cases Handled

### Deadlock

A deadlock can occur when coders hold one dongle while waiting for another.

Example:

```text
Coder 1 owns D1 -> waits for D2
Coder 2 owns D2 -> waits for D3
Coder 3 owns D3 -> waits for D4
Coder 4 owns D4 -> waits for D1
```

This creates a circular wait in which no coder can continue.

The implementation must coordinate dongle acquisition so that coders do not remain indefinitely blocked by circular resource dependencies.

### Starvation

Starvation occurs when a coder repeatedly loses access to a required dongle while other coders continue to be served.

The scheduling mechanism uses FIFO or EDF priority to determine which waiting coder should receive a dongle.

### Dongle Cooldown

After a dongle is released, it remains unavailable until its cooldown has elapsed.

The next available time is stored in:

```c
long long available_at;
```

Conceptually:

```text
available_at = release_time + dongle_cooldown
```

### Burnout Detection

A monitor thread checks whether a coder has exceeded its allowed time before starting another compilation.

A coder is considered burned out when its deadline is reached before it can begin compiling again.

### Log Serialization

Multiple threads may attempt to print at the same time.

A dedicated print mutex ensures that complete log messages are printed without being mixed together.

Example:

```c
pthread_mutex_lock(&sim->print_lock);

printf("%lld %d %s\n", timestamp, coder->id, state);

pthread_mutex_unlock(&sim->print_lock);
```

## Data Structures

### `t_sim`

Stores the global state of the simulation:

- number of coders
- timing parameters
- required compilation count
- dongle cooldown
- scheduler
- simulation start time
- simulation termination state
- synchronization mutexes
- coder array
- dongle array

### `t_coder`

Stores:

- coder identifier
- completed compilations
- last compilation start time
- burnout state
- pthread identifier
- left dongle
- right dongle
- pointer to the simulation

### `t_dongle`

Stores:

- dongle identifier
- mutex
- condition variable
- next available time
- scheduling queue

### `t_heap`

Stores the custom priority queue:

```text
data
capacity
size
```

Each heap element contains a coder and its scheduling key.

## Time Measurement

The simulation uses millisecond timestamps.

For example:

```c
long long get_time_in_ms(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return ((tv.tv_sec * 1000LL) + (tv.tv_usec / 1000LL));
}
```

The result represents the current time in milliseconds.

`1000LL` converts seconds to milliseconds while forcing the arithmetic to use a `long long` integer type.

## Memory Management

All dynamically allocated resources must be released correctly.

The simulation allocates memory for:

- coders
- dongles
- heap data

Synchronization objects must also be destroyed during cleanup:

```text
pthread_mutex_destroy()
pthread_cond_destroy()
```

The project must not leave memory leaks.

## Testing

The implementation should be tested with:

- one coder
- multiple coders
- FIFO scheduling
- EDF scheduling
- invalid arguments
- invalid scheduler
- small timing values
- dongle cooldown
- simultaneous dongle requests
- deadlock scenarios
- starvation scenarios
- burnout detection
- successful completion
- memory-leak checks

Example:

```bash
./codexion 1 800 200 200 200 3 50 fifo
```

Multiple coders:

```bash
./codexion 4 800 200 200 200 3 50 fifo
```

EDF:

```bash
./codexion 4 800 200 200 200 3 50 edf
```

## Resources

The project uses the following types of resources:

- POSIX threads documentation for `pthread_create`, `pthread_join`, mutexes, and condition variables.
- Documentation about `gettimeofday()` and millisecond time measurement.
- Documentation and educational material about min-heaps and priority queues.
- Educational material about deadlocks, starvation, mutexes, and condition variables.

## AI Usage

AI was used as a learning and assistance tool during the project.

It was used to:

- understand POSIX threads;
- understand mutexes and condition variables;
- understand deadlocks and synchronization;
- understand FIFO and EDF scheduling;
- understand min-heaps;
- review code structure;
- reason about concurrency problems;
- clarify project requirements.

AI suggestions were reviewed and adapted to the project requirements.

## Project Constraints

The project follows the subject requirements:

- Written in C.
- 42 Norm compliant.
- No global variables.
- No libft.
- Custom priority queue implemented as a min-heap.
- One thread per coder.
- One dongle between each pair of coders.
- Two dongles required for compilation.
- Dongles protected by synchronization primitives.
- Mandatory dongle cooldown.
- FIFO and EDF scheduling.
- Separate burnout monitor thread.
- Thread-safe logging.
- Proper memory cleanup.
- Compilation with `-Wall -Wextra -Werror -pthread`.

## Author

**42 login:** `valrakot`

**Project:** Codexion

**Language:** C

**Topic:** POSIX threads, synchronization, resource scheduling, deadlock prevention, starvation prevention, and concurrent programming.
