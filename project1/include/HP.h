#ifndef HP_H
#define HP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FILENAME "ask1_HT"
#include "BF.h"

//////////////////////////
#define YES 1
#define NO 0


typedef struct kd{
  int fileDesc;
  char keyType;
  char keyName[15];
  int keyLength;
  int isHeap;
  int isHash;
  int headerBlock;
}HP_info;


typedef struct{
  int id;
  char name[15];
  char surname[25];
  char address[50];
}Record;



// int HP_CreateFile(char *,char ,char* ,int ,int );
int HP_CreateFile(char *,char ,char* ,int);

HP_info* HP_OpenFile(char *);

int HP_CloseFile(HP_info*);

int HP_InsertEntry(HP_info ,Record );

int HP_GetAllEntries(HP_info ,void* );

int HP_DeleteEntry( HP_info , void * );

#endif
