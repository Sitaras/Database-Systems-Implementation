#include "HP.h"


int main(int argc, char** argv){

  char filename[10];
  strcpy(filename, FILENAME);
  // initialize BF library
  BF_Init();

  HP_CreateFile(filename,'i',"id",10);

  HP_info *info=HP_OpenFile(filename);

  FILE *fp;
  Record r;
  char buffer[100];
  int recordsCounter=0;   // counts how many records have been succesfully inserted
  fp=fopen("../examples/records500.txt","r");
  // fp=fopen("../records_examples/records1k.txt","r");
  // fp=fopen("../records_examples/records5k.txt","r");
  // fp=fopen("../records_examples/records10k.txt","r");
  // fp=fopen("../records_examples/records15k.txt","r");
  if(fp==NULL){
    perror("Error");
    return -1;
  }

  // ----- INSERTING RECORDS -----
  printf(">Records Inserting. (500 records)\n");
  while(!feof(fp)){
    if(fscanf(fp,"%s",buffer)<0){ //read a line from the file
      continue;
    }
    sscanf (buffer,"{ %d, \"%[^\"]\" , \"%[^\"]\" , \"%[^\"]\" } ",&(r.id),r.name,r.surname,r.address);   // format the line we read, and store the info in a Record variable
    // printf("-*-*-*-* READED id %d\n",(r.id));
    if(HP_InsertEntry(*info,r)!=-1) { // insert the record to the heap
      recordsCounter++;
    }
    // printf("%d %s %s %s \n \n",(r.id),r.name,r.surname,r.address);
  }
  fclose(fp);
  printf("______________________________________\n");



  time_t t;
  srand((unsigned) time(&t));
  int random=rand() % (recordsCounter);
  if (random+15<recordsCounter){
    printf("Checking if random records exist, deleting them and then checking again if they exist after the deletion\n");
    for(int j=random;j<=random+15;j++){ // selecting some random records
      printf("______________________________________\n");
      //check if the record exists
      if(HP_GetAllEntries(*info,&j)==-1){
        printf("HT_GetAllEntries: Not found record with id %d\n",j);
      }
      // delete the record
      if(HP_DeleteEntry(*info,&j)==-1){
        printf("HT_DeleteEntry: Not found record with id %d\n",j);
      }
      // check if the same record exists, so as to see if it was deleted correctly
      if(HP_GetAllEntries(*info,&j)==-1){
        printf("HT_GetAllEntries: Not found record with id %d\n",j);
      }
      printf("______________________________________\n");
    }
  }




  HP_CloseFile(info);



  return 0;
}
