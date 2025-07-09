#include "BlockBuffer.h"
#include <cstdlib>
#include <cstring>


RecBuffer::RecBuffer() : BlockBuffer::BlockBuffer('R'){}

BlockBuffer::BlockBuffer(char blockType){
     int ret;
        // set the blockNum field of the object to that of the allocated block
       // number if the method returned a valid block number,
       // otherwise set the error code returned as the block number
	if (blockType == 'R') {
		ret = getFreeBlock(REC);
	}
	else if (blockType == 'L') {
		ret = getFreeBlock(IND_LEAF);
	}
	else if (blockType == 'I') {
		ret = getFreeBlock(IND_INTERNAL);
	}
       this->blockNum=ret;    

    // (The caller must check if the constructor allocatted block successfully
    // by checking the value of block number field.)
}

RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}

BlockBuffer::BlockBuffer(int blockNum) {
  this->blockNum=blockNum;
}
int BlockBuffer::getBlockNum(){
       return this->blockNum;
}
/*int BlockBuffer::getHeader(struct HeadInfo *head) {
  unsigned char buffer[BLOCK_SIZE];

  Disk::readBlock(buffer, this->blockNum);

  memcpy(&head->numSlots, buffer + 24, 4);
  memcpy(&head->numEntries, buffer+16, 4);
  memcpy(&head->numAttrs, buffer+20, 4);
  memcpy(&head->rblock, buffer+12, 4);
  memcpy(&head->lblock, buffer+8, 4);

  return SUCCESS;
}*/

int BlockBuffer::getHeader(struct HeadInfo *head) {

  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;   
  }

  memcpy(&head->numSlots, bufferPtr + 24, 4);
  memcpy(&head->numEntries, bufferPtr+16, 4);
  memcpy(&head->numAttrs, bufferPtr+20, 4);
  memcpy(&head->rblock, bufferPtr+12, 4);
  memcpy(&head->lblock, bufferPtr+8, 4);
  memcpy(&head->pblock, bufferPtr + 4, 4);

  return SUCCESS;
}

/*int RecBuffer::getRecord(union Attribute *rec, int slotNum) {
  struct HeadInfo head;

  this->getHeader(&head);

  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;

  unsigned char buffer[BLOCK_SIZE];
  Disk::readBlock(buffer, this->blockNum);
  

  int recordSize = attrCount * ATTR_SIZE;
  unsigned char *slotPointer = buffer+HEADER_SIZE+slotCount+(recordSize*slotNum);

  memcpy(rec, slotPointer, recordSize);

  return SUCCESS;
}*/

int RecBuffer::getRecord(union Attribute *rec, int slotNum) {
  struct HeadInfo head;
  unsigned char *bufferPtr;
  this->getHeader(&head);
  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;

  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }
  int recordSize = attrCount * ATTR_SIZE;
  unsigned char *slotPointer = bufferPtr+HEADER_SIZE+slotCount+(recordSize*slotNum);

  memcpy(rec, slotPointer, recordSize);
  return SUCCESS;
}

int BlockBuffer::setHeader(struct HeadInfo* head) {

	unsigned char* bufferPtr;
	// get the starting address of the buffer containing the block using
	// loadBlockAndGetBufferPtr(&bufferPtr).
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);

	// if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
		// return the value returned by the call.
	if (ret != SUCCESS) {
		return ret;
	}

	// cast bufferPtr to type HeadInfo*
	struct HeadInfo* bufferHeader = (struct HeadInfo*)bufferPtr;

	// copy the fields of the HeadInfo pointed to by head (except reserved) to
	// the header of the block (pointed to by bufferHeader)
	//(hint: bufferHeader->numSlots = head->numSlots )
	// bufferHeader->blockType = head->blockType;
	bufferHeader->pblock = head->pblock;
	bufferHeader->lblock = head->lblock;
	bufferHeader->rblock = head->rblock;
	bufferHeader->numAttrs = head->numAttrs;
	bufferHeader->numEntries = head->numEntries;
	bufferHeader->numSlots = head->numSlots;

	// update dirty bit by calling StaticBuffer::setDirtyBit()
	// if setDirtyBit() failed, return the error code
	return StaticBuffer::setDirtyBit(this->blockNum);
}


