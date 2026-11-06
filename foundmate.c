
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ITEMS 200
#define NAME_LEN 50
#define CAT_LEN 30
#define COLOR_LEN 25
#define BRAND_LEN 40
#define LOC_LEN 60
#define DATE_LEN 11
#define SECRET_LEN 80

#define LOST_FILE "lost_items.txt"
#define FOUND_FILE "found_items.txt"

typedef struct {
    int id;
    char name[NAME_LEN];
    char category[CAT_LEN];
    char color[COLOR_LEN];
    char brand[BRAND_LEN];
    char location[LOC_LEN];
    char date[DATE_LEN];       /* YYYY-MM-DD */
    char secret[SECRET_LEN];
    int status;                /* 0 = active, 1 = returned */
} Item;

typedef struct {
    int index;
    int score;
} Match;

Item lostItems[MAX_ITEMS];
Item foundItems[MAX_ITEMS];

int lostCount = 0;
int foundCount = 0;
int nextLostID = 1001;
int nextFoundID = 2001;

/* ---------- Input functions ---------- */

void readLine(const char *prompt, char *buffer, int size) {
    int ch;

    printf("%s", prompt);
    fflush(stdout);

    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }

    size_t len = strlen(buffer);

    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    } else {
        while ((ch = getchar()) != '\n' && ch != EOF) {
        }
    }
}

int readInt(const char *prompt, int min, int max) {
    char buffer[100];
    char *end;
    long value;

    while (1) {
        readLine(prompt, buffer, sizeof(buffer));

        value = strtol(buffer, &end, 10);

        while (isspace((unsigned char)*end))
            end++;

        if (buffer[0] != '\0' && *end == '\0' &&
            value >= min && value <= max)
            return (int)value;

        printf("Enter a number between %d and %d.\n", min, max);
    }
}

void pauseScreen(void) {
    char buffer[10];
    readLine("\nPress Enter to continue...", buffer, sizeof(buffer));
}

void printLine(void) {
    printf("--------------------------------------------------\n");
}

/* ---------- String functions ---------- */

void lowerString(char *s) {
    while (*s) {
        *s = (char)tolower((unsigned char)*s);
        s++;
    }
}

int sameText(const char *a, const char *b) {
    char x[LOC_LEN], y[LOC_LEN];

    snprintf(x, sizeof(x), "%s", a);
    snprintf(y, sizeof(y), "%s", b);

    lowerString(x);
    lowerString(y);

    return strcmp(x, y) == 0;
}

int containsText(const char *text, const char *query) {
    char x[LOC_LEN], y[LOC_LEN];

    snprintf(x, sizeof(x), "%s", text);
    snprintf(y, sizeof(y), "%s", query);

    lowerString(x);
    lowerString(y);

    return strstr(x, y) != NULL;
}

/* ---------- Date validation ---------- */

int isLeapYear(int year) {
    return (year % 400 == 0) ||
           (year % 4 == 0 && year % 100 != 0);
}

int validDate(const char *date) {
    int year, month, day;
    int daysInMonth[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    if (strlen(date) != 10 ||
        date[4] != '-' || date[7] != '-')
        return 0;

    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7)
            continue;

        if (!isdigit((unsigned char)date[i]))
            return 0;
    }

    year = (date[0] - '0') * 1000 +
           (date[1] - '0') * 100 +
           (date[2] - '0') * 10 +
           (date[3] - '0');

    month = (date[5] - '0') * 10 +
            (date[6] - '0');

    day = (date[8] - '0') * 10 +
          (date[9] - '0');

    if (year < 1900 || month < 1 || month > 12)
        return 0;

    if (isLeapYear(year))
        daysInMonth[1] = 29;

    return day >= 1 && day <= daysInMonth[month - 1];
}

void readDate(const char *prompt, char *date) {
    while (1) {
        readLine(prompt, date, DATE_LEN);

        if (validDate(date))
            return;

        printf("Invalid date. Use YYYY-MM-DD, e.g. 2026-11-04.\n");
    }
}

/* ---------- File handling ---------- */

/* File format uses | as a field separator. */

