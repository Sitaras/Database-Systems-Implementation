#ifndef SHT_H
#define SHT_H

#define YES 1
#define NO 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "HT.h"
#include "BF.h"


typedef struct {
  int fileDesc;
  char attrName[15];
  int attrLength;
  long int numBuckets;
  char fileName[15];
  int headerBlock;    // pointer to the first block of the file that the actual SHT_info is stored
} SHT_info;


typedef struct{
  Record record;
  int blockId;
}SecondaryRecord;   // as given by the exercise

typedef struct{
  char surname[25];
  int blockId;
}secRec;   // the entry that is actualy saved in the secondary HashTable, as we only need surname and blockId for every entry


int SHT_CreateSecondaryIndex( char *, char* ,int ,int , char* );
SHT_info* SHT_OpenSecondaryIndex(char*);
int SHT_CloseSecondaryIndex( SHT_info*);
int SHT_SecondaryInsertEntry( SHT_info ,SecondaryRecord);
int SHT_SecondaryGetAllEntries(SHT_info , HT_info , void * );
int SHT_HashStatistics(char* );
int HashStatistics(char* );

#endif