int RecBuffer::setRecord(union Attribute *rec, int slotNum) {
    unsigned char *bufferPtr;
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);
    if (ret != SUCCESS) {
    return ret;
    }

    struct HeadInfo head;
    this->getHeader(&head);
    int attrCount = head.numAttrs;
    int slotCount = head.numSlots;
   
    if(slotNum>=slotCount) return E_OUTOFBOUND;

    int recordSize = attrCount * ATTR_SIZE;
    unsigned char *slotPointer = bufferPtr+HEADER_SIZE+slotCount+(recordSize*slotNum);

    memcpy(slotPointer, rec, recordSize);
    
    return StaticBuffer::setDirtyBit(blockNum);
}

int BlockBuffer::setBlockType(int blockType){

    unsigned char *bufferPtr;
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);
    if (ret != SUCCESS) {
       return ret;
    }
    
    // store the input block type in the first 4 bytes of the buffer.
    // (hint: cast bufferPtr to int32_t* and then assign it)
    // *((int32_t *)bufferPtr) = blockType;
    *((int32_t *)bufferPtr) = blockType;
    
    StaticBuffer::blockAllocMap[this->blockNum]=blockType;
    // update the StaticBuffer::blockAllocMap entry corresponding to the
    // object's block number to `blockType`.
    
    return StaticBuffer::setDirtyBit(this->blockNum);
}

int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr) {
  int bufferNum = StaticBuffer::getBufferNum(this->blockNum);
    if (bufferNum == E_OUTOFBOUND) {
      return E_OUTOFBOUND;
    }
    
    if (bufferNum != E_BLOCKNOTINBUFFER)
    {
      StaticBuffer::metainfo[bufferNum].timeStamp = 0;
      for (int bufferIndex = 0;bufferIndex<BUFFER_CAPACITY;bufferIndex++)
      {
         if(StaticBuffer::metainfo[bufferIndex].free==false)
              StaticBuffer::metainfo[bufferIndex].timeStamp++;
      }
       
    }
    else
    {
       bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);
       Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
    }
  *buffPtr = StaticBuffer::blocks[bufferNum];

  return SUCCESS;
}

/* used to get the slotmap from a record block
NOTE: this function expects the caller to allocate memory for `*slotMap`
*/
int RecBuffer::getSlotMap(unsigned char *slotMap) {
  unsigned char *bufferPtr;

  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }

  RecBuffer blockBuffer(this->blockNum);
  struct HeadInfo head;
  blockBuffer.getHeader(&head);

  int slotCount = head.numSlots;
  unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;
  
  memcpy(slotMap,slotMapInBuffer,slotCount);
  return SUCCESS;
}

int BlockBuffer::getFreeBlock(int blockType){
    int i;
    for( i=0;i<DISK_BLOCKS;i++)
    {
      if(StaticBuffer::blockAllocMap[i]==UNUSED_BLK) break;
    }
    if(i==DISK_BLOCKS) return E_DISKFULL;
    // iterate through the StaticBuffer::blockAllocMap and find the block number
    // of a free block in the disk.
    // if no block is free, return E_DISKFULL.
    this->blockNum=i;
    // set the object's blockNum to the block number of the free block.

    // find a free buffer using StaticBuffer::getFreeBuffer() 
    int bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);
    struct HeadInfo *head=(struct HeadInfo *)malloc(sizeof(struct HeadInfo));
    head->pblock=-1;
    head->lblock=-1;
    head->rblock=-1;
    head->numEntries=0;
    head->numAttrs=0;
    head->numSlots=0;
    this->setHeader(head);
    // initialize the header of the block passing a struct HeadInfo with values
    // pblock: -1, lblock: -1, rblock: -1, numEntries: 0, numAttrs: 0, numSlots: 0
    // to the setHeader() function.
    this->setBlockType(blockType);
    return this->blockNum;
    // update the block type of the block to the input block type using setBlockType().

    // return block number of the free block.
}

