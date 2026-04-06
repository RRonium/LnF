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

Item* hashTable[TABLE_SIZE] = {NULL};

int min3(int a,int b,int c){
    if(a<=b && a<=c) return a;
    if(b<=a && b<=c) return b;
    return c;
}

int calculateHash(char* str){
    unsigned long hash=5381;
    int c;
    while((c=*str++))
        hash=((hash<<5)+hash)+c;
    return hash % TABLE_SIZE;
}


void generateID(char* category,char* id){
    static int counter=1001;
    snprintf(id,20,"%.2s-%d",category,counter++);
}


int rabinKarp(char* pat,char* txt){
    int M=strlen(pat),N=strlen(txt);
    if(M>N) return 0;
    int p=0,t=0,h=1;
    for(int i=0;i<M-1;i++) h=(h*256)%PRIME;
    for(int i=0;i<M;i++){
        p=(256*p+pat[i])%PRIME;
        t=(256*t+txt[i])%PRIME;
    }
    for(int i=0;i<=N-M;i++){
        if(p==t){
            int j;
            for(j=0;j<M;j++) if(txt[i+j]!=pat[j]) break;
            if(j==M) return 1;
        }
        if(i<N-M){
            t=(256*(t-txt[i]*h)+txt[i+M])%PRIME;
            if(t<0) t+=PRIME;
        }
    }
    return 0;
}


int levenshtein(char* s1,char* s2){
    int l1=strlen(s1),l2=strlen(s2);
    int d[l1+1][l2+1];
    for(int i=0;i<=l1;i++) d[i][0]=i;
    for(int j=0;j<=l2;j++) d[0][j]=j;
    for(int i=1;i<=l1;i++)
        for(int j=1;j<=l2;j++){
            int cost=(s1[i-1]==s2[j-1])?0:1;
            d[i][j]=min3(d[i-1][j]+1,d[i][j-1]+1,d[i-1][j-1]+cost);
        }
    return d[l1][l2];
}


void saveToFile(){
    FILE* file=fopen("database.txt","w");
    if(!file) return;
    for(int i=0;i<TABLE_SIZE;i++){
        Item* temp=hashTable[i];
        while(temp){
            fprintf(file,"%s|%s|%s|%s|%s|%s|%s\n",
            temp->id,temp->name,temp->category,
            temp->location,temp->date,
            temp->description,temp->secret);
            temp=temp->next;
        }
    }
    fclose(file);
}


void loadFromFile(){
    FILE* file=fopen("database.txt","r");
    if(!file) return;

    while(1){
        Item* newItem=(Item*)malloc(sizeof(Item));
        if(fscanf(file," %[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]\n",
            newItem->id,newItem->name,newItem->category,
            newItem->location,newItem->date,
            newItem->description,newItem->secret)!=7){
            free(newItem);
            break;
        }
        int index=calculateHash(newItem->name);
        newItem->next=hashTable[index];
        hashTable[index]=newItem;
    }
    fclose(file);
}

void freeMemory(){
    for(int i=0;i<TABLE_SIZE;i++){
        Item* curr=hashTable[i];
        while(curr){
            Item* temp=curr;
            curr=curr->next;
            free(temp);
        }
    }
}

void draw_box(int y,int x,int h,int w,char* title){
    mvprintw(y,x,"+");
    for(int i=1;i<w-1;i++) printw("-");
    printw("+");
    for(int i=1;i<h-1;i++){
        mvprintw(y+i,x,"|");
        mvprintw(y+i,x+w-1,"|");
    }
    mvprintw(y+h-1,x,"+");
    for(int i=1;i<w-1;i++) printw("-");
    printw("+");
    if(title)
        mvprintw(y,x+(w-strlen(title))/2," %s ",title);
}

void addItemUI(){
    clear();
    draw_box(1,2,20,70,"LOG NEW FOUND ITEM");

    Item* newItem=(Item*)malloc(sizeof(Item));
    if(!newItem) return;

    const char* categories[] = {"Stationary", "Bags", "Devices", "Accessories"};
    int numCategories = sizeof(categories)/sizeof(categories[0]);

    mvprintw(3,4,"Available Categories:");
    for(int i=0;i<numCategories;i++)
        mvprintw(4+i,6,"%d. %s",i+1,categories[i]);

    echo();
    mvprintw(9,4,"Enter Category (number or name): ");
    char input[30];
    getnstr(input,29);

    int catIndex=-1;
    if(strlen(input)==1 && input[0]>='1' && input[0]<='0'+numCategories){
        catIndex=input[0]-'1';
        strcpy(newItem->category,categories[catIndex]);
    } else {
        strcpy(newItem->category,input);
    }

    mvprintw(10,4,"Name: "); getnstr(newItem->name,49);
    mvprintw(11,4,"Description: "); getnstr(newItem->description,99);
    mvprintw(12,4,"Location: "); getnstr(newItem->location,49);
    mvprintw(13,4,"Date (DD/MM/YYYY): "); getnstr(newItem->date,14);
    mvprintw(14,4,"Secret: "); getnstr(newItem->secret,49);
    noecho();

    generateID(newItem->category,newItem->id);
    int index=calculateHash(newItem->name);
    newItem->next=hashTable[index];
    hashTable[index]=newItem;
    saveToFile();

    attron(COLOR_PAIR(2)|A_BOLD);
    mvprintw(16,4,"SUCCESS! Unique ID: %s",newItem->id);
    attroff(COLOR_PAIR(2)|A_BOLD);
    mvprintw(18,4,"Press any key to return...");
    refresh();
    getch();
}

