#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HT.h"
#include "BF.h"


int HT_CreateIndex( char *fileName,char attrType,char* attrName, int attrLength, int buckets){
  // create and initialize a hash file with name fileName
  // the first block has only the file header (HT_info)
  // The next block(s) contain the empty hashTable that is created and initialized
        HT_info info;
        int fd;
        int blkCnt;
        void* block;
        // key could be integer type or char type
        if(attrType!='c' && attrType!='i') {
                printf("Wrong key type\n");
                return -1;
        }
        // create the file with the given name (fileName)
        if (BF_CreateFile(fileName) < 0) {
                BF_PrintError("Error creating file");
                exit(EXIT_FAILURE);
        }
        // open the file that was just created
        if ((fd = BF_OpenFile(fileName)) < 0) {
                BF_PrintError("Error opening file");
                return -1;
        }

        // allocate a new block so as to save the file header: HT_info
        if (BF_AllocateBlock(fd) < 0) {
                BF_PrintError("Error allocating block");
                return -1;
        }

        blkCnt = BF_GetBlockCounter(fd);
        printf("File %d has %d blocks\n", fd, blkCnt);
        int headerBlockNum=BF_GetBlockCounter(fd)-1;
        // get the block that was just allocated
        if (BF_ReadBlock(fd, headerBlockNum, &block) < 0) {
                BF_PrintError("Error getting block");
                return -1;
        }
        // initialize the HT_info with the given info
        info.keyType= attrType;
        info.keyLength= attrLength;
        info.buckets=buckets;
        strcpy(info.keyName,attrName);
        info.isHeap=NO;
        info.isHash=YES;
        info.headerBlock=headerBlockNum;
        // save the HT_info in the first block (that we allocated and read above)
        memcpy(block,&info,sizeof(HT_info));
        block+=sizeof(HT_info); // skip HT_info

        int firstBucketBlock=-1; // pointer to the first block that cointains the hashTable
        memcpy(block,&firstBucketBlock,sizeof(int));

        // write the block to save the changes that we made to the block
        if (BF_WriteBlock(fd, headerBlockNum) < 0) {
                BF_PrintError("Error writing block back");
                return -1;
        }

        // create the hash table
        int prevBucketBlock=0;

        int numOfPositions;
        int size=sizeof(int)*buckets; // total size that the hashTable needs
        int numOfBlockNeeded=size/(BLOCK_SIZE-2*sizeof(int)); // compute how many blocks we need so as to store the hashTable
        if(size%(BLOCK_SIZE-2*sizeof(int))!=0) {
                numOfBlockNeeded++;
        }
        //start creating the hashTable
        for(int i=1; i<=numOfBlockNeeded; i++) {  // for every block that we need so as to store the hashTable
                if (BF_AllocateBlock(fd) < 0) {   // allocate a new block
                        BF_PrintError("Error allocating block");
                        return -1;
                }
                // attach the new block to the previous block
                int newBlock=BF_GetBlockCounter(fd)-1;  // the number of the new block that we created
                if (BF_ReadBlock(fd, prevBucketBlock, &block) < 0) {    // read the previous block
                        BF_PrintError("Error getting block");
                        return -1;
                }
                if(prevBucketBlock==0){   // if the previous block is the header block (= the first block of the file)
                  block+=sizeof(HT_info);   // skip the HT_info that the first block contains, so as to store the pointer to the new block
                }else{
                  block+=sizeof(int);   // if the previous block is a block that contains a part of the hashTable, skip the first int that is the number of bucket
                  //  so as to store the pointer to the new block
                }
                firstBucketBlock=newBlock;
                memcpy(block,&firstBucketBlock,sizeof(int));    // copy the "pointer" to the new block
                if (BF_WriteBlock(fd, prevBucketBlock) < 0) {   // write the previous block so he change of the pointer is saved
                        BF_PrintError("Error writing block back");
                        return -1;
                }

                if (BF_ReadBlock(fd, newBlock, &block) < 0) {    // read the new block that was just allocated
                        BF_PrintError("Error getting block");
                        return -1;
                }
                if(i==numOfBlockNeeded) { // if we are in the last block needed for the hashTable
                        numOfPositions=buckets-(((BLOCK_SIZE-2*sizeof(int))/(sizeof(int))*(i-1)));  // compute how many buckets are left
                }else{
                        numOfPositions=(BLOCK_SIZE-2*sizeof(int))/sizeof(int);  // compute the max number of buckets that we can store to the block
                }
                // at the start of the block we store an integer that contains the number of buckets that the block contains
                memcpy(block,&numOfPositions,sizeof(int));    // store the number of buckets that this certain block contains
                block+=sizeof(int);   // skip the address that we stored the integer
                int nextBucketBlock=-1;
                memcpy(block,&nextBucketBlock,sizeof(int));    // store the pointer to the next block of buckets (if it exists)
                block+=sizeof(int);   // skip the address that we stored the integer
                int temp[numOfPositions];
                for(int j=0; j<numOfPositions; j++) { // create and initialize the hashTable (or the part of the hashTable) that we will store at this block
                        temp[j]=-1;
                }
                memcpy(block,temp,sizeof(temp));    // copy the array to the block

                if (BF_WriteBlock(fd, newBlock) < 0) {   // write the block so the changes that we made will be saved
                        BF_PrintError("Error writing block back");
                        return -1;
                }
                prevBucketBlock=newBlock;
                // continue with the next block (if we need another one)
        }
        // at this point the HashTable is created, initialized and saved to the File

        printf("File %d has %d blocks\n", fd, BF_GetBlockCounter(fd));
        // So we have succesfully created and initialized the Hash File
        if (BF_CloseFile(fd)<0) {   // so close the file
                BF_PrintError("Error closing file");
                return -1;
        }
        // and finally return
        return 0;


}


