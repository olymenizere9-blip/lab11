/*
 * expense_splitter.c
 * Roommate bill-splitting tool — runs in a Linux terminal, compiled with GCC.
 *
 * Features:
 *   • Equal split across all roommates
 *   • Weighted split with custom percentages (validated to 100 %)
 *   • Persistent ledger in "ledger.txt" — balances accumulate across sessions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ── constants ───────────────────────────────────────────────────────────── */

#define MAX_ROOMMATES  20
#define MAX_NAME_LEN   64
#define LEDGER_FILE    "ledger.txt"

/* ── data types ──────────────────────────────────────────────────────────── */

typedef struct {
    char   name[MAX_NAME_LEN]; /* roommate's name                          */
    double balance;            /* accumulated total owed across all sessions*/
    double share;              /* share calculated in the current session   */
} Roommate;

/* ── display helpers ─────────────────────────────────────────────────────── */

static void print_separator(void) {
    printf("─────────────────────────────────────────────────────\n");
}

static void print_header(const char *title) {
    printf("\n");
    print_separator();
    printf("  %s\n", title);
    print_separator();
}

/* ── input helpers ───────────────────────────────────────────────────────── */

/* Discard the rest of the current input line (including the newline). */
static void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

/*
 * Read a single line of text into buf (max len bytes).
 * Strips the trailing newline.  Returns 0 on EOF, 1 on success.
 */
static int read_line(char *buf, int len) {
    if (!fgets(buf, len, stdin)) return 0;
    buf[strcspn(buf, "\n")] = '\0';
    return 1;
}

/*
 * Prompt the user for a y/n answer.
 * Keeps asking until 'y', 'Y', 'n', or 'N' is entered.
 * Returns 1 for yes, 0 for no.
 */
