#include "StaticBuffer.h"
#include<bits/stdc++.h>


unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
unsigned char StaticBuffer::blockAllocMap[DISK_BLOCKS];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];

StaticBuffer::StaticBuffer() {

  for (int bufferIndex = 0;bufferIndex<BUFFER_CAPACITY;bufferIndex++) {
    metainfo[bufferIndex].free = true;
    metainfo[bufferIndex].dirty = false;
    metainfo[bufferIndex].timeStamp = -1;
    metainfo[bufferIndex].blockNum = -1;
  }
  
  //setting up block Allocation map
  for (int i = 0; i < BLOCK_ALLOCATION_MAP_SIZE; i++) {
		Disk::readBlock(blockAllocMap + i * BLOCK_SIZE, i);
	}
  
}

StaticBuffer::~StaticBuffer() {
   //copying bmap 
  for (int i = 0; i < BLOCK_ALLOCATION_MAP_SIZE; i++) {
		Disk::writeBlock(blockAllocMap + i * BLOCK_SIZE, i);
	}
  
   for (int bufferIndex = 0;bufferIndex<BUFFER_CAPACITY;bufferIndex++)
   {
     if(metainfo[bufferIndex].free == false&&metainfo[bufferIndex].dirty == true)
     {
       Disk::writeBlock(blocks[bufferIndex],metainfo[bufferIndex].blockNum);
     }
   }
}

int StaticBuffer::getFreeBuffer(int blockNum) {
	// Check if blockNum is valid (non zero and less than DISK_BLOCKS)
	// and return E_OUTOFBOUND if not valid.
	if (blockNum < 0 || blockNum >= DISK_BLOCKS) {
		return E_OUTOFBOUND;
	}

	// increase the timeStamp in metaInfo of all occupied buffers.
	for (int i = 0; i < BUFFER_CAPACITY; i++) {
		if (!metainfo[i].free) {
			metainfo[i].timeStamp++;
		}
	}

	// let bufferNum be used to store the buffer number of the free/freed buffer.
	int bufferNum = 0, isFree = 0;

	// iterate through metainfo and check if there is any buffer free

	// if a free buffer is available, set bufferNum = index of that free buffer.
	for (int i = 0; i < BUFFER_CAPACITY; i++) {
		if (metainfo[i].free) {
			bufferNum = i;
			isFree = 1;
			break;
		}
	}

	// if a free buffer is not available,
	//     find the buffer with the largest timestamp
	//     IF IT IS DIRTY, write back to the disk using Disk::writeBlock()
	//     set bufferNum = index of this buffer
	if (!isFree) {
		for (int i = 0; i < BUFFER_CAPACITY; i++) {
			if (metainfo[i].timeStamp > metainfo[bufferNum].timeStamp) {
				bufferNum = i;
			}
		}
		if (metainfo[bufferNum].dirty) {
			Disk::writeBlock(blocks[bufferNum], metainfo[bufferNum].blockNum);
		}
	}

	// update the metaInfo entry corresponding to bufferNum with
	// free:false, dirty:false, blockNum:the input block number, timeStamp:0.
	metainfo[bufferNum].free = false;
	metainfo[bufferNum].dirty = false;
	metainfo[bufferNum].timeStamp = 0;
	metainfo[bufferNum].blockNum = blockNum;
	// return the bufferNum.
	return bufferNum;
}


int StaticBuffer::getBufferNum(int blockNum) {
  if(blockNum<0||blockNum>DISK_BLOCKS) return E_OUTOFBOUND;
  int foundblock=-1;
  for (int bufferIndex = 0;bufferIndex<BUFFER_CAPACITY;bufferIndex++) {
    if(!metainfo[bufferIndex].free && metainfo[bufferIndex].blockNum == blockNum) 
     {
       foundblock=bufferIndex;
       break; 
     }
  }
  if(foundblock==-1)
     return E_BLOCKNOTINBUFFER;
  else
     return foundblock;
}

int StaticBuffer::setDirtyBit(int blockNum){
    
    int bufferNum=getBufferNum(blockNum);
    if(bufferNum == E_BLOCKNOTINBUFFER||bufferNum == E_OUTOFBOUND) return bufferNum;
    
    metainfo[bufferNum].dirty = true;
     return SUCCESS;
}

int StaticBuffer::getStaticBlockType(int blockNum){
    // Check if blockNum is valid (non zero and less than number of disk blocks)
    // and return E_OUTOFBOUND if not valid.
	if (blockNum >= DISK_BLOCKS || blockNum < 0) {
		return E_OUTOFBOUND;
	}
    // Access the entry in block allocation map corresponding to the blockNum argument
    // and return the block type after type casting to integer.
 	return (int) blockAllocMap[blockNum];
}