HT_info* HT_OpenIndex( char *fileName){
  // open the Hash File with the given name, read the first block that contains the file header (HT_info) and return it
        int fd;
        void* block;

        if ((fd = BF_OpenFile(fileName)) < 0) {   // open the file
                BF_PrintError("Error opening file");
                return NULL;
        }


        if (BF_ReadBlock(fd, 0, &block) < 0) {    // read the first block
                BF_PrintError("Error getting block");
                return NULL;
        }

        if (((HT_info*) block)->isHeap) {   // check if the file is actually a Hash file
                return NULL;  // If it is not a Hash File return NULL so as to indicate the error
        }


        ((HT_info*) block)->fileDesc=fd;

        //create and return a copy of HT_info to avoid the address conflict in the following operations
        HT_info * test=malloc(sizeof(HT_info));
        memcpy(test,(HT_info*) block,sizeof(HT_info));

        return test;
}

int HT_CloseIndex( HT_info* header_info ) {
// close the Hash File with the given name
        int fd=header_info->fileDesc;   // get the file id from the HT_info

        free(header_info);  // free the copy of the HT_info that was allocated

        if (BF_CloseFile(fd) < 0) {   // close the file
                BF_PrintError("Error closing file");
                return -1;
        }
        printf("*** Closed file %d succesfully\n",fd);

        return 0;

}

// simple one
// int hash(int id,int buckets){
//         return id%buckets;
// }

unsigned int hash(char id[],int buckets){ //find the hash table index for the given id
  unsigned long int hashcode = 0;
  int a=33;
  char *tempkey=id;
  for (; *tempkey!='\0'; tempkey++)   // for every char/number of the id
    hashcode=(a*hashcode + *tempkey) % buckets;
  return hashcode;    //hash code is being %hashTableSize so it is 0 <= hashcode <= number of buckets
}


