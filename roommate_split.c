#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROOMMATES 20
#define MAX_NAME_LEN  64
#define LEDGER_FILE   "ledger.txt"

/* ── helpers ─────────────────────────────────────────────────────────────── */

void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void print_separator(void) {
    printf("─────────────────────────────────────────────────────\n");
}

/* ── ledger I/O ──────────────────────────────────────────────────────────── */

/*
 * File format — one line per roommate:
 *   Name<TAB>balance\n
 * Returns the number of roommates loaded (0 if file absent or empty).
 */
int load_ledger(char names[][MAX_NAME_LEN], double balances[], int max) {
    FILE *fp = fopen(LEDGER_FILE, "r");
    if (!fp) return 0;

    int count = 0;
    char line[MAX_NAME_LEN + 32];

    while (count < max && fgets(line, sizeof(line), fp)) {
        /* strip trailing newline */
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;

        char *tab = strchr(line, '\t');
        if (!tab) continue;          /* malformed line — skip */

        *tab = '\0';
        strncpy(names[count], line, MAX_NAME_LEN - 1);
        names[count][MAX_NAME_LEN - 1] = '\0';
        balances[count] = atof(tab + 1);
        count++;
    }

    fclose(fp);
    return count;
}

void save_ledger(const char names[][MAX_NAME_LEN],
                 const double balances[], int count) {
    FILE *fp = fopen(LEDGER_FILE, "w");
    if (!fp) {
        fprintf(stderr, "  ✗  Warning: could not write to %s\n", LEDGER_FILE);
        return;
    }
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s\t%.2f\n", names[i], balances[i]);
    }
    fclose(fp);
}

void show_ledger(const char names[][MAX_NAME_LEN],
                 const double balances[], int count) {
    printf("\n");
    print_separator();
    printf("  LEDGER — accumulated balances\n");
    print_separator();
    for (int i = 0; i < count; i++) {
        printf("  %-20s  $%.2f\n", names[i], balances[i]);
    }
    print_separator();
    printf("\n");
}

/* ── equal split ─────────────────────────────────────────────────────────── */

/* Fills amounts[] with each person's share; does NOT update balances. */
void equal_split(const char names[][MAX_NAME_LEN],
                 int count, double total, double amounts[]) {
    double share = total / count;
    for (int i = 0; i < count; i++) amounts[i] = share;

    printf("\n");
    print_separator();
    printf("  EQUAL SPLIT — $%.2f ÷ %d person%s\n", total, count,
           count == 1 ? "" : "s");
    print_separator();
    for (int i = 0; i < count; i++) {
        printf("  %-20s  $%.2f\n", names[i], share);
    }
    print_separator();
    printf("  Each person owes: $%.2f\n", share);
    print_separator();
    printf("\n");
}

/* ── weighted split ──────────────────────────────────────────────────────── */

/* Fills amounts[] with each person's weighted share; does NOT update balances. */
void weighted_split(const char names[][MAX_NAME_LEN],
                    int count, double total, double amounts[]) {
    double percentages[MAX_ROOMMATES];
    double sum = 0.0;

    printf("\n");
    print_separator();
    printf("  WEIGHTED SPLIT — enter each person's percentage\n");
    print_separator();

    for (int i = 0; i < count; i++) {
        while (1) {
            printf("  Percentage for %-20s: ", names[i]);
            if (scanf("%lf", &percentages[i]) != 1) {
                printf("  ✗  Invalid input. Please enter a number.\n");
                clear_input_buffer();
                continue;
            }
            clear_input_buffer();
            if (percentages[i] < 0.0 || percentages[i] > 100.0) {
                printf("  ✗  Percentage must be between 0 and 100.\n");
                continue;
            }
            break;
        }
        sum += percentages[i];
    }

    /* validate total */
    if (sum < 99.99 || sum > 100.01) {
        printf("\n  ✗  Percentages add up to %.2f%% — they must total 100%%.\n",
               sum);
        printf("  Please re-enter the percentages.\n\n");
        weighted_split(names, count, total, amounts);   /* retry */
        return;
    }

    for (int i = 0; i < count; i++) {
        amounts[i] = total * (percentages[i] / 100.0);
    }

    printf("\n");
    print_separator();
    printf("  WEIGHTED SPLIT RESULTS\n");
    print_separator();
    for (int i = 0; i < count; i++) {
        printf("  %-20s  %5.1f%%   $%.2f\n", names[i], percentages[i],
               amounts[i]);
    }
    print_separator();
    printf("  Total:                        $%.2f\n", total);
    print_separator();
    printf("\n");
}

