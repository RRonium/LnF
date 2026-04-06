#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 101
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

int min(int a, int b, int c) {
    if (a <= b && a <= c) return a;
    if (b <= a && b <= c) return b;
    return c;
}

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
    int M = (int)strlen(pattern), N = (int)strlen(text);
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
    int s1len = (int)strlen(s1), s2len = (int)strlen(s2);
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
        if (newItem && fscanf(file, " %[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]\n", newItem->id, newItem->name, newItem->category, newItem->location, newItem->date, newItem->description, newItem->secret) == 7) {
            int index = calculateHash(newItem->name);
            newItem->next = hashTable[index];
            hashTable[index] = newItem;
        } else free(newItem);
    }
    fclose(file);
}

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
    if(title) mvprintw(y, x + (w - (int)strlen(title))/2, " %s ", title);
}

void addItemUI() {
    clear();
    draw_box(1, 2, 18, 70, "LOG NEW FOUND ITEM");
    Item* newItem = (Item*)malloc(sizeof(Item));
    
    mvprintw(3, 4, "Available: Stationary, Bags, Devices, Accessories");
    echo();
    mvprintw(5, 4, "Category: "); getnstr(newItem->category, 29);
    mvprintw(6, 4, "Item Name: "); getnstr(newItem->name, 49);
    mvprintw(7, 4, "Description: "); getnstr(newItem->description, 99);
    mvprintw(8, 4, "Location: "); getnstr(newItem->location, 49);
    mvprintw(9, 4, "Date (DD/MM/YYYY): "); getnstr(newItem->date, 14);
    mvprintw(10, 4, "Secret Detail: "); getnstr(newItem->secret, 49);
    noecho();

    generateID(newItem->category, newItem->id);
    int index = calculateHash(newItem->name);
    newItem->next = hashTable[index];
    hashTable[index] = newItem;
    saveToFile();

    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(12, 4, "SUCCESS! Unique ID: %s", newItem->id);
    attroff(COLOR_PAIR(2) | A_BOLD);
    mvprintw(14, 4, "Press any key to return...");
    refresh();
    getch();
}

void viewItemsUI() {
    clear();
    draw_box(1, 1, 22, 110, "CURRENT LOST & FOUND DATABASE");
    attron(A_BOLD);
    mvprintw(3, 3, "%-10s | %-15s | %-10s | %-15s | %-30s", "ID", "Name", "Date", "Location", "Description");
    attroff(A_BOLD);
    mvprintw(4, 3, "-------------------------------------------------------------------------------------------------------");
    
    int row = 5;
    for (int i = 0; i < TABLE_SIZE; i++) {
        Item* temp = hashTable[i];
        while (temp) {
            if(row < 20) { 
                mvprintw(row++, 3, "%-10s | %-15s | %-10s | %-15s | %-30.30s...", temp->id, temp->name, temp->date, temp->location, temp->description);
            }
            temp = temp->next;
        }
    }
    mvprintw(20, 3, "Press any key to return...");
    refresh();
    getch();
}

void searchItemUI() {
    clear();
    char cat[30], query[50];
    draw_box(1, 2, 10, 70, "SEARCH / RECLAIM");
    echo();
    mvprintw(3, 4, "Category: "); getnstr(cat, 29);
    mvprintw(4, 4, "Query: "); getnstr(query, 49); 
    noecho();

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
        refresh(); getch(); return;
    }

    clear();
    draw_box(1, 2, 20, 100, "RESULTS FOUND");
    attron(A_BOLD);
    mvprintw(2, 4, "%-3s %-10s | %-15s | %-10s | %-30s", "#", "ID", "Name", "Date", "Description");
    attroff(A_BOLD);
    
    for(int i=0; i<count; i++) {
        mvprintw(4+i, 4, "[%d] %-10s | %-15s | %-10s | %-30.30s...", i+1, matches[i]->id, matches[i]->name, matches[i]->date, matches[i]->description);
    }
    
    mvprintw(15, 4, "Select index to reclaim (0 to cancel): ");
    echo(); char choiceStr[5]; getnstr(choiceStr, 4); noecho();
    int choice = atoi(choiceStr);

    if (choice > 0 && choice <= count) {
        char checkID[20], checkSec[50];
        mvprintw(16, 4, "Confirm ID: "); echo(); getnstr(checkID, 19);
        mvprintw(17, 4, "Secret Detail: "); getnstr(checkSec, 49); noecho();

        Item* target = matches[choice-1];
        if (strcmp(target->id, checkID) == 0 && levenshtein(checkSec, target->secret) <= 3) {
            int idx = calculateHash(target->name);
            Item* del = hashTable[idx], *prev = NULL;
            while(del) {
                if (del == target) {
                    if (!prev) hashTable[idx] = del->next; else prev->next = del->next;
                    free(del); saveToFile();
                    attron(COLOR_PAIR(2) | A_BOLD);
                    mvprintw(18, 4, "RECLAIMED SUCCESSFULLY!");
                    attroff(COLOR_PAIR(2) | A_BOLD);
                    break;
                }
                prev = del; del = del->next;
            }
        } else {
            attron(COLOR_PAIR(1) | A_BOLD);
            mvprintw(18, 4, "VERIFICATION FAILED!");
            attroff(COLOR_PAIR(1) | A_BOLD);
        }
    }
    refresh(); getch();
}

int main() {
    initscr(); raw(); keypad(stdscr, TRUE); noecho(); start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_CYAN, COLOR_BLACK);
    
    loadFromFile();

    char *choices[] = {"Add Found Item", "Search / Reclaim", "View All Items", "Exit"};
    int highlight = 0;

    while (1) {
        clear();
        attron(COLOR_PAIR(3));
        draw_box(2, 5, 10, 45, "LOST & FOUND SYSTEM");
        attroff(COLOR_PAIR(3));
        
        for(int i = 0; i < 4; i++) {
            if(i == highlight) attron(A_REVERSE | A_BOLD);
            mvprintw(4 + i, 12, " [%s] ", choices[i]);
            attroff(A_REVERSE | A_BOLD);
        }
        
        mvprintw(13, 5, "Use ARROWS to navigate, ENTER to select.");
        refresh();

        int c = getch();
        if (c == KEY_UP && highlight > 0) highlight--;
        else if (c == KEY_DOWN && highlight < 3) highlight++;
        else if (c == 10) { 
            if (highlight == 0) addItemUI();
            else if (highlight == 1) searchItemUI();
            else if (highlight == 2) viewItemsUI();
            else break;
        }
    }
    endwin();
    return 0;
}