int HT_InsertEntry( HT_info header_info, Record record) {
        void *block;
        int bucketsCount;
        int fd=header_info.fileDesc;  // get the file descriptor from the HT_info

        if(HT_GetAllEntries(header_info,&(record.id))!=-1) {    // so call HT_GetAllEntries so as to check if it exists
          printf("A record with the same ID: %d ALREADY EXISTS\n",record.id);
          return -1; // then return failure without insert this record
        }

        int index;    // here will be stored the index of the hashTable that the new record should be inserted
        if(header_info.keyType=='i'){   // check if the key is int type
          char convertToInt[20];    // convert the integer to string so as to pass it as parameter to the hash function
          sprintf(convertToInt,"%d",record.id);
          index=hash(convertToInt,header_info.buckets); // call the hash function so as to return the index that the new record will be stored
        }
        // else if(header_info.keyType=='c'){   // in case the key is char type (isn't used in project1)
        //   index=hash((char *)value,header_info.buckets);
        // }
        if (BF_ReadBlock(fd,header_info.headerBlock, &block) < 0) {   // read the header block in order to find from the pointer in which block(s) the hash table is stored
                BF_PrintError("Error getting block");
                return -1;
        }
        block+=sizeof(HT_info);   // skip the HT_info
        int count=*((int *)block);  // from the header block, get the pointer to the first block that contains the buckets
        int newIndex=index; // stores the index that we want find, every time we skip a block it is incremented by the number of the buckets that this block had
        int blockOfBucket;
        int recordCount;
        // before we start inserting we need to check if the record already exists in the hashTable
        // if HT_GetAllEntries returns -1, that means that the record does not exist so we should insert it

        // we need to find the block that the index is stored (as the hashTable may be stored in multiple blocks)
        while(1) {
                if(count<0){
                  return -1;    //error while searching the bucket
                }
                if (BF_ReadBlock(fd,count, &block) < 0) {// read the block (starting from the second block of the file, in which the start of the hashTable is stored)
                        BF_PrintError("Error getting block");
                        return -1;
                }
                bucketsCount= *((int *)block);  // get how many buckets does this block have
                block+=sizeof(int);   // skip the address of the integer that we just read
                int nextBucketBlock=*((int *)block);  //get the next block that contains the buckets of the hash table (if it exists)
                block+=sizeof(int);   // skip the address of the integer that we just read

                if(newIndex<bucketsCount) { // if the index we want is in this block
                        // find the certain index we want.
                        if(*((int *)block+newIndex)==-1) {  // If it is empty (has no other record in this index)
                                if (BF_AllocateBlock(fd) < 0) { // allocate a new block
                                        BF_PrintError("Error allocating block");
                                        return -1;
                                }

                                if (BF_ReadBlock(fd,count, &block) < 0) {   // read again the block of the hashTable because a new block was allocate so it may have changed address
                                        BF_PrintError("Error getting block");
                                        return -1;
                                }
                                block+=sizeof(int); // skip the count of the buckets that this block contains
                                block+=sizeof(int); // skip the pointer to the next block
                                // find the index that the new record will be inserted
                                // and assign the "pointer" to the block that was just created and in wich the new record will be stored
                                *((int *)block+newIndex)=BF_GetBlockCounter(fd)-1;

                                blockOfBucket=*((int *)block+newIndex); // the number of the new block that the record will be instered

                                if (BF_WriteBlock(fd, count) < 0) {   // write the hashTable block so the pointer to the new block is saved
                                        BF_PrintError("Error writing block back");
                                        return -1;
                                }

                                if (BF_ReadBlock(fd,blockOfBucket, &block) < 0) {   // read the block that the new record will be inserted
                                        BF_PrintError("Error getting block");
                                        return -1;
                                }
                                recordCount=1;
                                memcpy(block,&recordCount,sizeof(int));   // store a counter that holds how many records exist in this block, at this point it is initialized with 1
                                block+=sizeof(int);   // skip the integer we just written
                                int nextBlock=-1;   // store a pointer for the next block (in case of overflow blocks), at this point is is initialized with -1 as we have no overflow block
                                memcpy(block,&nextBlock,sizeof(int));   // write the pointer
                                block+=sizeof(int);   // skip the address
                                memcpy(block,&record,sizeof(Record));   // and after the two integers, store the new record

                                if (BF_WriteBlock(fd, blockOfBucket) < 0) {   // finally write the block so as the changes are saved
                                        BF_PrintError("Error writing block back");
                                        return -1;
                                }
                                // printf("Inserted record with id %d\n"record.id);
                                return blockOfBucket;   // and return the block number that the record was inserted

                        }else{    // in case that a record already exists in the same index
                                  // that means that in the index of the hashTable exists a pointer to a block that contains record(s)
                                blockOfBucket=*((int *)block+newIndex);   // so get the number of the block that this index of the hashTable is pointing to
                                break;    // break the loop
                        }
                }
                // in case that the index we want is not in this block
                newIndex-=bucketsCount; // decrease the index that we want with the number of the buckets that this block has
                count=nextBucketBlock;    // go to the next block that contains the rest of the hashTable, that we took from the ponter of the current block to the next block
        }

///////////////////////////////////////////////////////////////////////////////////////////
        // if we reach here that means that at least one record already exists in the same index of the hashTable
        // so we need to find a "place" inside a block so as to insert the new record
        while(1) {    // the while loop exists as we may have overflow blocks
                if (BF_ReadBlock(fd,blockOfBucket, &block) < 0) {   // read the block (in the first loop the block that we read is the one that the hashTable's index we want points to)
                        BF_PrintError("Error getting block");
                        return -1;
                }

                int recordCount= *((int *)block);   // get how many records the block contains (from the start of the block that a counter is always stored)
                void *tempCount=block;

                block+=sizeof(int);   // skip the count of the records
                int nextBlock= *((int *)block);   // get the pointer to the next overflow block (that is always stored at the start of the block after the record count)

                if( BLOCK_SIZE - 2*sizeof(int) - sizeof(Record)*recordCount < sizeof(Record) ) {  // if there is no space for the record to be saved in this block
                        if(nextBlock!=-1) {   // we are not in the last block of the chain (of overflow blocks)
                                blockOfBucket=nextBlock;  // go to the next block of the overflow chain
                                continue; // continue with the next loop for the next block
                        }
                        // if we reached here that means that we are at the last block of the overflow chain an there is no space for the record to be stored
                        // so we need to create a new block to store the new record and attach it at the and of the overflow chain
                        if (BF_AllocateBlock(fd) < 0) {   // allocate the new block
                                BF_PrintError("Error allocating block");
                                return -1;
                        }
///////////////////////////////////////////////////////////////////////////////////////////
                        if (BF_ReadBlock(fd,blockOfBucket, &block) < 0) {   // read again the last block of the chain as we allocated a new block and address may have changed
                                BF_PrintError("Error getting block");
                                return -1;
                        }

                        block+=sizeof(int);
                        int newBlockNum=BF_GetBlockCounter(fd)-1;
                        memcpy(block,&newBlockNum,sizeof(int));   // save a pointer to the new block that was created

                        if (BF_WriteBlock(fd, blockOfBucket) < 0) {   // write the block so as the pointer to the new last block is saved
                                BF_PrintError("Error writing block back");
                                return -1;
                        }
///////////////////////////////////////////////////////////////////////////////////////////
                        if (BF_ReadBlock(fd,newBlockNum, &block) < 0) {   // read the new block that we allocated and the new record will be stored in
                                BF_PrintError("Error getting block");
                                return -1;
                        }
                        int recordCount=1;    // initialize the record count to 1
                        memcpy(block,&recordCount,sizeof(int));   // and copy the recordCount at the start of the block
                        block+=sizeof(int);
                        int nextBlock=-1;
                        memcpy(block,&nextBlock,sizeof(int));   // store a pointer to the next block of the chain, currently initialized to -1 as there is no other block
                        block+=sizeof(int);
                        memcpy(block,&record,sizeof(Record));     // copy the record (after the 2 integers)

                        if (BF_WriteBlock(fd, newBlockNum) < 0) {   // write the block so as the changes to the block are saved
                                BF_PrintError("Error writing block back");
                                return -1;
                        }
                        // printf("Inserted record with id %d\n"record.id);
                        return newBlockNum;   // return the number of the block that the new record was inserted

                }
                // in this case the record can be stored in an already created block
                // this block is already read above the if statement
                // also the number of the records that this block contains is also read and stored in recordCount variable
                block+=sizeof(int);   // skip the next block pointer
                block+=(sizeof(Record)*recordCount);  // skip the already saved records
                memcpy(block,&record,sizeof(Record));   // copy the record after the already saved ones
                recordCount++;      // increase the record count of the block as we have just inserted a new record to this block
                memcpy(tempCount,&recordCount,sizeof(int));   // copy the new recordCount value to the block
                if (BF_WriteBlock(fd, blockOfBucket) < 0) {   // finally write the block so as the changes are saved
                        BF_PrintError("Error writing block back");
                        return -1;
                }
                // printf("Inserted record with id %d\n"record.id);
                return blockOfBucket; // return the number of the block that the new record was inserted


        }
}

