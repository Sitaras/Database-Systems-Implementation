#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HP.h"
// int HP_CreateFile( char *fileName, char attrType, char* attrName, int attrLength,int buckets) // probably fault of report: "int buckets"
int HP_CreateFile( char *fileName, char attrType, char* attrName, int attrLength){
// create and initialize an empty heap file with name fileName
// the first block has only the file header (HT_info)

        HP_info info;
        int fd;
        int blkCnt;
        void* block;

        // key could be integer type or char type
        if(attrType!='c' && attrType!='i') {
                printf("Wrong key type\n");
                return -1;
        }


        // create a file with name fileName which consists of block
        if (BF_CreateFile(fileName) < 0) {
                BF_PrintError("Error creating file");
                exit(EXIT_FAILURE);
        }

        // open "block file"
        if ((fd = BF_OpenFile(fileName)) < 0) {
                BF_PrintError("Error opening file");
                return -1;
        }

        // allocate a new block to save the file header: HP_info
        if (BF_AllocateBlock(fd) < 0) {
                BF_PrintError("Error allocating block");
                return -1;
        }

        blkCnt = BF_GetBlockCounter(fd);
        printf("File %d has %d blocks\n", fd, blkCnt);
        int headerBlockNum=BF_GetBlockCounter(fd)-1;

        if (BF_ReadBlock(fd, headerBlockNum, &block) < 0) {
                BF_PrintError("Error getting block");
                return -1;
        }

        // initialize the HP_info with the given info
        info.keyType=attrType;
        info.keyLength=attrLength;
        strcpy(info.keyName,attrName);
        info.isHeap=YES;
        info.isHash=NO;
        info.headerBlock=headerBlockNum;
        // and save it at the first block
        memcpy(block,&info,sizeof(HP_info));

        block+=sizeof(HP_info);
        int nextblock=-1; // pointer for the next block
        memcpy(block,&nextblock,sizeof(int));

        // write the block to save the changes that we made to the block
        if (BF_WriteBlock(fd, headerBlockNum) < 0) {
                BF_PrintError("Error writing block back");
                return -1;
        }


        // So we have succesfully created and initialized the Heap File
        if (BF_CloseFile(fd)<0) {  // so close the file
                BF_PrintError("Error closing file");
                return -1;
        }
        // and finally return
        return 0;
}


HP_info* HP_OpenFile(char *fileName){
// open the heap file with name file name and read from the first block the file header and return it

        int fd;
        void* block;

        // open the file
        if ((fd = BF_OpenFile(fileName)) < 0) {
                BF_PrintError("Error opening file");
                return NULL;
        }

        // read the first block
        if (BF_ReadBlock(fd, 0, &block) < 0) {
                BF_PrintError("Error getting block");
                return NULL;
        }

        // check if the file is actually a Heap file
        if (((HP_info*) block)->isHash) {
                return NULL; // If it is not a Heap File return NULL so as to indicate the error
        }


        ((HP_info*) block)->fileDesc=fd;

        //create and return a copy of HP_info to avoid the address conflict in the following operations
        HP_info * test=malloc(sizeof(HP_info));
        memcpy(test,(HP_info*) block,sizeof(HP_info));

        return test;

}


int HP_CloseFile(HP_info* header_info){
// close the Heap file with the given name

        int fd=header_info->fileDesc; // get the file id from the HP_info

        free(header_info); // free the copy of the HP_info that was allocated

        if (BF_CloseFile(fd) < 0) { // close the file
                BF_PrintError("Error closing file");
                return -1;
        }
        printf("*** Closed file %d succesfully\n",fd);

        return 0;
}


