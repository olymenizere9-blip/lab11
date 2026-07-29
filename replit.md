# lab11 — Expense Splitter

## Overview
A single-file C program (`main.c`) that runs in a Linux terminal.
Splits a shared bill equally or by custom percentages, and tracks
accumulated balances per roommate in `ledger.txt`.

## How to build and run

```bash
make                   # compiles to ./expense_splitter
./expense_splitter     # run the program
make clean             # remove the binary
```

## Stack
- Language: C (C11)
- Compiler: GCC (`gcc -Wall main.c -o expense_splitter`)
- Build tool: GNU Make
- No external dependencies

## Project structure
- `main.c`     — full program source (structs, functions, input validation)
- `Makefile`   — build rules
- `ledger.txt` — auto-generated at runtime; stores roommate balances

## User preferences
- Keep the program as a single C source file (`main.c`) unless the user asks to split it