int HT_GetAllEntries( HT_info header_info, void *value){

        void *block;
        int bucketsCount;
        int fd=header_info.fileDesc;  // get the file descriptor from the HT_info
        int index;    // here will be stored the index of the hashTable that the record we want is in
        if(header_info.keyType=='i'){   // check if the key is int type
          char convertToInt[20];    // convert the integer to string so as to pass it as parameter to the hash function
          sprintf(convertToInt,"%d",*((int*)value));
          index=hash(convertToInt,header_info.buckets); // call the hash function so as to return the index that record we want is in
        }
        // else if(header_info.keyType=='c'){   // in case the key is char type (isn't used in project1)
        //   index=hash((char *)value,header_info.buckets);
        // }
        ///
        /// new ******
        if (BF_ReadBlock(fd,header_info.headerBlock, &block) < 0) {  // read the header block in order to find from the pointer in which block(s) the hash table is stored
                BF_PrintError("Error getting block");
                return -1;
        }
        block+=sizeof(HT_info);   // skip the HT_info
        int count=*((int *)block);  // from the header block, get the pointer to the first block that contains the buckets
        int newIndex=index; // stores the index that we want find, every time we skip a block it is incremented by the number of the buckets that this block had
        int blockOfBucket;
        // we need to find the block that the index is stored (as the hashTable may be stored in multiple blocks)
        while(1) {
                if(count<0){
                  return -1;  //error while searching the bucket
                }
                if (BF_ReadBlock(fd,count, &block) < 0) { // read the block (starting from the second block of the file, in which the start of the hashTable is stored)
                        BF_PrintError("Error getting block");
                        return -1;
                }
                bucketsCount= *((int *)block);  // get how many buckets does this block have
                block+=sizeof(int);   // skip the address of the integer that we just read
                int nextBucketBlock=*((int *)block);  //get the next block that contains the buckets of the hash table (if it exists)
                block+=sizeof(int);   // skip the address of the integer that we just read
                if(newIndex<bucketsCount) {   // if the index we want is in this block
                        if(*((int *)block+newIndex)==-1) {  // if this index does not point to a block
                                // then there is no record in this index, so surely the record we want to find does not exist in tha HashTable
                                return -1;
                        }else{    // if this bucket points to a block
                                blockOfBucket=*((int *)block+newIndex);   // get the number of the block that the bucket points to
                                break;    // break the loop so we can start searchig in this block
                        }
                }
                // in case that the index we want is not in this block
                newIndex-=bucketsCount; // decrease the index that we want with the number of the buckets that this block has
                count=nextBucketBlock;    // go to the next block that contains the rest of the hashTable, that we took from the ponter of the current block to the next block
        }
        int blocksReaded=1;   // counts how many blocks have been read until we find the record we are searching
        //  if we reach this point that means that there are record in the index we want, so we should start searching
        while(1) {    // the while loop exists as we may have overflow blocks
                if (BF_ReadBlock(fd,blockOfBucket, &block) < 0) {  // read the block (in the first loop the block that we read is the one that the hashTable's index we  want points to)
                        BF_PrintError("Error getting block");
                        return -1;
                }

                int recordCount= *((int *)block); // get how many records the block contains (from the start of the block that a counter is always stored)

                block+=sizeof(int); // skip the count of the records
                int nextBlock= *((int *)block);   // get the pointer to the next overflow block (that is always stored at the start of the block after the record count)
                block+=sizeof(int); // skip the pointer as we already read it

                for(int i=1; i<=recordCount; i++) {   // for every record that exists in this block that we have read
                        Record *rec=(Record *)block;    // get the record
                        if(header_info.keyType=='c') {    // if the key is string type (not used in project1)
                                // if(strcmp(rec->id,(char *)value)==0){
                                // printf("Found record with id: %s , name: %s , surname: %s, address: %s\n",rec->id,rec->name,rec->surname,rec->address);
                                // }
                        }else if(header_info.keyType=='i') {  // if the key is int type
                                if(rec->id== *((int *)value)) {   // compare the key of the certain record of this block with the key of the record we are searching
                                        // if they are the same, we have found the record we want
                                        printf("Found record with id: %d , name: %s , surname: %s, address: %s\n",rec->id,rec->name,rec->surname,rec->address);
                                        return blocksReaded;    // after printing, return how many blocks we have read until we found the record
                                }

                        }
                        // if we didn't find the record, continue with the next record of this block
                        block+=sizeof(Record);
                }
                // check the pointer that points to the next block of the overflow chain
                if(nextBlock!=-1) {  // if there is another block in the chain
                        blockOfBucket=nextBlock;    // assign the pointer to that block to the next block that we are going to read
                        // so at the next while loop, the next block of the overflow chain will be read
                        blocksReaded++;   // increase the number of blocks readed until finding the record
                }
                else{   // if there is no other block in the overflow chain
                  // that mean we have checked all the records of the hashTable index (all the record of the overflow chain)
                        break; // so the record does not exist, break the loop so as to return
                }
        }
        // if we reached this point that means that we checked all the records belonging to the index that the record we are searching should be in
        // so that mean that the record does not exist in the hashTable
        return -1;    // the record was not found



}