int RecBuffer::setSlotMap(unsigned char* slotMap) {
	unsigned char* bufferPtr;
	/* get the starting address of the buffer containing the block using
	   loadBlockAndGetBufferPtr(&bufferPtr). */
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);

	// if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
		// return the value returned by the call.
	if (ret != SUCCESS) {
		ret;
	}

	// get the header of the block using the getHeader() function
	HeadInfo head;
	getHeader(&head);

	/* the number of slots in the block */
	int numSlots = head.numSlots;

	// the slotmap starts at bufferPtr + HEADER_SIZE. Copy the contents of the
	// argument `slotMap` to the buffer replacing the existing slotmap.
	// Note that size of slotmap is `numSlots`
	memcpy(bufferPtr + HEADER_SIZE, slotMap, numSlots);

	// update dirty bit using StaticBuffer::setDirtyBit
	// if setDirtyBit failed, return the value returned by the call

    // return SUCCESS
	return StaticBuffer::setDirtyBit(ret);
}

void BlockBuffer::releaseBlock(){

       if (this->blockNum == INVALID_BLOCKNUM ) return ;
        /* get the buffer number of the buffer assigned to the block
           using StaticBuffer::getBufferNum().
           (this function return E_BLOCKNOTINBUFFER if the block is not
           currently loaded in the buffer)
            */
        int bufnum=StaticBuffer::getBufferNum(this->blockNum);
        // if the block is present in the buffer, free the buffer
        // by setting the free flag of its StaticBuffer::tableMetaInfo entry
        // to true.
        if(bufnum>=0&&bufnum<BUFFER_CAPACITY)
        {
               
        StaticBuffer::blockAllocMap[this->blockNum]=UNUSED_BLK;
        this->blockNum=INVALID_BLOCKNUM;
}        // free the block in disk by setting the data type of the entry
        // corresponding to the block number in StaticBuffer::blockAllocMap
        // to UNUSED_BLK.

        // set the object's blockNum to INVALID_BLOCK (-1)
}

int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType) {

    double diff;
    if (attrType == STRING)
        diff = strcmp(attr1.sVal, attr2.sVal);

     else
         diff = attr1.nVal - attr2.nVal;

    if (diff > 0)  return 1;
    else if (diff < 0)  return -1;
      return 0;
}

// call the corresponding parent constructor
IndBuffer::IndBuffer(char blockType) : BlockBuffer(blockType){}
// call the corresponding parent constructor
IndBuffer::IndBuffer(int blockNum) : BlockBuffer(blockNum){}
IndInternal::IndInternal() : IndBuffer('I'){}
// call the corresponding parent constructor
// 'I' used to denote IndInternal.
IndInternal::IndInternal(int blockNum) : IndBuffer(blockNum){}
// call the corresponding parent constructor
IndLeaf::IndLeaf() : IndBuffer('L'){} // this is the way to call parent non-default constructor.
                      // 'L' used to denote IndLeaf.
//this is the way to call parent non-default constructor.
IndLeaf::IndLeaf(int blockNum) : IndBuffer(blockNum){}

