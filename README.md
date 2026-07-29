# lab11 — Expense Splitter

A terminal-based C program that splits a shared bill between roommates, with persistent balance tracking across sessions.

## Features

- **Equal split** — divides any bill evenly across all roommates
- **Weighted split** — each person sets a custom percentage; validates that percentages sum to 100 % and re-prompts if they don't
- **Persistent ledger** — balances accumulate in `ledger.txt` across sessions; roommates are loaded automatically on the next run

## Build

```bash
make
```

This runs `gcc -Wall main.c -o expense_splitter` under the hood.

To remove the compiled binary:

```bash
make clean
```

## Run

```bash
./expense_splitter
```

## Usage

1. If `ledger.txt` exists, existing roommates and their accumulated balances are shown — choose to reuse them or start fresh
2. Enter the total bill amount
3. If starting fresh, enter the number of roommates and their names
4. The equal split is shown automatically
5. Choose whether to also perform a weighted split
6. If yes, enter a percentage for each person (must total 100 %)
7. Balances are updated and saved to `ledger.txt`

## Files

| File         | Purpose                              |
|--------------|--------------------------------------|
| `main.c`     | Full program source                  |
| `Makefile`   | Build rules                          |
| `ledger.txt` | Auto-generated; stores balances      |

## Requirements

- GCC
- Linux / macOS terminal (any POSIX shell)
- GNU Make