int HT_DeleteEntry( HT_info header_info,void *value ) {

        void *block;
        int bucketsCount;
        int fd=header_info.fileDesc;  // get the file descriptor from the HT_info
        int index;  // here will be stored the index of the hashTable that the record we want is in
        if(header_info.keyType=='i'){   // check if the key is int type
          char convertToInt[20];    // convert the integer to string so as to pass it as parameter to the hash function
          sprintf(convertToInt,"%d",*((int*)value));
          index=hash(convertToInt,header_info.buckets);   // call the hash function so as to return the index that record we want is in
        }
        // else if(header_info.keyType=='c'){   // in case the key is char type (isn't used in project1)
        //   index=hash((char *)value,header_info.buckets);
        // }
        /// new ******
        if (BF_ReadBlock(fd,header_info.headerBlock, &block) < 0) {  // read the header block in order to find from the pointer in which block(s) the hash table is stored
                BF_PrintError("Error getting block");
                return -1;
        }
        block+=sizeof(HT_info);   // skip HT_info
        int count=*((int *)block);  // from the header block, get the first block that contains the buckets
        int newIndex=index; // stores the index that we want find, every time we skip a block it is incremented by the number of the buckets that this block had
        int blockOfBucket;


        // we need to find the block that the index is stored (as the hashTable may be stored in multiple blocks)
        while(1) {
                if(count<0){
                  return -1;    //error while searching the bucket
                }
                if (BF_ReadBlock(fd,count, &block) < 0) {   // read the block (starting from the second block of the file, in which the start of the hashTable is stored)
                        BF_PrintError("Error getting block");
                        return -1;
                }
                bucketsCount= *((int *)block);  // get how many buckets does this block have
                block+=sizeof(int);   // skip the address of the integer that we just read
                int nextBucketBlock=*((int *)block);  //get the next block that contains the buckets of the hash table (if it exists)
                block+=sizeof(int);   // skip the address of the integer that we just read
                if(newIndex<bucketsCount) {    // if the index we want is in this block
                        if(*((int *)block+newIndex)==-1) {  // if this index does not point to a block
                          // then there is no record in this index, so surely the record we want to find does not exist in tha HashTable
                                return -1;
                        }else{    // if this bucket points to a block
                                blockOfBucket=*((int *)block+newIndex);    // get the number of the block that the bucket points to
                                break;    // break the loop so we can start searchig in this block
                        }
                }
                // in case that the index we want is not in this block
                newIndex-=bucketsCount; // decrease the index that we want with the number of the buckets that this block has
                count=nextBucketBlock;    // go to the next block that contains the rest of the hashTable, that we took from the ponter of the current block to the next block
        }
        int blocksReaded=1; // counts how many blocks have been read until we find the record we are searching
        //  if we reach this point that means that there are record in the index we want, so we should start searching
        while(1) {    // the while loop exists as we may have overflow blocks
                if (BF_ReadBlock(fd,blockOfBucket, &block) < 0) {   // read the block (in the first loop the block that we read is the one that the hashTable's index we  want points to)
                        BF_PrintError("Error getting block");
                        return -1;
                }

                int recordCount= *((int *)block); // get how many records the block contains (from the start of the block that a counter is always stored)
                void *tempCount=block;    // keep the address that the recordCount is stored
                Record *hold;
                int found=NO;

                block+=sizeof(int);   // skip the count of the records
                int nextBlock= *((int *)block); // get the pointer to the next overflow block (that is always stored at the start of the block after the record count)
                block+=sizeof(int);   // skip the pointer as we already read it

                for(int i=1; i<=recordCount; i++) {   // for every record that exists in this block that we have read
                        Record *rec=(Record *)block;    // get the record
                        if(header_info.keyType=='c'){   // if the key is string type (not used in project1)
                          // if(strcmp(rec->id,(char *)value)==0){
                          //         printf("DELETING record with id: %s , name: %s , surname: %s, address: %s\n",rec->id,rec->name,rec->surname,rec->address);
                          //         hold=(Record *)block;
                          //         found=YES;
                          // }
                        }
                        else if(header_info.keyType=='i') {   // if the key is int type
                                if(rec->id== *((int *)value)) {   // compare the key of the certain record of this block with the key of the record we are searching
                                        // if they are the same, we have found the record we want, so we must delete it
                                        printf("DELETING record with id: %d , name: %s , surname: %s, address: %s\n",rec->id,rec->name,rec->surname,rec->address);
                                        hold=(Record *)block;   // store the address that the record we want to delete is stored
                                        found=YES;    // mark the flag as yes because we found the record we want to delete
                                        // now we must continue looping until the last record of this block, so as to copy the last record in the place of the record that we will delete
                                }

                        }
                        block+=sizeof(Record);
                }
                // at this point we have read all the records of the block
                if(found==YES) {    // if the record we want to delete was one of them
                        // hold hold variable contains the address of the record we want to delete
                        // block-sizeof(Record) point to the last record of the block
                        recordCount--;    // so at fist decrease the recordCount as we are about to delete a record from this block
                        memcpy(tempCount,&recordCount,sizeof(int));   // copy the recordCount
                        // in order to delete the record, we copy the last record of the block in the address of the record we want to delete
                        // in this way we achieved both: deleting the record we want, and also move the last record of the block to the "empty spot" that was created after deleting the record
                        memcpy(hold,block-sizeof(Record),sizeof(Record));

                        if (BF_WriteBlock(fd, blockOfBucket) < 0) {   // write the block so the changes to the block are saved
                                BF_PrintError("Error writing block back");
                                return -1;
                        }
                        return 0;
                }
                // if we didn't find the record, we should continue with the next block of the overflow chain
                if(nextBlock!=-1) {   // if there is a next block
                        blockOfBucket=nextBlock;   // assign the pointer to that block to the next block that we are going to read
                        // so at the next while loop, the next block of the overflow chain will be read
                        blocksReaded++;   // increase the number of blocks readed until finding the record
                }
                else{// if there is no other block in the overflow chain
                  // that mean we have checked all the records of the hashTable index (all the record of the overflow chain)
                        break; // so the record does not exist, break the loop so as to return
                }
        }
        // if we reached this point that means that we checked all the records belonging to the index that the record we are searching should be in
        // so that mean that the record does not exist in the hashTable
        return -1;    // the record was not found
}