/* ── main ────────────────────────────────────────────────────────────────── */

int main(void) {
    double total;
    int    count = 0;
    char   names[MAX_ROOMMATES][MAX_NAME_LEN];
    double balances[MAX_ROOMMATES];
    double amounts[MAX_ROOMMATES];   /* per-session shares */
    char   choice;

    printf("\n");
    print_separator();
    printf("           ROOMMATE BILL SPLITTER\n");
    print_separator();

    /* ── load ledger ── */
    int loaded = load_ledger(names, balances, MAX_ROOMMATES);

    if (loaded > 0) {
        printf("\n  Loaded %d roommate%s from %s:\n",
               loaded, loaded == 1 ? "" : "s", LEDGER_FILE);
        for (int i = 0; i < loaded; i++) {
            printf("    %-20s  accumulated: $%.2f\n", names[i], balances[i]);
        }

        /* ask whether to reuse the loaded roommates */
        while (1) {
            printf("\n  Use these roommates? (y/n): ");
            if (scanf(" %c", &choice) != 1) {
                clear_input_buffer();
                continue;
            }
            clear_input_buffer();
            if (choice == 'y' || choice == 'Y') {
                count = loaded;
                break;
            } else if (choice == 'n' || choice == 'N') {
                /* fall through to manual entry; reset balances */
                count = 0;
                break;
            } else {
                printf("  ✗  Please enter 'y' or 'n'.\n");
            }
        }
    }

    printf("\n");

    /* ── bill amount ── */
    while (1) {
        printf("  Enter the total bill amount ($): ");
        if (scanf("%lf", &total) != 1) {
            printf("  ✗  Invalid amount. Please enter a number.\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();
        if (total <= 0.0) {
            printf("  ✗  Amount must be greater than 0.\n");
            continue;
        }
        break;
    }

    /* ── number of roommates (only when not reusing ledger) ── */
    if (count == 0) {
        while (1) {
            printf("  Number of roommates (1–%d): ", MAX_ROOMMATES);
            if (scanf("%d", &count) != 1) {
                printf("  ✗  Invalid number. Please enter a whole number.\n");
                clear_input_buffer();
                continue;
            }
            clear_input_buffer();
            if (count < 1 || count > MAX_ROOMMATES) {
                printf("  ✗  Please enter a number between 1 and %d.\n",
                       MAX_ROOMMATES);
                continue;
            }
            break;
        }

        /* ── names & zero balances ── */
        printf("\n");
        for (int i = 0; i < count; i++) {
            printf("  Name of roommate %d: ", i + 1);
            if (fgets(names[i], MAX_NAME_LEN, stdin) == NULL) {
                names[i][0] = '\0';
            }
            names[i][strcspn(names[i], "\n")] = '\0';
            if (strlen(names[i]) == 0) {
                snprintf(names[i], MAX_NAME_LEN, "Roommate %d", i + 1);
            }
            balances[i] = 0.0;
        }
    }

    /* ── equal split (always shown) ── */
    equal_split(names, count, total, amounts);

    /* ── weighted split option ── */
    while (1) {
        printf("  Would you like to do a weighted (custom %%) split? (y/n): ");
        if (scanf(" %c", &choice) != 1) {
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();
        if (choice == 'y' || choice == 'Y') {
            weighted_split(names, count, total, amounts);
            break;
        } else if (choice == 'n' || choice == 'N') {
            break;
        } else {
            printf("  ✗  Please enter 'y' or 'n'.\n");
        }
    }

    /* ── update balances with the final amounts and save ── */
    for (int i = 0; i < count; i++) {
        balances[i] += amounts[i];
    }
    save_ledger(names, balances, count);

    /* ── show updated ledger ── */
    show_ledger(names, balances, count);
    printf("  Balances saved to %s\n\n", LEDGER_FILE);

    return 0;
}