void saveItems(const char *filename, Item items[], int count) {
    FILE *fp = fopen(filename, "w");

    if (fp == NULL) {
        printf("Warning: Could not save %s.\n", filename);
        return;
    }

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d|%s|%s|%s|%s|%s|%s|%s|%d\n",
                items[i].id,
                items[i].name,
                items[i].category,
                items[i].color,
                items[i].brand,
                items[i].location,
                items[i].date,
                items[i].secret,
                items[i].status);
    }

    fclose(fp);
}

int loadItems(const char *filename, Item items[]) {
    FILE *fp = fopen(filename, "r");
    char line[600];
    int count = 0;

    if (fp == NULL)
        return 0;

    while (fgets(line, sizeof(line), fp) != NULL &&
           count < MAX_ITEMS) {
        Item item;

        int fields = sscanf(
            line,
            "%d|%49[^|]|%29[^|]|%24[^|]|%39[^|]|%59[^|]|%10[^|]|%79[^|]|%d",
            &item.id,
            item.name,
            item.category,
            item.color,
            item.brand,
            item.location,
            item.date,
            item.secret,
            &item.status
        );

        if (fields == 9 && validDate(item.date))
            items[count++] = item;
    }

    fclose(fp);
    return count;
}

void saveAll(void) {
    saveItems(LOST_FILE, lostItems, lostCount);
    saveItems(FOUND_FILE, foundItems, foundCount);
}

void loadAll(void) {
    lostCount = loadItems(LOST_FILE, lostItems);
    foundCount = loadItems(FOUND_FILE, foundItems);

    for (int i = 0; i < lostCount; i++)
        if (lostItems[i].id >= nextLostID)
            nextLostID = lostItems[i].id + 1;

    for (int i = 0; i < foundCount; i++)
        if (foundItems[i].id >= nextFoundID)
            nextFoundID = foundItems[i].id + 1;
}

/* ---------- Item registration ---------- */

void enterItemDetails(Item *item, int isLost) {
    readLine("What item was lost/found? ",
             item->name, NAME_LEN);

    readLine("Category: ",
             item->category, CAT_LEN);

    readLine("Color: ",
             item->color, COLOR_LEN);

    readLine("Brand (or Unknown): ",
             item->brand, BRAND_LEN);

    if (isLost) {
        readLine("Where was it lost? ",
                 item->location, LOC_LEN);

        readDate("Date lost (YYYY-MM-DD): ",
                 item->date);
    } else {
        readLine("Where was it found? ",
                 item->location, LOC_LEN);

        readDate("Date found (YYYY-MM-DD): ",
                 item->date);
    }
}

void reportLostItem(void) {
    if (lostCount >= MAX_ITEMS) {
        printf("Lost-item storage is full.\n");
        return;
    }

    Item item = {0};
    item.id = nextLostID++;
    item.status = 0;

    printLine();
    printf("REPORT LOST ITEM\n");
    printLine();

    enterItemDetails(&item, 1);

    printf("\nEnter a private detail for ownership verification.\n");
    readLine("Secret identifying detail: ",
             item.secret, SECRET_LEN);

    lostItems[lostCount++] = item;
    saveAll();

    printf("\nLost item registered. Your ID is %d.\n", item.id);
}

void reportFoundItem(void) {
    if (foundCount >= MAX_ITEMS) {
        printf("Found-item storage is full.\n");
        return;
    }

    Item item = {0};
    item.id = nextFoundID++;
    item.status = 0;

    printLine();
    printf("REPORT FOUND ITEM\n");
    printLine();

    enterItemDetails(&item, 0);

    foundItems[foundCount++] = item;
    saveAll();

    printf("\nFound item registered. Its ID is %d.\n", item.id);
}

/* ---------- View and search ---------- */

void displayItem(Item item, int isLost) {
    printf("\nID: %d | %s\n", item.id, item.name);
    printf("Category: %s | Color: %s | Brand: %s\n",
           item.category, item.color, item.brand);
    printf("Location: %s\n", item.location);
    printf("Date: %s\n", item.date);

    if (item.status)
        printf("Status: Returned / Claimed\n");
    else
        printf("Status: %s\n", isLost ? "Still Lost" : "Unclaimed");
}