void viewItemsUI(){
    clear();
    draw_box(1,1,22,110,"DATABASE");
    int row=4;
    for(int i=0;i<TABLE_SIZE;i++){
        Item* temp=hashTable[i];
        while(temp && row<20){
            mvprintw(row++,3,"%s | %s | %s | %s | %s", temp->id,temp->name,temp->category,temp->date,temp->location);
            temp=temp->next;
        }
    }
    mvprintw(21,3,"Press any key...");
    getch();
}


void searchItemUI(){
    clear();

    const char* categories[] = {"Stationary", "Bags", "Devices", "Accessories"};
    int numCategories = sizeof(categories)/sizeof(categories[0]);

    char cat[30], query[50];
    draw_box(1,2,15,70,"SEARCH");

    mvprintw(3,4,"Available Categories:");
    for(int i=0;i<numCategories;i++)
        mvprintw(4+i,6,"%d. %s",i+1,categories[i]);

    echo();
    mvprintw(9,4,"Enter Category (number or name): ");
    char input[30];
    getnstr(input,29);

    int catIndex=-1;
    if(strlen(input)==1 && input[0]>='1' && input[0]<='0'+numCategories){
        catIndex=input[0]-'1';
        strcpy(cat, categories[catIndex]);
    } else {
        strcpy(cat, input);
    }

    mvprintw(10,4,"Enter search query: "); 
    getnstr(query,49);
    noecho();

    Item* matches[50]; 
    int count=0;
    for(int i=0;i<TABLE_SIZE;i++){
        Item* curr=hashTable[i];
        while(curr){
            if(strcmp(curr->category,cat)==0 &&
               (strcmp(curr->name,query)==0 || rabinKarp(query,curr->name) || levenshtein(query,curr->name)<=2)){
                matches[count++] = curr;
            }
            curr = curr->next;
        }
    }

    if(count==0){
        mvprintw(12,4,"No match found.");
        getch();
        return;
    }

    clear();
    draw_box(1,2,20,100,"RESULTS");

    for(int i=0;i<count;i++){
        mvprintw(3+i,4,"[%d] %s | %s | %s", i+1, matches[i]->id, matches[i]->name, matches[i]->category);
    }

    mvprintw(15,4,"Enter number to claim item (0 to exit): ");
    echo();
    int choice;
    scanw("%d",&choice);
    noecho();

    if(choice>0 && choice<=count){
        Item* itemToClaim = matches[choice-1];
        int index = calculateHash(itemToClaim->name);
        Item* curr = hashTable[index];
        Item* prev = NULL;

        while(curr){
            if(curr == itemToClaim){
                if(prev) prev->next = curr->next;
                else hashTable[index] = curr->next;

                free(curr);
                saveToFile();

                mvprintw(17,4,"Item claimed and removed successfully!");
                getch();
                return;
            }
            prev = curr;
            curr = curr->next;
        }
    } else {
        mvprintw(17,4,"No item claimed.");
        getch();
    }
}

int main(){
    initscr();
    keypad(stdscr,TRUE);
    noecho();
    start_color();
    init_pair(2,COLOR_GREEN,COLOR_BLACK);

    loadFromFile();

    char *menu[]={"Add Item","Search","View Items","Exit"};
    int highlight=0;

    while(1){
        clear();
        draw_box(2,5,10,40,"LOST & FOUND");
        for(int i=0;i<4;i++){
            if(i==highlight) attron(A_REVERSE);
            mvprintw(4+i,10,"%s",menu[i]);
            attroff(A_REVERSE);
        }

        int ch=getch();
        if(ch==KEY_UP && highlight>0) highlight--;
        else if(ch==KEY_DOWN && highlight<3) highlight++;
        else if(ch==10){
            if(highlight==0) addItemUI();
            else if(highlight==1) searchItemUI();
            else if(highlight==2) viewItemsUI();
            else break;
        }
    }

    freeMemory();
    endwin();
    return 0;
}