int HP_InsertEntry( HP_info header_info,Record record) {
// insert the record in to the heap
// (every block of record has in the beginning a records counter and a pointer to the next block)

        int fd=header_info.fileDesc; // get the file id from the HP_info
        void* block;
        int recordCount;


        // at this case, only the first block exists which has the file header (HP_info)
        if (BF_GetBlockCounter(fd)==1) {

                // allocate a new block to save the record
                if (BF_AllocateBlock(fd) < 0) {
                        BF_PrintError("Error allocating block");
                        return -1;
                }

                int newblock=BF_GetBlockCounter(fd)-1;

                if (BF_ReadBlock(fd, header_info.headerBlock, &block) < 0) {
                        BF_PrintError("Error getting block");
                        return -1;
                }
                // update the pointer (of first block) to the next new block
                block+=sizeof(HP_info);
                int newBlockNum=newblock;
                memcpy(block,&newBlockNum,sizeof(int));

                // finally, write the block back at disc
                if (BF_WriteBlock(fd, 0) < 0) {
                        BF_PrintError("Error writing block back");
                        return -1;
                }

                if (BF_ReadBlock(fd, newblock, &block) < 0) {
                        BF_PrintError("Error getting block");
                        return -1;
                }
                // now, save the record at the new block
                recordCount=1;
                memcpy(block,&recordCount,sizeof(int));
                block+=sizeof(int);
                int nextBlock=-1;
                memcpy(block,&nextBlock,sizeof(int));
                block+=sizeof(int);
                memcpy(block,&record,sizeof(Record));

                // finally, write the new block back at disc
                if (BF_WriteBlock(fd,newblock) < 0) {
                        BF_PrintError("Error writing block back");
                        return -1;
                }
                return newblock;
        }
        else if(BF_GetBlockCounter(fd)>1) {

                // before we start inserting we need to check if the record already exists in the heap file
                if(HP_GetAllEntries(header_info,&(record.id))!=-1) {  // so call HT_GetAllEntries so as to check if it exists
                        printf("A record with the same ID: %d ALREADY EXISTS\n",record.id);
                        return -1;
                }

                // if HP_GetAllEntries returns -1, that means that the record does not exist so we should insert it

                // let's go to check block per block to see if it has space to insert the record,
                // we begin from the second one.
                if (BF_ReadBlock(fd, header_info.headerBlock, &block) < 0) {
                        BF_PrintError("Error getting block");
                        return -1;
                }
                block+=sizeof(HP_info);
                int nextblock=*((int *)block);
                int prevblock=nextblock;

                while(1) {
                        if(nextblock==-1) { // the existing blocks haven't space to insert the record (end of heap)
                                break;
                        }
                        if (BF_ReadBlock(fd, nextblock, &block) < 0) {
                                BF_PrintError("Error getting block");
                                return -1;
                        }
                        recordCount= *((int *)block);
                        int *tempCount=(int *)block;
                        block+=sizeof(int);
                        prevblock=nextblock;
                        nextblock=*((int *)block);
                        // check if the current block has free space to insert the record
                        if(BLOCK_SIZE - 2*sizeof(int) - sizeof(Record)*recordCount >= sizeof(Record) ) {
                                // insert record at the current block and increase the recordCount by 1
                                block+=sizeof(int);
                                block+=(sizeof(Record)*recordCount);
                                memcpy(block,&record,sizeof(Record));
                                recordCount++;
                                memcpy(tempCount,&recordCount,sizeof(int));

                                // finally, write the block back at disc
                                if (BF_WriteBlock(fd, prevblock) < 0) {
                                        BF_PrintError("Error writing block back");
                                        return -1;
                                }
                                printf("Inserted record with id %d\n",record.id);
                                // printf(">> record inserted in already CREATED BLOCK\n");

                                // return number of block that the record inserted
                                return prevblock;
                        }

                }
                // the existing blocks haven't space to insert the record, need to allocate a new one
                if (BF_AllocateBlock(fd) < 0) {
                        BF_PrintError("Error allocating block");
                        return -1;
                }
                int newblock=BF_GetBlockCounter(fd)-1;
                if (BF_ReadBlock(fd, prevblock, &block) < 0) {
                        BF_PrintError("Error getting block");
                        return -1;
                }
                // update the pointer (of previous block) to the next new block
                block+=sizeof(int);
                int newBlockNum=newblock;
                memcpy(block,&newBlockNum,sizeof(int));
                if (BF_WriteBlock(fd, prevblock) < 0) {
                        BF_PrintError("Error writing block back");
                        return -1;
                }

                if (BF_ReadBlock(fd, newblock, &block) < 0) {
                        BF_PrintError("Error getting block");
                        return -1;
                }

                // now, save the record at the new block
                recordCount=1;
                memcpy(block,&recordCount,sizeof(int));
                block+=sizeof(int);
                int nextBlock=-1;
                memcpy(block,&nextBlock,sizeof(int));
                block+=sizeof(int);
                memcpy(block,&record,sizeof(Record));

                // finally, write the new block back at disc
                if (BF_WriteBlock(fd, BF_GetBlockCounter(fd)-1) < 0) {
                        BF_PrintError("Error writing block back");
                        return -1;
                }
                // printf("Inserted record with id %d\n"record.id);
                // printf(">> NEW BLOCK created for the record\n");

                // return number of block that the new record was inserted
                return newblock;

        }
        return -1; // if insertion failed

}