int HashStatistics( char* filename ) {
        HT_info *info=HT_OpenIndex(filename);

        int blocksCointainingHash=1;
        void *block;
        int bucketsCount;
        int fd=info->fileDesc;    // get the file descriptor from the HT_info
        int blockOfBucket;
        int bucketsSeen=0;
        int sumOfRecords=0;   // in sumOfRecords will be stored the total number of records that exist in the hashTable (used for finding the mean)
        int min=-1,max=-1;    // min will contain the minimum record that exist in a bucket, and max the maximum records in a bucket
        int overflowBuckets[info->buckets];   // for every bucket of the hashTable, store how many overflow blocks it has
        for(int i=0; i<info->buckets; i++) {
                overflowBuckets[i]=-1;    // initialize
        }
        int bucketNum=0;

        if (BF_ReadBlock(fd,0, &block) < 0) {    // read the header block in order to find from the pointer in which block(s) the hash table is stored
                BF_PrintError("Error getting block");
                return -1;
        }
        block+=sizeof(HT_info);   // skip the HT_info
        int count=*((int *)block);  // from the header block, get the first block that contains the buckets


        while(bucketsSeen<info->buckets) {    // start traversing the hashTable, while loop exists as the hashTable may be stored in multiple blocks
                if(count<0){
                  return -1;    //error while searching the bucket
                }
                if (BF_ReadBlock(fd,count, &block) < 0) {   // read a block that contains the hashTable (or a part of it)
                        BF_PrintError("Error getting block1");
                        return -1;
                }
                ////
                ////
                /// block ----> buckets
                bucketsCount= *((int *)block);  // get how many buckets does this block have
                block+=sizeof(int);   // skip the address of the integer that we just read
                int nextBucketBlock=*((int *)block);  //get the next block that contains the buckets of the hash table (if it exists)
                block+=sizeof(int);   // skip the address of the integer that we just read

                for(int i=0; i<bucketsCount; i++) { // for every bucket that exists in this block
                        if (BF_ReadBlock(fd,count, &block) < 0) {   // read again the block that contains the hashTable as it's address may have changed
                                BF_PrintError("Error getting block1");
                                return -1;
                        }
                        block+=sizeof(int);   // skip the bucketsCounter
                        block+=sizeof(int);   // skip the nextBlock pointer
                        blockOfBucket=*((int *)block+i);    // get the pointer to the block that this index of the hash table points to
                        int bucketSum=0;
                        while(1) {    // start traversing to the blocks that are correspondig to the index of the hashTable we are currently searchig
                                void *block2;
                                if(blockOfBucket==-1) {   // if we reached at the end of the overflow chain
                                        break;
                                }
                                if (BF_ReadBlock(fd,blockOfBucket, &block2) < 0) {  // read the block
                                        BF_PrintError("Error getting block2");
                                        return -1;
                                }
                                int recordCount= *((int *)block2);    // get how many record the block contains
                                block2+=sizeof(int);
                                int nextBlock= *((int *)block2);    // get the pointer to the next block of the overflow chain
                                block2+=sizeof(int);
                                bucketSum+=recordCount;   // add the recordCount of this block to total number of records in this bucket of the hashTable
                                overflowBuckets[bucketNum]++;   // increase the number of block existing in this bucket of the hashTable
                                ////
                                ////
                                blockOfBucket=nextBlock;    // go to the next block of the overflow chain
                        }

                        sumOfRecords+=bucketSum;    // add the number of record that this bucket contains to the total records of the hashTable
                        if (min==-1 && max==-1) {   // if both min and max have no value
                                // initialize them with the sums of this bucket
                                min=bucketSum;
                                max=bucketSum;
                        }
                        if(max<bucketSum) {   // find the maximum number of records existing in a bucket
                                max=bucketSum;
                        }
                        if (min>bucketSum) {  // find the minimum number of records existing in a bucket
                                min=bucketSum;
                        }
                        bucketNum++; // increase the index as we will continue with the next bucket of the hashTable
                }

                // continue with the next block that contains buckets of the hashTable (if the hashTable is stored in multiple blocks)
                bucketsSeen+=bucketsCount;
                count=nextBucketBlock;    // go to the next block that contains the rest of the hashTable, that we took from the ponter of the current block to the next block
                blocksCointainingHash++;
        }
        // at this point we have computed every statistic we need in order to find the statistics that are requested

        printf("________ STATISTICS ________\n");
        // for a) the number of blocks that only contain records are printed.
        // from the total number of block, the first block that cointains the HT_info and the block that contain the hashTable are subtracted
        printf("a) Total Blocks of file: %d , Blocks that contain records:%d\n",BF_GetBlockCounter(info->fileDesc),BF_GetBlockCounter(info->fileDesc)-blocksCointainingHash);
        double mean= (double) sumOfRecords/ (double) info->buckets;
        // we have already found min and max number of record in a block. For the mean we take the total number of records contained in the hashTable divided by the number of buckets
        printf("b) Number of records in buckets: min :%d  mean: %lf  max: %d\n",min,mean,max);
        // for the mean blocks in each bucket we take the number of block (containing records) divided by the number of buckets that the hashTable has
        double meanBlocks= (double) (BF_GetBlockCounter(info->fileDesc)-count)/(double) info->buckets;
        printf("c) Average blocks in buckets: %lf\n",meanBlocks);
        printf("d)\n");
        // for d) we first print how many overflow blocks each bucket has (the first block of each bucket is not counted as it is not considered as an overflow)
        int overflowCount=0;  // counts how many buckets have overflow blocks
        for(int i=0; i<info->buckets; i++) {
                if(overflowBuckets[i]<1)    // if this buckets has no overflow blocks, continue with the next one
                        continue;
                printf("Bucket %d has %d overflowed block(s)\n",i,overflowBuckets[i]);
                overflowCount++;
        }
        printf("*** Total overflowed buckets: %d\n",overflowCount);


        HT_CloseIndex(info);
        return 0;
}