void viewItems(Item items[], int count, int isLost) {
    if (count == 0) {
        printf("No reports available.\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        printLine();
        displayItem(items[i], isLost);
    }

    printLine();
}

void searchReports(void) {
    char query[LOC_LEN];
    int results = 0;

    readLine("Search by name, category, or location: ",
             query, sizeof(query));

    if (query[0] == '\0') {
        printf("Search cannot be empty.\n");
        return;
    }

    printf("\nSEARCH RESULTS\n");
    printLine();

    for (int i = 0; i < lostCount; i++) {
        if (containsText(lostItems[i].name, query) ||
            containsText(lostItems[i].category, query) ||
            containsText(lostItems[i].location, query)) {
            displayItem(lostItems[i], 1);
            results++;
        }
    }

    for (int i = 0; i < foundCount; i++) {
        if (containsText(foundItems[i].name, query) ||
            containsText(foundItems[i].category, query) ||
            containsText(foundItems[i].location, query)) {
            displayItem(foundItems[i], 0);
            results++;
        }
    }

    if (results == 0)
        printf("No matching reports found.\n");
}

/* ---------- Matching algorithm ---------- */

int dateScore(Item lost, Item found) {
    return strcmp(lost.date, found.date) == 0 ? 10 : 0;
}

int calculateMatch(Item lost, Item found) {
    int score = 0;

    if (sameText(lost.category, found.category))
        score += 20;

    if (sameText(lost.name, found.name))
        score += 25;
    else if (containsText(lost.name, found.name) ||
             containsText(found.name, lost.name))
        score += 15;

    if (sameText(lost.color, found.color))
        score += 15;

    if (sameText(lost.brand, found.brand) &&
        !sameText(lost.brand, "unknown"))
        score += 15;

    if (sameText(lost.location, found.location))
        score += 15;

    score += dateScore(lost, found);

    return score;
}

void sortMatches(Match matches[], int count) {
    /* Selection sort: highest score first. */
    for (int i = 0; i < count - 1; i++) {
        int best = i;

        for (int j = i + 1; j < count; j++)
            if (matches[j].score > matches[best].score)
                best = j;

        if (best != i) {
            Match temp = matches[i];
            matches[i] = matches[best];
            matches[best] = temp;
        }
    }
}

void explainMatch(Item lost, Item found) {
    int namePoints = 0;

    if (sameText(lost.name, found.name))
        namePoints = 25;
    else if (containsText(lost.name, found.name) ||
             containsText(found.name, lost.name))
        namePoints = 15;

    printf("Category: %s (+%d)\n",
           sameText(lost.category, found.category) ? "Match" : "No match",
           sameText(lost.category, found.category) ? 20 : 0);

    printf("Name: %s (+%d)\n",
           namePoints == 25 ? "Exact match" :
           namePoints == 15 ? "Partial match" : "No match",
           namePoints);

    printf("Color: %s (+%d)\n",
           sameText(lost.color, found.color) ? "Match" : "No match",
           sameText(lost.color, found.color) ? 15 : 0);

    printf("Brand: %s (+%d)\n",
           sameText(lost.brand, found.brand) ? "Match" : "No match",
           sameText(lost.brand, found.brand) &&
           !sameText(lost.brand, "unknown") ? 15 : 0);

    printf("Location: %s (+%d)\n",
           sameText(lost.location, found.location) ? "Match" : "No match",
           sameText(lost.location, found.location) ? 15 : 0);

    printf("Date: %s (+%d)\n",
           strcmp(lost.date, found.date) == 0 ? "Match" : "No match",
           dateScore(lost, found));
}

void findMatches(void) {
    int lostID = readInt("Enter Lost Item ID: ", 1, 1000000);
    int lostIndex = -1;

    for (int i = 0; i < lostCount; i++) {
        if (lostItems[i].id == lostID) {
            lostIndex = i;
            break;
        }
    }

    if (lostIndex == -1) {
        printf("Lost item ID not found.\n");
        return;
    }

    if (lostItems[lostIndex].status) {
        printf("This lost report is already closed.\n");
        return;
    }

    Match matches[MAX_ITEMS];
    int count = 0;

    for (int i = 0; i < foundCount; i++) {
        if (!foundItems[i].status) {
            int score = calculateMatch(lostItems[lostIndex],
                                       foundItems[i]);

            if (score >= 40) {
                matches[count].index = i;
                matches[count].score = score;
                count++;
            }
        }
    }

    if (count == 0) {
        printf("No possible matches found yet.\n");
        return;
    }

    sortMatches(matches, count);

    printf("\nMATCHES FOR: %s\n", lostItems[lostIndex].name);
    printLine();

    for (int i = 0; i < count; i++) {
        Item found = foundItems[matches[i].index];
        int score = matches[i].score;

        printf("\n%d. Found ID: %d | %s | Score: %d%%\n",
               i + 1, found.id, found.name, score);

        if (score >= 90)
            printf("Very strong match\n");
        else if (score >= 75)
            printf("Strong match\n");
        else if (score >= 50)
            printf("Possible match\n");
        else
            printf("Weak match\n");

        explainMatch(lostItems[lostIndex], found);
    }
}

/* ---------- Claim and return ---------- */

void claimItem(void) {
    int foundID = readInt("Enter Found Item ID: ", 1, 1000000);
    int foundIndex = -1;

    for (int i = 0; i < foundCount; i++)
        if (foundItems[i].id == foundID) {
            foundIndex = i;
            break;
        }

    if (foundIndex == -1) {
        printf("Found item ID not found.\n");
        return;
    }

    if (foundItems[foundIndex].status) {
        printf("This item has already been claimed.\n");
        return;
    }

    int lostID = readInt("Enter your Lost Item ID: ", 1, 1000000);
    int lostIndex = -1;

    for (int i = 0; i < lostCount; i++)
        if (lostItems[i].id == lostID) {
            lostIndex = i;
            break;
        }

    if (lostIndex == -1 || lostItems[lostIndex].status) {
        printf("Invalid or closed lost-item report.\n");
        return;
    }

    int score = calculateMatch(lostItems[lostIndex],
                               foundItems[foundIndex]);

    printf("Match score: %d%%\n", score);

    if (score < 50) {
        printf("Match score is too low to proceed.\n");
        return;
    }

    char answer[SECRET_LEN];

    readLine("Enter your secret identifying detail: ",
             answer, sizeof(answer));

    if (strcmp(answer, lostItems[lostIndex].secret) != 0) {
        printf("Verification failed. Item not claimed.\n");
        return;
    }

    foundItems[foundIndex].status = 1;
    lostItems[lostIndex].status = 1;
    saveAll();

    printf("\nOwnership verified! Item marked as returned.\n");
}

/* ---------- Statistics ---------- */

void showStatistics(void) {
    int activeLost = 0;
    int activeFound = 0;
    int returned = 0;

    for (int i = 0; i < lostCount; i++)
        if (!lostItems[i].status)
            activeLost++;

    for (int i = 0; i < foundCount; i++) {
        if (!foundItems[i].status)
            activeFound++;
        else
            returned++;
    }

    printLine();
    printf("FINDBACK STATISTICS\n");
    printLine();

    printf("Total lost reports: %d\n", lostCount);
    printf("Total found reports: %d\n", foundCount);
    printf("Still lost: %d\n", activeLost);
    printf("Unclaimed found items: %d\n", activeFound);
    printf("Items returned: %d\n", returned);

    if (foundCount > 0)
        printf("Recovery rate: %.2f%%\n",
               returned * 100.0 / foundCount);
    else
        printf("Recovery rate: 0.00%%\n");
}

/* ---------- Main menu ---------- */

int main(void) {
    loadAll();

    while (1) {
        printLine();
        printf("       FINDBACK - LOST & FOUND\n");
        printLine();

        printf("1. Report Lost Item\n");
        printf("2. Report Found Item\n");
        printf("3. View Lost Items\n");
        printf("4. View Found Items\n");
        printf("5. Find Possible Matches\n");
        printf("6. Search Reports\n");
        printf("7. Claim / Return Item\n");
        printf("8. View Statistics\n");
        printf("9. Exit\n");

        int choice = readInt("Choose an option: ", 1, 9);

        switch (choice) {
            case 1:
                reportLostItem();
                break;
            case 2:
                reportFoundItem();
                break;
            case 3:
                viewItems(lostItems, lostCount, 1);
                break;
            case 4:
                viewItems(foundItems, foundCount, 0);
                break;
            case 5:
                findMatches();
                break;
            case 6:
                searchReports();
                break;
            case 7:
                claimItem();
                break;
            case 8:
                showStatistics();
                break;
            case 9:
                saveAll();
                printf("Thank you for using FindBack!\n");
                return 0;
        }

        pauseScreen();
    }
}
