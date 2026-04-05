#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 10
#define PRIME 101

typedef struct Item {
    char id[20];
    char name[50];
    char category[30];
    char location[50];
    char date[15];
    char description[100];
    char secret[50];
    struct Item* next;
} Item;

Item* hashTable[TABLE_SIZE];

// Helper for Edit Distance
int min(int a, int b, int c) {
    if (a <= b && a <= c) return a;
    if (b <= a && b <= c) return b;
    return c;
}

// Logic Algorithms (Unchanged)
int calculateHash(char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) hash = ((hash << 5) + hash) + c;
    return hash % TABLE_SIZE;
}

void generateID(char* category, char* id) {
    static int counter = 1001;
    sprintf(id, "%.2s-%d", category, counter++);
}

int rabinKarp(char* pattern, char* text) {
    int M = strlen(pattern), N = strlen(text);
    if (M > N) return 0;
    int i, j, p = 0, t = 0, h = 1;
    for (i = 0; i < M - 1; i++) h = (h * 256) % PRIME;
    for (i = 0; i < M; i++) {
        p = (256 * p + pattern[i]) % PRIME;
        t = (256 * t + text[i]) % PRIME;
    }
    for (i = 0; i <= N - M; i++) {
        if (p == t) {
            for (j = 0; j < M; j++) if (text[i + j] != pattern[j]) break;
            if (j == M) return 1;
        }
        if (i < N - M) {
            t = (256 * (t - text[i] * h) + text[i + M]) % PRIME;
            if (t < 0) t = (t + PRIME);
        }
    }
    return 0;
}

int levenshtein(char* s1, char* s2) {
    int s1len = strlen(s1), s2len = strlen(s2);
    int matrix[s1len + 1][s2len + 1];
    for (int i = 0; i <= s1len; i++) matrix[i][0] = i;
    for (int j = 0; j <= s2len; j++) matrix[0][j] = j;
    for (int i = 1; i <= s1len; i++) {
        for (int j = 1; j <= s2len; j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            matrix[i][j] = min(matrix[i - 1][j] + 1, matrix[i][j - 1] + 1, matrix[i - 1][j - 1] + cost);
        }
    }
    return matrix[s1len][s2len];
}

// File Handling
void saveToFile() {
    FILE *file = fopen("database.txt", "w");
    if (!file) return;
    for (int i = 0; i < TABLE_SIZE; i++) {
        Item* temp = hashTable[i];
        while (temp) {
            fprintf(file, "%s|%s|%s|%s|%s|%s|%s\n", temp->id, temp->name, temp->category, temp->location, temp->date, temp->description, temp->secret);
            temp = temp->next;
        }
    }
    fclose(file);
}

void loadFromFile() {
    FILE *file = fopen("database.txt", "r");
    if (!file) return;
    while (!feof(file)) {
        Item* newItem = (Item*)malloc(sizeof(Item));
        if (fscanf(file, " %[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]\n", newItem->id, newItem->name, newItem->category, newItem->location, newItem->date, newItem->description, newItem->secret) == 7) {
            int index = calculateHash(newItem->name);
            newItem->next = hashTable[index];
            hashTable[index] = newItem;
        } else free(newItem);
    }
    fclose(file);
}

// UI Functions
void draw_box(int y, int x, int h, int w, char* title) {
    mvprintw(y, x, "+");
    for(int i=1; i<w-1; i++) printw("-");
    printw("+");
    for(int i=1; i<h-1; i++) {
        mvprintw(y+i, x, "|");
        mvprintw(y+i, x+w-1, "|");
    }
    mvprintw(y+h-1, x, "+");
    for(int i=1; i<w-1; i++) printw("-");
    printw("+");
    if(title) mvprintw(y, x + (w - strlen(title))/2, " %s ", title);
}

void addItemUI() {
    clear();
    draw_box(1, 2, 18, 60, "LOG NEW FOUND ITEM");
    Item* newItem = (Item*)malloc(sizeof(Item));
    
    mvprintw(3, 4, "Available: Stationary, Bags, Devices, Accessories");
    echo();
    mvprintw(5, 4, "Category: "); getstr(newItem->category);
    mvprintw(6, 4, "Item Name: "); getstr(newItem->name);
    mvprintw(7, 4, "Description: "); getstr(newItem->description);
    mvprintw(8, 4, "Location: "); getstr(newItem->location);
    mvprintw(9, 4, "Date: "); getstr(newItem->date);
    mvprintw(10, 4, "Secret Detail: "); getstr(newItem->secret);
    noecho();

    generateID(newItem->category, newItem->id);
    int index = calculateHash(newItem->name);
    newItem->next = hashTable[index];
    hashTable[index] = newItem;
    saveToFile();

    attron(COLOR_PAIR(2));
    mvprintw(12, 4, "SUCCESS! Unique ID: %s", newItem->id);
    attroff(COLOR_PAIR(2));
    mvprintw(14, 4, "Press any key to return...");
    refresh();
    getch();
}

