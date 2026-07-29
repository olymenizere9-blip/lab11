# lab11 — Roommate Bill Splitter

A terminal-based C program that splits a shared bill between roommates.

## Features
- **Equal split** — divides any bill evenly across all roommates
- **Weighted split** — lets each person specify a custom percentage; validates that percentages sum to 100% and re-prompts if they don't

## Build & Run

```bash
gcc -Wall -Wextra -o roommate_split roommate_split.c
./roommate_split
```

## Usage

1. Enter the total bill amount (e.g. `450`)
2. Enter the number of roommates and their names
3. The equal split is shown automatically
4. Choose whether to also do a weighted split
5. If yes, enter a percentage for each person (must total 100%)

## Requirements
- GCC
- Linux / macOS terminal (any POSIX shell)
