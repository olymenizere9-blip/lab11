#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROOMMATES 20
#define MAX_NAME_LEN  64

/* ── helpers ─────────────────────────────────────────────────────────────── */

void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void print_separator(void) {
    printf("─────────────────────────────────────────────────────\n");
}

/* ── equal split ─────────────────────────────────────────────────────────── */

void equal_split(const char names[][MAX_NAME_LEN], int count, double total) {
    double share = total / count;

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

void weighted_split(const char names[][MAX_NAME_LEN], int count, double total) {
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
        weighted_split(names, count, total);   /* retry */
        return;
    }

    printf("\n");
    print_separator();
    printf("  WEIGHTED SPLIT RESULTS\n");
    print_separator();
    for (int i = 0; i < count; i++) {
        double amount = total * (percentages[i] / 100.0);
        printf("  %-20s  %5.1f%%   $%.2f\n", names[i], percentages[i], amount);
    }
    print_separator();
    printf("  Total:                        $%.2f\n", total);
    print_separator();
    printf("\n");
}

/* ── main ────────────────────────────────────────────────────────────────── */

int main(void) {
    double total;
    int    count;
    char   names[MAX_ROOMMATES][MAX_NAME_LEN];
    char   choice;

    printf("\n");
    print_separator();
    printf("           ROOMMATE BILL SPLITTER\n");
    print_separator();
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

    /* ── number of roommates ── */
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

    /* ── names ── */
    printf("\n");
    for (int i = 0; i < count; i++) {
        printf("  Name of roommate %d: ", i + 1);
        if (fgets(names[i], MAX_NAME_LEN, stdin) == NULL) {
            names[i][0] = '\0';
        }
        /* strip trailing newline */
        names[i][strcspn(names[i], "\n")] = '\0';
        if (strlen(names[i]) == 0) {
            snprintf(names[i], MAX_NAME_LEN, "Roommate %d", i + 1);
        }
    }

    /* ── equal split (always shown) ── */
    equal_split(names, count, total);

    /* ── weighted split option ── */
    while (1) {
        printf("  Would you like to do a weighted (custom %%) split? (y/n): ");
        if (scanf(" %c", &choice) != 1) {
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();
        if (choice == 'y' || choice == 'Y') {
            weighted_split(names, count, total);
            break;
        } else if (choice == 'n' || choice == 'N') {
            printf("  Goodbye!\n\n");
            break;
        } else {
            printf("  ✗  Please enter 'y' or 'n'.\n");
        }
    }

    return 0;
}
