#ifndef HT_H
#define HT_H

#define YES 1
#define NO 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "BF.h"

#define FILENAME1 "ask2_HT"
#define FILENAME2 "ask2_SHT"

typedef struct kd{
  int fileDesc;
  char keyType;
  char keyName[15];
  int keyLength;
  int buckets;
  int isHeap;
  int isHash;
  int headerBlock;
}HT_info;




typedef struct{
  int id;
  char name[15];
  char surname[25];
  char address[50];
}Record;




int HT_CreateIndex(char* ,char ,char*,int ,int);
HT_info* HT_OpenIndex(char*);
int HT_CloseIndex(HT_info*);
int HT_InsertEntry(HT_info ,Record);
int HT_DeleteEntry( HT_info ,void *);
int HT_GetAllEntries(HT_info , void*);
int HT_HashStatistics(char* );

#endif
