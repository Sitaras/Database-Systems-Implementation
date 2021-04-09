#include "HT.h"

#define NUM_OF_BUCKETS 100


int main(int argc, char** argv){
  char filename [10];
  strcpy(filename, FILENAME);
  // initialize BF library
  BF_Init();

  HT_CreateIndex(filename,'i',"id",10,NUM_OF_BUCKETS); // the last argument is the number of buckets and is defined above, so it can be changed
  HT_info *info=HT_OpenIndex(filename);
  printf("BLOCKS:%d\n",BF_GetBlockCounter(info->fileDesc));

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
    if(HT_InsertEntry(*info,r)!=-1) { // insert the record to the hashTable
      recordsCounter++;
    }
    // printf("%d %s %s %s \n \n",(r.id),r.name,r.surname,r.address);
  }
  fclose(fp);

  time_t t;
  srand((unsigned) time(&t));
  int random=rand() % (recordsCounter);
  if (random+15<recordsCounter){
    printf("Checking if random records exist, deleting them and then checking again if they exist after the deletion\n");
    for(int j=random;j<=random+15;j++){ // selecting some random records
      printf("______________________________________\n");
      //check if the record exists
      if(HT_GetAllEntries(*info,&j)==-1){
        printf("HT_GetAllEntries: Not found record with id %d\n",j);
      }
      // delete the record
      if(HT_DeleteEntry(*info,&j)==-1){
        printf("HT_DeleteEntry: Not found record with id %d\n",j);
      }
      // check if the same record exists, so as to see if it was deleted correctly
      if(HT_GetAllEntries(*info,&j)==-1){
        printf("HT_GetAllEntries: Not found record with id %d\n",j);
      }
      printf("______________________________________\n");
    }
  }


  printf("\n\nTotal Blocks of file:%d\n",BF_GetBlockCounter(info->fileDesc) );
  printf("______________________________________\n");

  HT_CloseIndex(info);
  printf("______________________________________\n\n");

  HashStatistics(filename);


  return 0;
}
