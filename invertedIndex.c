#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "invertedIndex.h"

//checks whether or not character is alphanumeric
int isAlphaNumeric(char c){  
    if((c >= 48 && c <= 57) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122)){
        return 1;
    }
    else{
        return 0;
    }
}

//checks whether or not character is a letter
int isAlpha(char c){
    if((c >= 65 && c <= 90) || (c >= 97 && c <= 122)){
        return 1;
    }
    else{
        return 0;
    }
}

//adds word to linked list
void addWord(char* wordString, char* fileName){
    int len = strlen(wordString);
    int lenFileName = strlen(fileName);

    //if no words have been made so far, initialize linked list
    if(head == NULL){
        head = (word*) malloc(sizeof(word));
        head->text = (char*) malloc(sizeof(char) * len);
        head->files = (file*) malloc(sizeof(file));
        head->files->fileName = (char*) malloc(sizeof(char) * lenFileName);
        strncpy(head->text, wordString, len);
        strncpy(head->files->fileName, fileName, lenFileName);
        head->files->count = 1;
        head->files->next = NULL;
        head->next = NULL;
    }
    else{
        //find the instance of word if it exists within the list
        word * temp = head;
        while(temp != NULL){
            //if an instance of the word exists in the list, find an instance of the file that it was currently found in
            if(strcmp(wordString, temp->text) == 0){
                file * files = temp->files;
                while(files != NULL){
                    //increase count at file for word and return
                    if(strcmp(files->fileName, fileName) == 0){
                        files->count++;
                        return;
                    }
                    //if no file exists currently, add one
                    if(files->next == NULL){
                        files->next = (file*) malloc(sizeof(file));
                        files->next->fileName = (char*) malloc(sizeof(char) * lenFileName);
                        strncpy(files->next->fileName, fileName, lenFileName);
                        files->next->count = 1;
                        files->next->next = NULL;
                        return;
                    }
                    files = files->next;
                }
            }
            //if word does not exist in list, create word
            if(temp->next == NULL){
                temp->next = (word*) malloc(sizeof(word));
                temp->next->text = (char*) malloc(sizeof(char) * len);
                strncpy(temp->next->text, wordString, len);
                temp->next->next = NULL;
                temp->next->files = (file*) malloc(sizeof(file));
                temp->next->files->fileName = (char*) malloc(sizeof(char) * lenFileName);
                strncpy(temp->next->files->fileName, fileName, lenFileName);
                temp->next->files->count = 1;
                temp->next->files->next = NULL;
                return;
            }
            temp = temp->next;
        }
    }
}

//protocol for reading in a file
void processFile(char* path, char* name){
    int fd = open(path, O_RDONLY);

    if(fd < 0){
        printf("Error occurred opening file: %s\n", path);
        return;
    }

    char buffer[BUFFSIZE];
    int bytes;

    char * carry = NULL;
    int carrySize = 0;
    int carried = 0;

    //loop through all bytes of file
    while((bytes = read(fd, buffer, BUFFSIZE)) > 0){
        int i;
        for(i = 0; i < bytes; i++){
            //check whether or not the current character is a letter
            if(isAlpha(buffer[i]) == 1){
                //count the length of the following alphanumeric characters
                int size = 1;
                int j;
                for(j = i+1; j < bytes; j++){
                    if(isAlphaNumeric(buffer[j]) == 1){
                        size++;
                    }
                    else{
                        break;
                    }
                }
                //if word spands to end of buffer, add the word to the carried array so that it can be concatenated to the front of the next buffer
                if(j == bytes){
                    if(carry != NULL){
                        free(carry);
                    }
                    carry = (char*) malloc(sizeof(char) * size);
                    strncpy(carry, &buffer[i], size);
                    carried = 1;
                    carrySize = size;
		   
		  
                    break;
                }
                //if there is a carried word, concatenate it to the front of the current word
                char* text;
                if(carried == 1){
                    text = (char*) malloc((sizeof(char) * (carrySize + size)) + 1);
                    strncpy(text, carry, carrySize);
                    strncpy(&text[carrySize], &buffer[i], size);
                    text[carrySize + size] = '\0';
                    carried = 0;
                    carrySize = 0;
                    free(carry);
                    carry = NULL;
                }
                //regular word, copy into a word buffer
                else{
                    text = (char*) malloc((sizeof(char) * size) + 1);
                    strncpy(text, &buffer[i], size);
                    text[size] = '\0';
                }
                //convert word to lower
                int x; 
                for(x = 0; text[x]; x++){
                    text[x] = tolower(text[x]);
                }
                //process word
                addWord(text, name);
                free(text);
                //move to next non alphanumeric character
                i = j-1; 
            }
        }
    }
    //free carry
    if(carry != NULL){
        free(carry);
        carry = NULL;
    }

    if(bytes == -1){
        printf("Error occurred reading file: %s\n", path);
        close(fd);
        return;
    }

    close(fd);
}

//recursive directory method
void processDirectory(char* dir){
    DIR * directory = opendir(dir);
    if(directory == NULL){
        printf("Could not open directory: %s\n", dir);
        return;
    }

    //if directory was opened, read each entity
    struct dirent * de;
    while((de = readdir(directory)) != NULL){
        //add the entity to the current directory path
        char * path = (char*) malloc(sizeof(char) * (strlen(dir) + strlen(de->d_name)) + 2);
        strncpy(path, dir, strlen(dir));
        path[strlen(dir)] = '/';
        strncpy(&path[strlen(dir) + 1], de->d_name, strlen(de->d_name));
        path[strlen(dir) + strlen(de->d_name) + 1] = '\0';

        //if file, process as file
        if(de->d_type == DT_REG){
            processFile(path, de->d_name);
        }
        //if directory recurse except for . and ..
        else if(de->d_type == DT_DIR){
            if(strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0){
                processDirectory(path);
            }
        }
        free(path);
    }
    closedir(directory);
}

