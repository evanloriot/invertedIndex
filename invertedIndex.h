#ifndef INVERTEDINDEX_H
#define INVERTEDINDEX_H

#define BUFFSIZE 1024

typedef struct file{
    char * fileName;
    int count;
    struct file * next;
} file;

typedef struct word{
    char * text;
    struct word * next;
    struct file * files;
} word;

word * head = NULL;

int isAlphaNumeric(char);
int isAlpha(char);
void addWord(char*, char*);
void processFile(char*, char*);
void processDirectory(char*);
void sortWords();
file* sortFiles(file*);

#endif
