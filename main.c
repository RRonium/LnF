#include <stdio.h>
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

int min(int a, int b, int c) {
    if (a <= b && a <= c) return a;
    if (b <= a && b <= c) return b;
    return c;
}

int calculateHash(char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash % TABLE_SIZE;
}

void generateID(char* category, char* id) {
    static int counter = 1001;
    sprintf(id, "%.2s-%d", category, counter++);
}

int rabinKarp(char* pattern, char* text) {
    int M = strlen(pattern);
    int N = strlen(text);
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
    int s1len = strlen(s1);
    int s2len = strlen(s2);
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
    if (file == NULL) return;
    for (int i = 0; i < TABLE_SIZE; i++) {
        Item* temp = hashTable[i];
        while (temp != NULL) {
            fprintf(file, "%s|%s|%s|%s|%s|%s|%s\n", temp->id, temp->name, temp->category, temp->location, temp->date, temp->description, temp->secret);
            temp = temp->next;
        }
    }
    fclose(file);
}

void loadFromFile() {
    FILE *file = fopen("database.txt", "r");
    if (file == NULL) return;
    while (!feof(file)) {
        Item* newItem = (Item*)malloc(sizeof(Item));
        if (newItem == NULL) break;
        if (fscanf(file, " %[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]\n", newItem->id, newItem->name, newItem->category, newItem->location, newItem->date, newItem->description, newItem->secret) == 7) {
            int index = calculateHash(newItem->name);
            newItem->next = hashTable[index];
            hashTable[index] = newItem;
        } else {
            free(newItem);
        }
    }
    fclose(file);
}

void addItem() {
    Item* newItem = (Item*)malloc(sizeof(Item));
    printf("\nAvailable Categories: Stationary, Bags, Devices, Accessories\n");
    printf("Enter Category: ");
    scanf("%s", newItem->category);
    printf("Enter Item Name: ");
    scanf("%s", newItem->name);
    printf("Enter Description: ");
    scanf(" %[^\n]s", newItem->description);
    printf("Enter Location: ");
    scanf("%s", newItem->location);
    printf("Enter Date (DD/MM/YYYY): ");
    scanf("%s", newItem->date);
    printf("Setup Secret Detail: ");
    scanf(" %[^\n]s", newItem->secret);
    generateID(newItem->category, newItem->id);
    int index = calculateHash(newItem->name);
    newItem->next = hashTable[index];
    hashTable[index] = newItem;
    saveToFile();
    printf("\nItem Logged! UNIQUE ID: %s\n", newItem->id);
}

void viewItems() {
    int empty = 1;
    printf("\n--- CURRENT LOST & FOUND ITEMS ---\n");
    printf("%-10s | %-15s | %-15s | %-15s\n", "ID", "Name", "Category", "Location");
    printf("------------------------------------------------------------\n");
    for (int i = 0; i < TABLE_SIZE; i++) {
        Item* temp = hashTable[i];
        while (temp != NULL) {
            printf("%-10s | %-15s | %-15s | %-15s\n", temp->id, temp->name, temp->category, temp->location);
            temp = temp->next;
            empty = 0;
        }
    }
    if (empty) printf("The database is currently empty.\n");
}

void searchItem() {
    char category[30], query[50];
    printf("\nAvailable Categories: Stationary, Bags, Devices, Accessories\n");
    printf("Enter Category: ");
    scanf("%s", category);
    printf("Enter Search Query: ");
    scanf("%s", query);
    Item* matches[50];
    int foundCount = 0;
    for (int i = 0; i < TABLE_SIZE; i++) {
        Item* curr = hashTable[i];
        while (curr != NULL) {
            if (strcmp(curr->category, category) == 0) {
                if (strcmp(curr->name, query) == 0 || rabinKarp(query, curr->name) || rabinKarp(query, curr->description) || levenshtein(query, curr->name) <= 2) {
                    matches[foundCount++] = curr;
                }
            }
            curr = curr->next;
        }
    }
    if (foundCount == 0) {
        printf("\nNo matches found.\n");
        return;
    }
    printf("\nMatches Found:\n");
    for (int i = 0; i < foundCount; i++) printf("[%d] ID: %s | Name: %s | Location: %s\n", i + 1, matches[i]->id, matches[i]->name, matches[i]->location);
    int choice;
    printf("\nEnter index to reclaim (0 to cancel): ");
    scanf("%d", &choice);
    if (choice > 0 && choice <= foundCount) {
        char checkID[20], checkSecret[50];
        Item* target = matches[choice - 1];
        printf("Confirm Unique ID: ");
        scanf("%s", checkID);
        printf("Enter Secret Detail: ");
        scanf(" %[^\n]s", checkSecret);
        if (strcmp(target->id, checkID) == 0 && levenshtein(checkSecret, target->secret) <= 3) {
            int idx = calculateHash(target->name);
            Item* del = hashTable[idx], *prev = NULL;
            while (del != NULL) {
                if (del == target) {
                    if (prev == NULL) hashTable[idx] = del->next;
                    else prev->next = del->next;
                    free(del);
                    saveToFile();
                    printf("\nItem reclaimed successfully.\n");
                    return;
                }
                prev = del; del = del->next;
            }
        } else printf("\nVerification failed. ID mismatch or secret detail too different.\n");
    }
}

int main() {
    for (int i = 0; i < TABLE_SIZE; i++) hashTable[i] = NULL;
    loadFromFile();
    int choice;
    while (1) {
        printf("\n--- LOST & FOUND SYSTEM ---\n");
        printf("1. Add Found Item\n2. Search/Reclaim Item\n3. View All Items\n4. Exit\n>> ");
        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n'); 
            continue;
        }
        switch (choice) {
            case 1: addItem(); break;
            case 2: searchItem(); break;
            case 3: viewItems(); break;
            case 4: saveToFile(); exit(0);
            default: printf("Invalid choice.\n");
        }
    }
    return 0;
}