//sort words in linked list (selection sort)
void sortWords(){
    word * ptr = head;
    word * prevPtr = NULL;
    //find smallest element in list
    while(ptr != NULL){
        word * temp = ptr->next;
        word * prev = ptr;
        word * small = ptr;
        word * prevSmall = NULL;
        while(temp != NULL){
            if(strcmp(temp->text, small->text) < 0){
                prevSmall = prev;
                small = temp;
            }
            prev = temp;
            temp = temp->next;
        }
        //swap with ptr and update the head if necessary
        if(small == ptr){
            prevPtr = ptr;
            ptr = ptr->next;
            continue;
        }
        if(prevPtr != NULL){
            prevPtr->next = small;
        }
        else{
            head = small;
        }
        prevSmall->next = ptr;
        word * t = small->next;
        small->next = ptr->next;
        ptr->next = t;
        prevPtr = small;
        ptr = small->next;
    }
}

//sort files in word linked list (selection sort)
file* sortFiles(file * f){
    file* top = f;
    file * ptr = f;
    file * prevPtr = NULL;
    //find largest count file
    while(ptr != NULL){
        file * temp = ptr->next;
        file * prev = ptr;
        file * large = ptr;
        file * prevLarge = NULL;
        while(temp != NULL){
            if(temp->count > large->count){
                prevLarge = prev;
                large = temp;
            }
            prev = temp;
            temp = temp->next;
        }
        //swap with ptr and update the head if necessary
        if(large == ptr){
            prevPtr = ptr;
            ptr = ptr->next;
            continue;
        }
        if(prevPtr != NULL){
            prevPtr->next = large;
        }
        else{
            top = large;
        }
        prevLarge->next = ptr;
        file * t = large->next;
        large->next = ptr->next;
        ptr->next = t;
        prevPtr = large;
        ptr = large->next;
    }
    return top;
}

int main(int argc, char** argv){
    //check for correct number of arguments
    if(argc != 3){
        printf("Incorrect usage. Use as follows: invertedIndex <inverted-index file name> <directory or file name>\n");
        return 1;
    }

    //check if the entry is a file
    DIR * directory = opendir(argv[2]);
    if(directory == NULL){
        processFile(argv[2], argv[2]);
    }
    //if it is a directory, begin driver
    else{
        struct dirent * de;
        while((de = readdir(directory)) != NULL){
            char * path = (char*) malloc(sizeof(char) * (strlen(argv[2]) + strlen(de->d_name)) + 2);
            strncpy(path, argv[2], strlen(argv[2]));
            path[strlen(argv[2])] = '/';
            strncpy(&path[strlen(argv[2]) + 1], de->d_name, strlen(de->d_name));
            path[strlen(argv[2]) + strlen(de->d_name) + 1] = '\0';
            if(de->d_type == DT_REG){
                processFile(path, de->d_name);
            }
            else if(de->d_type == DT_DIR){
                if(strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0){
                    processDirectory(path);
                }
            }
            free(path);
        }
    }

    closedir(directory);

    //sort the words and for each word sort the files
    sortWords();
    word * ptr = head;
    while(ptr != NULL){
        ptr->files = sortFiles(ptr->files);
        ptr = ptr->next;
    }

    //write to file
    int fileD = open(argv[1], O_WRONLY | O_CREAT, 0600);
    if(access(argv[1], F_OK) != -1) {
	printf("File with same name as index argument exists. Would you like to overwrite it? (y/n)\n");
	char line[1024];
	char *str = fgets(line, 1024, stdin);
	while(!(strlen(str) == 2 && (str[0] == 'y' || str[0] == 'Y' || str[0] == 'n' || str[0] == 'N'))) {
		printf("Please enter y/n\n");
		str = fgets(line, 1024, stdin);
	}
	if(strlen(str) == 2 && (str[0] == 'n' || str[0] == 'N'))
		exit(0);
    }
    if(fileD > 0){
        write(fileD, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", 39);
        write(fileD, "<fileIndex>\n", 12);
        ptr = head;
        while(ptr != NULL){
            write(fileD, "\t<word text=\"", 13);
            write(fileD, ptr->text, strlen(ptr->text));
            write(fileD, "\">\n", 3);
            file* f = ptr->files;
            while(f != NULL){
                write(fileD, "\t\t<file name=\"", 14);
                write(fileD, f->fileName, strlen(f->fileName));
                write(fileD, "\">", 2);

                char str[12];
                sprintf(str, "%d", f->count);

                write(fileD, str, strlen(str));
                write(fileD, "</file>\n", 8);
                f = f->next;
            }
            write(fileD, "\t</word>\n", 9);
            ptr = ptr->next;
        }
        write(fileD, "</fileIndex>", 12);
    }
    close(fileD);

    ptr = head;
    while(ptr != NULL){
        word * prev = ptr;
        file * f = prev->files;
        while(f != NULL){
            file * fprev = f;
            free(f->fileName);
            f = f->next;
            free(fprev);
        }
        free(ptr->text);
        ptr = ptr->next;
        free(prev);
    }

    return 0;
}