// Gives the indexNumth entry of the block.
int IndInternal::getEntry(void *ptr, int indexNum) {
    // if the indexNum is not in the valid range of [0, MAX_KEYS_INTERNAL-1]
    //     return E_OUTOFBOUND.
	if (indexNum < 0 || indexNum >= MAX_KEYS_INTERNAL) {
		return E_OUTOFBOUND;
	}
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
	int retVal = loadBlockAndGetBufferPtr(&bufferPtr);
    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
    //     return the value returned by the call.
	if (retVal != SUCCESS) {
		return retVal;
	}
    // typecast the void pointer to an internal entry pointer
    struct InternalEntry *internalEntry = (struct InternalEntry *)ptr;
    /*
    - copy the entries from the indexNum`th entry to *internalEntry
    - make sure that each field is copied individually as in the following code
    - the lChild and rChild fields of InternalEntry are of type int32_t
    - int32_t is a type of int that is guaranteed to be 4 bytes across every
      C++ implementation. sizeof(int32_t) = 4
    */
    /* the indexNum'th entry will begin at an offset of
       HEADER_SIZE + (indexNum * (sizeof(int) + ATTR_SIZE) )         [why?]
       from bufferPtr */
    unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * 20);
    memcpy(&(internalEntry->lChild), entryPtr, sizeof(int32_t));
    memcpy(&(internalEntry->attrVal), entryPtr + 4, sizeof(Attribute));
    memcpy(&(internalEntry->rChild), entryPtr + 20, 4);
    // return SUCCESS.
	return SUCCESS;
}
// Gives the indexNumth entry of the block.
int IndLeaf::getEntry(void *ptr, int indexNum) {
    // if the indexNum is not in the valid range of [0, MAX_KEYS_LEAF-1]
    //     return E_OUTOFBOUND.
	if (indexNum < 0 || indexNum >= MAX_KEYS_INTERNAL) {
		return E_OUTOFBOUND;
	}
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
	int retVal = loadBlockAndGetBufferPtr(&bufferPtr);
    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
    //     return the value returned by the call.
	if (retVal != SUCCESS) {
		return retVal;
	}
    // copy the indexNum'th Index entry in buffer to memory ptr using memcpy
    /* the indexNum'th entry will begin at an offset of
       HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE)  from bufferPtr */
    unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE);
    memcpy((struct Index *)ptr, entryPtr, LEAF_ENTRY_SIZE);
    // return SUCCESS
	return SUCCESS;
}

// for avoiding compile error
int IndInternal::setEntry(void *ptr, int indexNum) {
    // if the indexNum is not in the valid range of [0, MAX_KEYS_INTERNAL-1]
    //     return E_OUTOFBOUND.
	if (indexNum < 0 || indexNum >= MAX_KEYS_INTERNAL) {
		return E_OUTOFBOUND;
	}
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);
    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
    //     return the value returned by the call.
	if (ret != SUCCESS) {
		return ret;
	}
    // typecast the void pointer to an internal entry pointer
    struct InternalEntry *internalEntry = (struct InternalEntry *)ptr;
    /*
    - copy the entries from *internalEntry to the indexNum`th entry
    - make sure that each field is copied individually as in the following code
    - the lChild and rChild fields of InternalEntry are of type int32_t
    - int32_t is a type of int that is guaranteed to be 4 bytes across every
      C++ implementation. sizeof(int32_t) = 4
    */
    /* the indexNum'th entry will begin at an offset of
       HEADER_SIZE + (indexNum * (sizeof(int) + ATTR_SIZE) )         [why?]
       from bufferPtr */
    unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * (sizeof(int32_t) + ATTR_SIZE));
    memcpy(entryPtr, &(internalEntry->lChild), 4);
    memcpy(entryPtr + 4, &(internalEntry->attrVal), ATTR_SIZE);
    memcpy(entryPtr + 20, &(internalEntry->rChild), 4);
    // update dirty bit using setDirtyBit()
    // if setDirtyBit failed, return the value returned by the call
    // return SUCCESS
	return StaticBuffer::setDirtyBit(this->blockNum);
}

int IndLeaf::setEntry(void *ptr, int indexNum) {
    // if the indexNum is not in the valid range of [0, MAX_KEYS_INTERNAL-1]
    //     return E_OUTOFBOUND.
	if (indexNum < 0 || indexNum >= MAX_KEYS_INTERNAL) {
		return E_OUTOFBOUND;
	}
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
	int ret = loadBlockAndGetBufferPtr(&bufferPtr);
    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
    //     return the value returned by the call.
	if (ret != SUCCESS) {
		return ret;
	}
    // copy the Index at ptr to indexNum'th entry in the buffer using memcpy
    /* the indexNum'th entry will begin at an offset of
       HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE)  from bufferPtr */
    unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE);
    memcpy(entryPtr, (struct Index *)ptr, LEAF_ENTRY_SIZE);
    // update dirty bit using setDirtyBit()
    // if setDirtyBit failed, return the value returned by the call
    //return SUCCESS
	return StaticBuffer::setDirtyBit(this->blockNum);
}