int HP_GetAllEntries( HP_info header_info,void *value){
// search in the heap to find the record with the correspoding key which given as argument in function

        int fd=header_info.fileDesc;
        int i;
        void* block;
        int blkCnt = BF_GetBlockCounter(fd);

          // at this case, only the first block exists which has the file header (HP_info)
          // No records...
        if (blkCnt==1) {
                return -1;
        }
        else{
                // let's go to find the record
                // we begin the search from the second block.
                if (BF_ReadBlock(fd, header_info.headerBlock, &block) < 0) {
                        BF_PrintError("Error getting block");
                        return -1;
                }
                block+=sizeof(HP_info);
                int nextblock=*((int *)block);
                int count=1; // blocks counter

                while(1) {
                        if (nextblock==-1) { // the record doesn't exist (reached at the last block,end of heap)
                                break;
                        }


                        if (BF_ReadBlock(fd, nextblock, &block) < 0) {
                                BF_PrintError("Error getting block");
                                return -1;
                        }


                        int recordCount= *((int *)block);
                        block+=sizeof(int);

                        nextblock=*((int *)block);
                        block+=sizeof(int);
                        // for every block check all the records to find the required one
                        for(i=1; i<=recordCount; i++) {
                                Record *rec=(Record *)block;
                                if(header_info.keyType=='c') {
                                        // if(strcmp(rec->id,(char *)value)==0){
                                        // printf("Found record with id: %s , name: %s , surname: %s, address: %s\n",rec->id,rec->name,rec->surname,rec->address);
                                        // }
                                }else if(header_info.keyType=='i') {
                                        if(rec->id== *((int *)value)) {
                                                // yes, record found
                                                printf("Found record with id: %d , name: %s , surname: %s, address: %s\n",rec->id,rec->name,rec->surname,rec->address);
                                                // then return the number of block where the record found
                                                return count;
                                        }

                                }
                                block+=sizeof(Record);
                        }
                        count++;

                }
                return -1; // failed to find the record

        }
}


int HP_DeleteEntry( HP_info header_info, void *value) {
// search in the heap to find and delete the record with the correspoding key which given as argument in function

        int fd=header_info.fileDesc;
        int i;
        void* block;
        int blkCnt = BF_GetBlockCounter(fd);

        // at this case, only the first block exists which has the file header (HP_info)
        // No records...
        if (blkCnt==1) {
                return -1;
        }
        else{
                // let's go to find the record
                // we begin the search from the second block.
                if (BF_ReadBlock(fd, header_info.headerBlock, &block) < 0) {
                        BF_PrintError("Error getting block");
                        return -1;
                }

                block+=sizeof(HP_info);
                int nextblock=*((int *)block);
                int prevblock=nextblock;

                while (1) {
                        if (nextblock==-1){ // the record doesn't exist (reached at the last block,end of heap)
                          return -1;
                        }

                        if (BF_ReadBlock(fd, nextblock, &block) < 0) {
                                BF_PrintError("Error getting block");
                                return -1;
                        }

                        int found=NO;
                        int recordCount= *((int *)block);
                        int *holdCount=(int *)block; // a pointer to recordCount variable of current block
                        Record *hold;
                        block+=sizeof(int);
                        prevblock=nextblock;
                        nextblock=*((int *)block);
                        block+=sizeof(int);

                        // for every block check all the records to find the required one
                        for(i=1; i<=recordCount; i++) {
                                Record *rec=(Record *)block;

                                if(header_info.keyType=='c') {
                                        // if(strcmp(rec->id,(char *)value)==0){
                                        // printf("Found record with id: %s , name: %s , surname: %s, address: %s\n",rec->id,rec->name,rec->surname,rec->address);
                                        // }
                                }
                                else if(header_info.keyType=='i') {
                                        if(rec->id== *((int *)value)) {
                                                // yes, record found
                                                printf("Deleting record with id: %d , name: %s , surname: %s, address: %s\n",rec->id,rec->name,rec->surname,rec->address);
                                                hold=(Record *)block; // save the position of the record inside the block
                                                found=YES;
                                        }

                                }
                                block+=sizeof(Record);
                        }
                        if(found==YES) {
                                // ~ Deleting the record ~
                                // update the recordCount and shift the last record to the deleted position
                                /* If the selected record to delete is the only one or is the last at the block, just reduce by 1 the recordCount.
                                   So, at the future insert of record at this block the "deleted record" will be overwriten.  */
                                recordCount--;
                                memcpy(holdCount,&recordCount,sizeof(int));
                                memcpy(hold,block-sizeof(Record),sizeof(Record));

                                // finally, write block back at disc
                                if (BF_WriteBlock(fd, prevblock) < 0) {
                                        BF_PrintError("Error writing block back");
                                        return -1;
                                }
                                return 0; // record deleted succesfully
                        }
                }
                return -1; // failed to find the record

        }
}