void viewItemsUI() {
    clear();
    draw_box(1, 1, 20, 78, "CURRENT LOST & FOUND DATABASE");
    mvprintw(3, 3, "%-10s | %-15s | %-15s | %-15s", "ID", "Name", "Category", "Location");
    mvprintw(4, 3, "------------------------------------------------------------------");
    
    int row = 5;
    for (int i = 0; i < TABLE_SIZE; i++) {
        Item* temp = hashTable[i];
        while (temp) {
            mvprintw(row++, 3, "%-10s | %-15s | %-15s | %-15s", temp->id, temp->name, temp->category, temp->location);
            temp = temp->next;
        }
    }
    mvprintw(18, 3, "Press any key to return...");
    refresh();
    getch();
}

void searchItemUI() {
    clear();
    char cat[30], query[50];
    draw_box(1, 2, 10, 60, "SEARCH / RECLAIM");
    mvprintw(3, 4, "Category: "); echo(); getstr(cat);
    mvprintw(4, 4, "Query: "); getstr(query); noecho();

    Item* matches[10];
    int count = 0;
    for (int i = 0; i < TABLE_SIZE; i++) {
        Item* curr = hashTable[i];
        while (curr) {
            if (strcmp(curr->category, cat) == 0 && (strcmp(curr->name, query) == 0 || rabinKarp(query, curr->name) || levenshtein(query, curr->name) <= 2)) {
                if (count < 10) matches[count++] = curr;
            }
            curr = curr->next;
        }
    }

    if (count == 0) {
        mvprintw(6, 4, "No matches found. Press any key...");
        getch(); return;
    }

    clear();
    draw_box(1, 2, 15, 60, "RESULTS FOUND");
    for(int i=0; i<count; i++) mvprintw(3+i, 4, "[%d] ID: %s | Name: %s", i+1, matches[i]->id, matches[i]->name);
    
    mvprintw(12, 4, "Select index to reclaim (0 to cancel): ");
    echo(); char choiceStr[5]; getstr(choiceStr); noecho();
    int choice = atoi(choiceStr);

    if (choice > 0 && choice <= count) {
        char checkID[20], checkSec[50];
        mvprintw(13, 4, "Unique ID: "); echo(); getstr(checkID);
        mvprintw(14, 4, "Secret Detail: "); getstr(checkSec); noecho();

        Item* target = matches[choice-1];
        if (strcmp(target->id, checkID) == 0 && levenshtein(checkSec, target->secret) <= 3) {
            int idx = calculateHash(target->name);
            Item* del = hashTable[idx], *prev = NULL;
            while(del) {
                if (del == target) {
                    if (!prev) hashTable[idx] = del->next; else prev->next = del->next;
                    free(del); saveToFile();
                    mvprintw(15, 4, "RECLAIMED SUCCESSFULLY!");
                    break;
                }
                prev = del; del = del->next;
            }
        } else mvprintw(15, 4, "VERIFICATION FAILED!");
    }
    refresh(); getch();
}

int main() {
    initscr(); raw(); keypad(stdscr, TRUE); noecho(); start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    loadFromFile();

    char *choices[] = {"Add Found Item", "Search / Reclaim", "View All Items", "Exit"};
    int highlight = 0;

    while (1) {
        clear();
        draw_box(2, 5, 10, 40, "LOST & FOUND SYSTEM");
        for(int i = 0; i < 4; i++) {
            if(i == highlight) attron(A_REVERSE);
            mvprintw(4 + i, 10, choices[i]);
            attroff(A_REVERSE);
        }
        refresh();

        int c = getch();
        if (c == KEY_UP && highlight > 0) highlight--;
        else if (c == KEY_DOWN && highlight < 3) highlight++;
        else if (c == 10) { // Enter Key
            if (highlight == 0) addItemUI();
            else if (highlight == 1) searchItemUI();
            else if (highlight == 2) viewItemsUI();
            else break;
        }
    }
    endwin();
    return 0;
}