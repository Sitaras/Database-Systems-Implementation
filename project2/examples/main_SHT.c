#include "HT.h"
#include "SHT.h"

#define NUM_OF_BUCKETS 100


int main(int argc, char** argv){
  char filenameHT [10];
  char filenameSHT [10];
  strcpy(filenameHT, FILENAME1);
  strcpy(filenameSHT, FILENAME2);
  // initialize BF library
  BF_Init();

  HT_CreateIndex(filenameHT,'i',"id",10,NUM_OF_BUCKETS); // the last argument is the number of buckets and is defined above, so it can be changed
  SHT_CreateSecondaryIndex(filenameSHT,"surname",10,NUM_OF_BUCKETS,filenameHT);

  HT_info *info=HT_OpenIndex(filenameHT);   // open primary HashTable
  SHT_info *shInfo=SHT_OpenSecondaryIndex(filenameSHT); // open secondary HashTable
  printf("HT BLOCKS:%d\n",BF_GetBlockCounter(info->fileDesc));
  printf("SHT BLOCKS:%d\n",BF_GetBlockCounter(shInfo->fileDesc));

  FILE *fp;
  Record r;
  char buffer[100];
  int recordsCounter=0;   // counts how many records have been succesfully inserted
  fp=fopen("../examples/records1K.txt","r");  // select the file from which we will insert the records
  if(fp==NULL){
    perror("Error");
    return -1;
  }

  // ----- INSERTING RECORDS -----
  printf(">Records Inserting. (1000 records)\n");
  while(!feof(fp)){
    if(fscanf(fp,"%s",buffer)<0){ //read a line from the file
      continue;
    }
    sscanf (buffer,"{ %d, \"%[^\"]\" , \"%[^\"]\" , \"%[^\"]\" } ",&(r.id),r.name,r.surname,r.address); // format the line we read, and store the info in a Record variable
    // printf("-*-*-*-* READED id %d\n",(r.id));
    int blockInserted=HT_InsertEntry(*info,r);
    if(blockInserted!=-1) { // if it was inserted succesfully
      recordsCounter++;
      SecondaryRecord sr;   // create the SecondaryRecord that will be given to SHT_SecondaryInsertEntry as argument
      sr.record=r;
      sr.blockId=blockInserted;
      SHT_SecondaryInsertEntry(*shInfo,sr);   // insert the entry to the secondary HashTable
    }
    // printf("%d %s %s %s \n \n",(r.id),r.name,r.surname,r.address);
  }
  fclose(fp);


  time_t t;
  srand((unsigned) time(&t));
  printf("\nChecking if random records exist in primary HT\n");
  int random;
  for(int i=0;i<15;i++){
      random=rand()%(recordsCounter);   // generate a random id
      printf("-----------------------------------------\n");
      printf("* Searching record with id: %d\n",random);
      //check if the record exists
      if(HT_GetAllEntries(*info,&random)==-1){
        printf("HT_GetAllEntries: Not found record with id %d\n",random);
      }
      printf("-----------------------------------------\n");
  }

  printf("\n\nChecking if random records exist in secondary SHT (searching by surname)\n");
  char surname[15];

  for(int i=0;i<15;i++){
      random=rand() % (recordsCounter);   // generate a random id
      sprintf(surname,"surname_%d",random);   // generate a random surname using the random id
      printf("-----------------------------------------\n");
      printf("* Searching record with surname: %s\n",surname);
      //check if the record exists in the secondary HashTable
      if(SHT_SecondaryGetAllEntries(*shInfo,*info,surname)==-1){
        printf("SHT_SecondaryGetAllEntries: Not found record with surname %s\n",surname);
      }
      printf("-----------------------------------------\n");
  }

  printf("\n\nTotal Blocks of file:%d\n",BF_GetBlockCounter(info->fileDesc) );
  printf("______________________________________\n");

  HT_CloseIndex(info);    // close primary HashTable file
  SHT_CloseSecondaryIndex(shInfo);  // close secondary HashTable file
  printf("______________________________________\n\n");

  HashStatistics(filenameHT);   //get statistics for the primary hash table
  HashStatistics(filenameSHT);  //get statistics for the secondary hash table


  return 0;
}