static int ask_yes_no(const char *prompt) {
    char c;
    while (1) {
        printf("  %s (y/n): ", prompt);
        if (scanf(" %c", &c) != 1) {
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();
        if (c == 'y' || c == 'Y') return 1;
        if (c == 'n' || c == 'N') return 0;
        printf("  ✗  Please enter 'y' or 'n'.\n");
    }
}

/*
 * Prompt for a positive dollar amount.
 * Keeps asking until a valid value > 0 is entered.
 */
static double ask_bill_amount(void) {
    double amount;
    while (1) {
        printf("  Enter the total bill amount ($): ");
        if (scanf("%lf", &amount) != 1) {
            printf("  ✗  Invalid input — please enter a number.\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();
        if (amount <= 0.0) {
            printf("  ✗  Amount must be greater than $0.00.\n");
            continue;
        }
        return amount;
    }
}

/*
 * Prompt for a roommate count in [1, MAX_ROOMMATES].
 * Keeps asking until a valid integer is entered.
 */
static int ask_roommate_count(void) {
    int n;
    while (1) {
        printf("  Number of roommates (1–%d): ", MAX_ROOMMATES);
        if (scanf("%d", &n) != 1) {
            printf("  ✗  Invalid input — please enter a whole number.\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();
        if (n < 1 || n > MAX_ROOMMATES) {
            printf("  ✗  Please enter a number between 1 and %d.\n",
                   MAX_ROOMMATES);
            continue;
        }
        return n;
    }
}

/*
 * Prompt for a percentage in [0, 100].
 * Keeps asking until a valid value is entered.
 */
static double ask_percentage(const char *name) {
    double pct;
    while (1) {
        printf("  Percentage for %-20s: ", name);
        if (scanf("%lf", &pct) != 1) {
            printf("  ✗  Invalid input — please enter a number.\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();
        if (pct < 0.0 || pct > 100.0) {
            printf("  ✗  Percentage must be between 0 and 100.\n");
            continue;
        }
        return pct;
    }
}

/* ── ledger I/O ──────────────────────────────────────────────────────────── */

/*
 * Load roommate names and accumulated balances from LEDGER_FILE.
 * File format: one line per roommate → "Name\tbalance\n"
 * Returns the number of records loaded (0 if file is missing or empty).
 */
static int load_ledger(Roommate roommates[], int max_count) {
    FILE *fp = fopen(LEDGER_FILE, "r");
    if (!fp) return 0;

    int   count = 0;
    char  line[MAX_NAME_LEN + 32];

    while (count < max_count && fgets(line, (int)sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        if (line[0] == '\0') continue;       /* skip blank lines */

        char *tab = strchr(line, '\t');
        if (!tab) continue;                  /* malformed — skip */

        *tab = '\0';
        strncpy(roommates[count].name, line, MAX_NAME_LEN - 1);
        roommates[count].name[MAX_NAME_LEN - 1] = '\0';
        roommates[count].balance = atof(tab + 1);
        roommates[count].share   = 0.0;
        count++;
    }

    fclose(fp);
    return count;
}

/*
 * Write current roommate names and accumulated balances to LEDGER_FILE,
 * overwriting any previous contents.
 */
static void save_ledger(const Roommate roommates[], int count) {
    FILE *fp = fopen(LEDGER_FILE, "w");
    if (!fp) {
        fprintf(stderr, "\n  ✗  Warning: could not write to %s\n\n",
                LEDGER_FILE);
        return;
    }
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s\t%.2f\n", roommates[i].name, roommates[i].balance);
    }
    fclose(fp);
}

/* Print the current accumulated ledger to stdout. */
static void show_ledger(const Roommate roommates[], int count) {
    print_header("LEDGER — accumulated balances");
    for (int i = 0; i < count; i++) {
        printf("  %-20s  $%.2f\n", roommates[i].name, roommates[i].balance);
    }
    print_separator();
    printf("\n");
}

/* ── roommate setup ──────────────────────────────────────────────────────── */

/*
 * Ask the user to enter names for 'count' roommates.
 * Initialises balance and share to 0.
 * Falls back to "Roommate N" if the user leaves a name blank.
 */
static void enter_roommates(Roommate roommates[], int count) {
    printf("\n");
    for (int i = 0; i < count; i++) {
        printf("  Name of roommate %d: ", i + 1);
        char buf[MAX_NAME_LEN];
        if (!read_line(buf, MAX_NAME_LEN) || buf[0] == '\0') {
            snprintf(buf, MAX_NAME_LEN, "Roommate %d", i + 1);
        }
        strncpy(roommates[i].name, buf, MAX_NAME_LEN - 1);
        roommates[i].name[MAX_NAME_LEN - 1] = '\0';
        roommates[i].balance = 0.0;
        roommates[i].share   = 0.0;
    }
}

/* ── split calculations ──────────────────────────────────────────────────── */

/*
 * Divide 'total' evenly among all roommates.
 * Stores each person's share in roommate.share and displays the result.
 */
static void compute_equal_split(Roommate roommates[], int count, double total) {
    double share = total / (double)count;

    print_header("EQUAL SPLIT");
    printf("  $%.2f ÷ %d person%s\n", total, count, count == 1 ? "" : "s");
    print_separator();
    for (int i = 0; i < count; i++) {
        roommates[i].share = share;
        printf("  %-20s  $%.2f\n", roommates[i].name, share);
    }
    print_separator();
    printf("  Each person owes: $%.2f\n", share);
    print_separator();
    printf("\n");
}

/*
 * Ask each roommate for a custom percentage, validate they sum to 100 %,
 * compute each person's share, and display the result.
 * Stores results in roommate.share.  Re-prompts the entire group if the
 * percentages do not add up to 100 %.
 */
static void compute_weighted_split(Roommate roommates[], int count,
                                   double total) {
    double percentages[MAX_ROOMMATES];
    double sum;

    /* Keep asking until percentages are valid. */
    while (1) {
        sum = 0.0;
        print_header("WEIGHTED SPLIT — enter each person's percentage");
        for (int i = 0; i < count; i++) {
            percentages[i] = ask_percentage(roommates[i].name);
            sum += percentages[i];
        }

        if (sum >= 99.99 && sum <= 100.01) break;   /* valid */

        printf("\n  ✗  Percentages add up to %.2f%% — they must total 100%%.\n",
               sum);
        printf("  Please re-enter all percentages.\n");
    }

    /* Compute and store shares. */
    for (int i = 0; i < count; i++) {
        roommates[i].share = total * (percentages[i] / 100.0);
    }

    printf("\n");
    print_separator();
    printf("  WEIGHTED SPLIT RESULTS\n");
    print_separator();
    for (int i = 0; i < count; i++) {
        printf("  %-20s  %5.1f%%   $%.2f\n",
               roommates[i].name, percentages[i], roommates[i].share);
    }
    print_separator();
    printf("  Total:                        $%.2f\n", total);
    print_separator();
    printf("\n");
}

/*
 * Add each roommate's current session share to their accumulated balance.
 */
static void apply_shares_to_balances(Roommate roommates[], int count) {
    for (int i = 0; i < count; i++) {
        roommates[i].balance += roommates[i].share;
    }
}

/* ── main ────────────────────────────────────────────────────────────────── */

int main(void) {
    Roommate roommates[MAX_ROOMMATES];
    int      count = 0;

    printf("\n");
    print_separator();
    printf("              EXPENSE SPLITTER\n");
    print_separator();

    /* ── load existing ledger ── */
    int loaded = load_ledger(roommates, MAX_ROOMMATES);

    if (loaded > 0) {
        printf("\n  Loaded %d roommate%s from %s:\n",
               loaded, loaded == 1 ? "" : "s", LEDGER_FILE);
        for (int i = 0; i < loaded; i++) {
            printf("    %-20s  accumulated: $%.2f\n",
                   roommates[i].name, roommates[i].balance);
        }

        if (ask_yes_no("\nUse these roommates")) {
            count = loaded;          /* reuse names and balances */
        }
        /* if "no": count stays 0, user will enter fresh roommates below */
    }

    printf("\n");

    /* ── bill amount ── */
    double total = ask_bill_amount();

    /* ── roommate names (only when not reusing ledger) ── */
    if (count == 0) {
        count = ask_roommate_count();
        enter_roommates(roommates, count);
    }

    /* ── equal split (always computed and displayed) ── */
    compute_equal_split(roommates, count, total);

    /* ── optional weighted split ── */
    if (ask_yes_no("Would you like to do a weighted (custom %%) split")) {
        compute_weighted_split(roommates, count, total);
    }

    /* ── persist: add session shares to running balances, then save ── */
    apply_shares_to_balances(roommates, count);
    save_ledger(roommates, count);

    /* ── display updated ledger ── */
    show_ledger(roommates, count);
    printf("  Balances saved to %s\n\n", LEDGER_FILE);

    return 0;
}
