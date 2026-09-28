#include "rose_config.h"
#include "AST_FILE_IO.h"
#include <string.h>
#include <algorithm>
#include <ROSE_UNUSED.h>

namespace AST_FILE_IO_MARKER
{
    void writeMarker(std::string marker, std::ostream& outputFileStream) {
#if FILE_IO_MARKER
        outputFileStream.write(marker.c_str(),marker.size());
#endif
    }

    void readMarker(std::string marker, std::istream& inputFileStream) {
#if FILE_IO_MARKER
        char* markerIn{new char[marker.size()+1]};
        markerIn[marker.size()] = '\0';
        inputFileStream.read(markerIn,marker.size());
        ASSERT_require(marker == markerIn);
        delete [] markerIn;
#endif
    }
};

// The static memory pool is empty at the beginning.
template <class TYPE>
unsigned long StorageClassMemoryManagement<TYPE>::filledUpTo{0};

// The size for one memory block is constantly set.
template <class TYPE>
unsigned long StorageClassMemoryManagement<TYPE>::blockSize{INITIAL_SIZE_OF_MEMORY_BLOCKS};

// There working memory block is always actualBlock-1, so it is 0 in the beginning
template <class TYPE>
unsigned int StorageClassMemoryManagement<TYPE>::actualBlock{0};

// There are no blocks allocated in the beginning.
template <class TYPE>
unsigned int StorageClassMemoryManagement<TYPE>::blocksAllocated{0};

// The memoryBlockList is pointing to a valid pointer at the start,
// in order to be able to call the delete operator. This seems to be
// useless, but skips one if-statement that would be called very often!
template <class TYPE>
TYPE** StorageClassMemoryManagement<TYPE>::memoryBlockList{nullptr};

// Just a working pointer, pointing to the position to be set next. In
// principle it is just pointing to the position filledUpTo
template <class TYPE>
TYPE* StorageClassMemoryManagement<TYPE>::actual{nullptr};

/* method for initializing the working data before adding
   * the sizeOfData is set
   * the positionInTheStaticMemoryPool is set to the next one that will be filled,
     namely filledUpTo
   * filledUpTo gets increased by the amount of the new data
   * finally, the offset is computed and returned. The offset contains that amount of data,
     that does not fit in the actual memory block, i.e.
      * it is negative or zero, if there is still enough space in the actual block
      * it is positive, if the data does not fit completely in the actual block
*/
template <class TYPE>
long StorageClassMemoryManagement<TYPE> :: setPositionAndSizeAndReturnOffset (long sizeOfData_) {
    sizeOfData = sizeOfData_;
    positionInStaticMemoryPool = filledUpTo;
    filledUpTo += sizeOfData;
    return (filledUpTo - actualBlock*blockSize);
}

// method that returns the a pointer beginning the beginning of the actual memory block
template <class TYPE>
TYPE* StorageClassMemoryManagement<TYPE> :: getBeginningOfActualBlock() const {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
    ASSERT_not_null(actual);
    ASSERT_require(0 < actualBlock);
    ASSERT_not_null(memoryBlockList);
    ASSERT_not_null(memoryBlockList[actualBlock-1]);
#endif
    return memoryBlockList[actualBlock-1];
}

// method that returns the a pointer to the beginning of the data block stored
// in the static array! Only used for the rebuilding of the data, in order to enable several
// additional asserts and eases the implementation!
template <class TYPE>
TYPE* StorageClassMemoryManagement<TYPE> :: getBeginningOfDataBlock() const {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
    ASSERT_not_null(actual);
    ASSERT_require(actualBlock == 1);
    ASSERT_not_null(memoryBlockList);
    ASSERT_not_null(memoryBlockList[0]);
    ASSERT_require(0 <= sizeOfData);
    ASSERT_require(filledUpTo <= blockSize);
#endif
    return (memoryBlockList[0] + positionInStaticMemoryPool);
}

/* "new-method": this method returns a new memory block. First, we check, if there is still
   a free pointer in the memoryBlockList.
   * if no, we extend the memory block list by MEMORY_BLOCK_LIST_INCREASE (defined at the beginning of
     this file). Therefore, we build a new list that has space for MEMORY_BLOCK_LIST_INCREASE more memory
     block pointers. Subsequently, we copy the pointers from the old memoryBlockList to the new one,
     delete the old one, and finally replace the old one by the new one. Then we have free pointers in
     the memoryBlockList
   * if yes, we just allocate a new memory block of size blockSize, increase the actualBlock and return
     the new memory block
*/
template <class TYPE>
TYPE* StorageClassMemoryManagement<TYPE> :: getNewMemoryBlock() {
    // check, wheather there is no more free block pointer
    if (actualBlock >= blocksAllocated) {
        // allocate  new space for block poitners and copy the old one
        // in the new array; delete the old array
        TYPE** new_memoryBlockList{new TYPE*[blocksAllocated + MEMORY_BLOCK_LIST_INCREASE]};
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
        ASSERT_not_null(new_memoryBlockList);
#endif
        if (memoryBlockList != nullptr) {
            memcpy(new_memoryBlockList, memoryBlockList, blocksAllocated * sizeof(TYPE*));
            delete [] memoryBlockList;
        }
        blocksAllocated += MEMORY_BLOCK_LIST_INCREASE;
        memoryBlockList = new_memoryBlockList;
    }
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
    ASSERT_not_null(memoryBlockList);
#endif
  // return a new memory block and increase the actualBlock
    memoryBlockList[actualBlock] = new TYPE[blockSize];
    ++actualBlock;
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
    ASSERT_not_null(memoryBlockList[actualBlock-1]);
#endif
    return memoryBlockList[actualBlock-1];
}

/* "delete-method": since we handle all data as static, we provide this method, in order to
   delete the static memory pool. If the memoryBlockList is not empty (e.g. 0 < actualBlock)
   we delete the pool, otherwise we do nothing.
*/
template <class TYPE>
void StorageClassMemoryManagement<TYPE> :: deleteMemoryPool() {
    if (0 < actualBlock) {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
        ASSERT_not_null(memoryBlockList);
#endif
        // delete the memory pool and reset the static memoryBlockList
        // used as destructor and way to write the data only once.
        for (unsigned int i = 0; i < actualBlock; ++i) {
            delete [] memoryBlockList[i];
        }
        delete [] memoryBlockList;
        memoryBlockList = nullptr;
        actual = nullptr;
        filledUpTo = 0;
        actualBlock = 0;
        blocksAllocated = 0;
        blockSize = INITIAL_SIZE_OF_MEMORY_BLOCKS;
    }
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
    // check, if all data was reset to its initial values
    ASSERT_require(memoryBlockList == nullptr);
    ASSERT_require(actual == nullptr);
    ASSERT_require(actualBlock == 0);
    ASSERT_require(blocksAllocated == 0);
    ASSERT_require(filledUpTo == 0);
    ASSERT_require(blockSize == INITIAL_SIZE_OF_MEMORY_BLOCKS);
#endif
}

// return the amount of the data stored or to be stored
template <class TYPE>
long StorageClassMemoryManagement<TYPE> :: getSizeOfData() const {
    return sizeOfData;
}

// print data that is stored, only used for debugging
template <class TYPE>
void StorageClassMemoryManagement<TYPE>::displayStorageClassData() const {
    for (long i = 0; i < getSizeOfData(); ++i) {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
        ASSERT_not_null(memoryBlockList);
#endif
        unsigned int offset = (positionInStaticMemoryPool+i)%blockSize;
        std::cout << memoryBlockList[(positionInStaticMemoryPool+i-offset)/blockSize][offset];
    }
}

/* method for writing the memoryBlockList to disk. Only the amount of the data and the static
   data gets stored. The plain data (positionInStaticMemoryPool, sizeOfData) is stored by the
   class where the EasyStorage object is declared and used. Therefore, the any EasyStorage
   class might always be a object, never a poitner.
   Further, we always save the filledUpTo, even if the data is already stored and filledUpTo
   was set to 0. Then we store just the 0 and nothin more is done. The reasen for this is
   the reading back of the data is calles in the same order and will so read the 0 and
   also do nothing. I am not very glad with this implementation, because this 0s could be
   spared, by changing the StorageClassMemoryManagement class. Since this is not a real
   perofrmace issue (its only static data, and we already try to spare the doubles in
   the StorageClasses) I just leave it in that manner. But this can be improved!
*/
template <class TYPE>
void StorageClassMemoryManagement<TYPE> :: writeToFile(std::ostream& outputFileStream) {
#if FILE_IO_MARKER
    AST_FILE_IO_MARKER::writeMarker("|01|",outputFileStream);
#endif
    outputFileStream.write((char*)(&filledUpTo), sizeof(filledUpTo));
    // since the method might get called several times, we only want to store once
    if (0 < filledUpTo ) {
        ASSERT_not_null(actual);
        ASSERT_not_null(memoryBlockList);
        ASSERT_require(0 < actualBlock);
        for (unsigned int i = 0; i < actualBlock-1; ++i) {
            ASSERT_not_null(memoryBlockList[i]);
            outputFileStream.write((char*)(memoryBlockList[i]), blockSize*sizeof(TYPE));
        }
        outputFileStream.write((char*)(memoryBlockList[actualBlock-1]), ((filledUpTo-1)%blockSize+1)*sizeof(TYPE));
        // delete the pool, since the data is not needed any more!
        deleteMemoryPool();
    }
}

/* reading the data from a file. Since we read first the total amount of datai ( stored in
   filledUpTo ), the memory pool is created in the manner, that is has only one memory block
   with filledUpTo size, i.e. memoryBlockSize is set to filledUpTo. Reasons for this
   construction:
    * this accelerates the reading of the data, since we can read one big block from disk
    * the location of the data ( positionInStaticMemoryPool ) is the global position in the pool.
      Therefore, we just can access the position without any further pool computations
*/
template <class TYPE>
void StorageClassMemoryManagement<TYPE> :: readFromFile (std::istream& inputFileStream) {
#if FILE_IO_MARKER
    AST_FILE_IO_MARKER::readMarker("|01|",inputFileStream);
#endif
    inputFileStream.read((char*)(&filledUpTo),sizeof(filledUpTo));
    // since the method might get called several times, we only want to read the data one.
    if (0 < filledUpTo) {
        ASSERT_require(actual == nullptr);
        ASSERT_require(memoryBlockList == nullptr);
        memoryBlockList = new TYPE*[1];
        memoryBlockList[0] = new TYPE[filledUpTo];
        inputFileStream.read((char*)(memoryBlockList[0]), filledUpTo*sizeof(TYPE));
        blockSize = filledUpTo;
        actualBlock = 1;
        blocksAllocated = 1;
        actual = memoryBlockList[0] + filledUpTo;
    }
}

/* This method reorganizes the memory pool to contain all data in one memory block.
   It is only used for testing, for storing and repairing the data without writing
   to a file !
*/
template <class TYPE>
void StorageClassMemoryManagement<TYPE> :: arrangeMemoryPoolInOneBlock() {
    // check, wheather the data belongs to more than one memory block
    if (actual != nullptr) {
        if (1 < actualBlock) {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
            ASSERT_not_null(memoryBlockList);
#endif
            TYPE* newMemoryBlock{new TYPE [filledUpTo]};
            unsigned int i{0};
            for (i = 0; i < actualBlock - 1; ++i) {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
                ASSERT_not_null(memoryBlockList[i]);
#endif
                // memcpy(newMemoryBlock + (i*blockSize), memoryBlockList[i], blockSize*sizeof(TYPE));
                std::copy(memoryBlockList[i], memoryBlockList[i] + blockSize, newMemoryBlock + i*blockSize);
            }
            // memcpy(newMemoryBlock + i*blockSize, memoryBlockList[i], ((filledUpTo-1)%blockSize + 1) * sizeof(TYPE));
            std::copy(memoryBlockList[i], memoryBlockList[i] + ((filledUpTo-1)%blockSize + 1), newMemoryBlock + i*blockSize);

            for (i = 0; i < actualBlock; ++i) {
                delete [] memoryBlockList[i];
            }
            memoryBlockList[0] = newMemoryBlock;
            blockSize = filledUpTo;
            actual = &(memoryBlockList[0][filledUpTo]);
            actualBlock = 1;
            blocksAllocated = 1;
        }
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
        ASSERT_not_null(memoryBlockList);
        ASSERT_not_null(memoryBlockList[0]);
        ASSERT_require(filledUpTo <= blockSize);
        ASSERT_not_null(actual);
        ASSERT_require(actualBlock <= 1);
#endif
    }
}

/***************************************************************************************
  JH (01/11/2006) EasyStorage for arrays of plain data (mainly char arrays)
  Some comments to the implementation at all:
  * NULL pointers are stored equally to empty arrays, lists, etc. This means, that if
    rebuilded we will have an valid object behind the pointer. The exeptions are
    * AstAttributeMechanism
    therefore an NULL pointer will be recreated ( this is realized by setting sizeOfData
    to -1 and check while rebuilt)
  * the actual pointer is pointing to the next position to be stored in. Except for the case, the
    memmory pool is not allocated at all, then the actual pointer is NULL. Therefore, we use
    the actual pointer for checking if the pool is valid (everytime you see (actual != NULL) )
  * every storting is mainly separated in 2 parts:
    * the setup with the call of setPositionAndSizeAndReturnOffset
    * the storing of the data, checking, whether it fits in the actual memory block or not
      ( done by ( 0 < offset ) )
      * the case it does not fit is separated in 2 or 3 parts
        * filling the actual memory block
        * getting a new block and iterating while the rest of the data does not fit in a
          whole memory block
        * storing of the rest ( runs sometimes just to storing data )
    * more complex data (PreprocessingInfo, maps, AstAttributeMechanism) contain
      EasyStorage classes themself. Therefore, we must provide explicit member functions
      deleting the static data of those. These functions call also the corresponding one
      of the parent class (if the class is an inherited class).
***************************************************************************************/

/*
   ****************************************************************************************
   **           Implementations for EasyStorage <char*>                                  **
   ****************************************************************************************
*/
void EasyStorage<char*> :: storeDataInEasyStorageClass(const char* data_) {
    if (data_ == nullptr) {
        Base::sizeOfData = -1;
    }
    else {
        storeDataInEasyStorageClass(data_, strlen(data_));
    }
}

void EasyStorage<char*> :: storeDataInEasyStorageClass(const char* data_, int sizeOfData_) {
    if (data_ == nullptr) {
        Base::sizeOfData = -1;
    }
    else {
        // get changeable pointer
        char* copy_{const_cast<char*>(data_)};
        long offset{Base::setPositionAndSizeAndReturnOffset(sizeOfData_)};
        // if the new data does not fit in the actual block
        if (0 < offset) {
            // if there is still space in the actual block
            if (offset < Base::getSizeOfData() && Base::actual != nullptr) {
                memcpy(Base::actual, copy_, (Base::getSizeOfData()-offset)*sizeof(char));
                copy_ += (Base::getSizeOfData() - offset);
            }
            // the data does not fit in one block
            while (Base::blockSize < (unsigned long)(offset)) {
                Base::actual = Base::getNewMemoryBlock();
                memcpy(Base::actual, copy_, Base::blockSize*sizeof(char));
                offset -= Base::blockSize;
                copy_ += Base::blockSize;
            }
            // put the rest of the data in a new memory block
            Base::actual = Base::getNewMemoryBlock();
            memcpy(Base::actual, copy_, offset*sizeof(char));
            Base::actual += offset;
        }
        else {
            // put the data in the memory block
            memcpy(Base::actual, copy_, Base::getSizeOfData()*sizeof(char));
            Base::actual += Base::getSizeOfData();
        }
    }
}

char* EasyStorage<char*> :: rebuildDataStoredInEasyStorageClass() const {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
    ASSERT_require(Base::actualBlock <= 1);
    ASSERT_require((0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0));
#endif
    char* data_{nullptr};
    if (0 <= Base::getSizeOfData()) {
        data_ = new char[Base::getSizeOfData()+1];
        // if there is any data in the pool at all
        if (Base::actual != nullptr && 0 < Base::getSizeOfData()) {
            memcpy(data_, Base::getBeginningOfDataBlock(), Base::getSizeOfData() * sizeof(char));
        }
        // terminate char array
        data_[Base::getSizeOfData()] = '\0';
    }
    return data_;
}

/*
   ****************************************************************************************
   **           Implementations for EasyStorage <std::string>                            **
   ****************************************************************************************
*/
void EasyStorage<std::string> :: storeDataInEasyStorageClass(const std::string& data_) {
    // get changeable pointer
    char* copy_{const_cast<char*>(data_.c_str())};
    long offset{Base::setPositionAndSizeAndReturnOffset(strlen(copy_))};
    // if the new data does not fit in the actual block
    if (0 < offset) {
        // if there is still space in the actual block
        if (offset < Base::getSizeOfData() && Base::actual != nullptr) {
            memcpy(Base::actual, copy_, (Base::getSizeOfData()-offset)*sizeof(char));
            copy_ += (Base::getSizeOfData()-offset);
        }
        // the data does not fit in one block
        while (Base::blockSize < (unsigned long)(offset)) {
            Base::actual = Base::getNewMemoryBlock();
            memcpy(Base::actual, copy_, Base::blockSize*sizeof(char));
            offset -= Base::blockSize;
            copy_ += Base::blockSize;
        }
        // put the rest of the data in a new memory block
        Base::actual = Base::getNewMemoryBlock();
        memcpy(Base::actual, copy_ , offset*sizeof(char) );
        Base::actual += offset;
    }
    else {
        // put the the data in the memory block
        memcpy(Base::actual, copy_, Base::getSizeOfData()*sizeof(char));
        Base::actual += Base::getSizeOfData();
    }
}

std::string EasyStorage<std::string> :: rebuildDataStoredInEasyStorageClass() const {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
     ASSERT_require(Base::actualBlock <= 1);
     ASSERT_require((0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() == 0));
#endif
      std::string return_string;
   // if there is any data in the pool at all
      if (Base::actual != NULL && 0 < Base::getSizeOfData() )
         {
           return_string = std::string(Base::getBeginningOfDataBlock(),Base::getSizeOfData());
         }
      return return_string;
   }


/*
   ****************************************************************************************
   **       Implementations for EasyStorage <Sawyer::Container::BitVector>               **
   ****************************************************************************************
*/

void
EasyStorage<Sawyer::Container::BitVector>::storeDataInEasyStorageClass(const Sawyer::Container::BitVector& data_) {
    typedef Sawyer::Container::BitVector::Word BVWord;
    const BVWord* words{data_.data()};                  // the raw words of the bit vector or null
    size_t nWords{data_.dataSize()};                    // number of words to save
    BVWord excess = data_.size() % (8*sizeof(BVWord));  // number of bits in final word; zero means full word, or empty vector
    size_t idx{0};                                      // index into the destination words
    long offset{Base::setPositionAndSizeAndReturnOffset(1+nWords)};
    if (offset > 0) {
        // there is still space in the actual block
        if (offset < Base::getSizeOfData() && Base::actual != nullptr) {
            while ((unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize) {
                ASSERT_not_null(words);
                ASSERT_require(idx <= nWords);
                *Base::actual++ = 0==idx ? excess : words[idx-1];
                ++idx;
            }
        }
        // the data might not fit in one block
        while (Base::blockSize < (unsigned long)offset) {
            Base::actual = Base::getNewMemoryBlock();
            while ((unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize) {
                ASSERT_not_null(words);
                ASSERT_require(idx <= nWords);
                *Base::actual++ = 0==idx ? excess : words[idx-1];
                ++idx;
            }
            offset -= Base::blockSize;
        }
        Base::actual = Base::getNewMemoryBlock();
    }
    // put (the rest of) the data in a new memory block
    while (idx <= nWords) {
        ASSERT_not_null(words);
        ASSERT_require(idx <= nWords);
        *Base::actual++ = 0==idx ? excess : words[idx-1];
        ++idx;
    }
}

Sawyer::Container::BitVector
EasyStorage<Sawyer::Container::BitVector>::rebuildDataStoredInEasyStorageClass() const {
    ASSERT_require(Base::actualBlock <= 1);
    ASSERT_require((0 < Base::getSizeOfData() && Base::actual != nullptr) || Base::getSizeOfData() == 0);
    typedef Sawyer::Container::BitVector::Word BVWord;
    Sawyer::Container::BitVector retval;
    // if there is data in the memory pool at all
    if (Base::actual != NULL  && 0 < Base::getSizeOfData()) {
        BVWord *words = Base::getBeginningOfDataBlock();

        // The first stored word is the number of bits that are stored in the last word (zero means the last word is full if it
        // exists). This is our opportunity to allocate space for the vector.
        ASSERT_require(Base::getSizeOfData() >= 1);
        size_t excess{*words++};
        ASSERT_require(excess < 8*sizeof(BVWord));
        size_t nBits{0};
        if (Base::getSizeOfData() == 1) {
            // bit vector is empty
            ASSERT_require(excess == 0);
        } else if (0==excess) {
            // final word is full
            nBits = (Base::getSizeOfData()-1) * 8 * sizeof(BVWord);
        } else {
            // final word is partial
            ASSERT_require(Base::getSizeOfData() >= 2);
            nBits = (Base::getSizeOfData()-2) * 8 * sizeof(BVWord) + excess;
        }
        retval.resize(nBits);

        // Now read the data and load it into the bit vector, note that since
        // the size is known to be positive the pointer better not be nil.
        BVWord *dst = retval.data();
        ASSERT_not_null(dst);
        for (size_t i=1; i < (size_t)Base::getSizeOfData(); ++i) {
            *dst++ = *words++;
        }
    }
    return retval;
}


/*
   ****************************************************************************************
   **       Implementations for EasyStorage <CONTAINER<TYPE>>                           **
   ****************************************************************************************
*/
template <class TYPE, template <class A> class CONTAINER >
void EasyStorage <CONTAINER<TYPE>> :: storeDataInEasyStorageClass(const CONTAINER<TYPE>& data_)
   {
  // get iterator pointing to begin
     typename CONTAINER<TYPE>::const_iterator dat = data_.begin();
     long offset = Base::setPositionAndSizeAndReturnOffset ( data_.size() ) ;
  // if the new data does not fit in the actual block
     if (0 < offset)
        {
       // if there is still space in the actual block
          if ( offset < Base::getSizeOfData()  && Base::actual != NULL )
             {
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                  {
                    *Base::actual = *dat;
                  }
             }
       // the data does not fit in one block
          while (Base::blockSize < (unsigned long)(offset))
             {
               Base::actual = Base::getNewMemoryBlock();
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                  {
                    *Base::actual = *dat;
                  }
               offset -= Base::blockSize;
             }
       // get a new memory block
          Base::actual = Base::getNewMemoryBlock();
        }
  // put (the rest of) the data in a new memory block
     for (; dat != data_.end(); ++dat, ++Base::actual)
        {
          *Base::actual = *dat;
        }
   }

template <class TYPE, template <class A> class CONTAINER >
CONTAINER<TYPE> EasyStorage <CONTAINER<TYPE>> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
     ASSERT_require(Base::actualBlock <= 1);
     ASSERT_require((0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() == 0));
#endif

     CONTAINER<TYPE> data_;
   // if there is data in the memory pool at all
      if ( Base::actual != NULL  && 0 < Base::getSizeOfData() )
         {
           TYPE* pointer = Base::getBeginningOfDataBlock();
           for (long i=0; i < Base::getSizeOfData(); ++i )
              {
                data_.push_back(*(pointer+i));
              }
         }
      return data_;
   }



/*
   ****************************************************************************************
   **       Implementations for EasyStorage <std::list<TYPE>>                           **
   ****************************************************************************************
*/
template <class TYPE>
void EasyStorage <std::list<TYPE>> :: storeDataInEasyStorageClass(const std::list<TYPE>& data_)
   {
  // get iterator pointing to begin
     typename std::list<TYPE>::const_iterator dat = data_.begin();
     long offset = Base::setPositionAndSizeAndReturnOffset ( data_.size() ) ;
  // if the new data does not fit in the actual block
     if (0 < offset)
        {
       // if there is still space in the actual block
          if ( offset < Base::getSizeOfData()  && Base::actual != NULL )
             {
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                  {
                    *Base::actual = *dat;
                  }
             }
       // the data does not fit in one block
          while (Base::blockSize < (unsigned long)(offset))
             {
               Base::actual = Base::getNewMemoryBlock();
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                  {
                    *Base::actual = *dat;
                  }
               offset -= Base::blockSize;
             }
       // get a new memory block
          Base::actual = Base::getNewMemoryBlock();
        }
  // put (the rest of) the data in a new memory block
     for (; dat != data_.end(); ++dat, ++Base::actual)
        {
          *Base::actual = *dat;
        }
   }

template <class TYPE>
std::list<TYPE> EasyStorage <std::list<TYPE>> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
     assert ( Base::actualBlock <= 1 );
     assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() == 0 ) );
#endif

     std::list<TYPE> data_;
   // if there is data in the memory pool at all
      if ( Base::actual != NULL  && 0 < Base::getSizeOfData() )
         {
           TYPE* pointer = Base::getBeginningOfDataBlock();
           for (long i=0; i < Base::getSizeOfData(); ++i )
              {
                data_.push_back(*(pointer+i));
              }
         }
      return data_;
   }



/*
   ****************************************************************************************
   **       Implementations for EasyStorage <std::vector<TYPE>>                         **
   ****************************************************************************************
*/
template <class TYPE>
void EasyStorage <std::vector<TYPE>> :: storeDataInEasyStorageClass(const std::vector<TYPE>& data_)
   {
  // get iterator pointing to begin
     typename std::vector<TYPE>::const_iterator dat = data_.begin();
     long offset = Base::setPositionAndSizeAndReturnOffset ( data_.size() ) ;
  // if the new data does not fit in the actual block
     if (0 < offset)
        {
       // if there is still space in the actual block
          if ( offset < Base::getSizeOfData()  && Base::actual != NULL )
             {
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                  {
                    *Base::actual = *dat;
                  }
             }
       // the data does not fit in one block
          while (Base::blockSize < (unsigned long)(offset))
             {
               Base::actual = Base::getNewMemoryBlock();
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                  {
                    *Base::actual = *dat;
                  }
               offset -= Base::blockSize;
             }
       // get a new memory block
          Base::actual = Base::getNewMemoryBlock();
        }
  // put (the rest of) the data in a new memory block
     for (; dat != data_.end(); ++dat, ++Base::actual)
        {
          *Base::actual = *dat;
        }
   }

template <class TYPE>
std::vector<TYPE> EasyStorage <std::vector<TYPE>> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
     assert ( Base::actualBlock <= 1 );
     assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() == 0 ) );
#endif

     std::vector<TYPE> data_;
   // if there is data in the memory pool at all
      if ( Base::actual != NULL  && 0 < Base::getSizeOfData() )
         {
           TYPE* pointer = Base::getBeginningOfDataBlock();
           for (long i=0; i < Base::getSizeOfData(); ++i )
              {
                data_.push_back(*(pointer+i));
              }
         }
      return data_;
   }


/*
   ****************************************************************************************
   **       Implementations for EasyStorage <std::set<TYPE>>                            **
   ****************************************************************************************
*/
template <class TYPE >
void EasyStorage <std::set<TYPE>> :: storeDataInEasyStorageClass(const std::set<TYPE>& data_)
   {
  // get iterator pointing to begin
     typename std::set<TYPE>::const_iterator dat = data_.begin();
     long offset = Base::setPositionAndSizeAndReturnOffset ( data_.size() ) ;
  // if the new data does not fit in the actual block
     if (0 < offset)
        {
       // if there is still space in the actual block
          if ( offset < Base::getSizeOfData() && Base::actual != NULL )
             {
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                  {
                    *Base::actual = *dat;
                  }
             }
       // the data does not fit in one block
          while (Base::blockSize < (unsigned long)(offset))
             {
               Base::actual = Base::getNewMemoryBlock();
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                  {
                    *Base::actual = *dat;
                  }
               offset -= Base::blockSize;
             }
       // get a new memory block
          Base::actual = Base::getNewMemoryBlock();
        }
  // put (the rest of) the data in a new memory block
     for (; dat != data_.end(); ++dat, ++Base::actual)
        {
          *Base::actual = *dat;
        }
   }

template <class TYPE >
std::set<TYPE> EasyStorage <std::set<TYPE>> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() == 0 ) );
#endif
      std::set<TYPE> data_;
   // if there is data in the memory pool at all
      if ( Base::actual != NULL  && 0 < Base::getSizeOfData() )
         {
           TYPE* pointer = Base::getBeginningOfDataBlock();
           for (long  i=0; i < Base::getSizeOfData(); ++i )
              {
                data_.insert(*(pointer+i));
              }
         }
      return data_;
   }


/*
   ****************************************************************************************
   **       Implementations for EasyStorage <std::unordered_set<TYPE>>                  **
   ****************************************************************************************
*/
template <class TYPE >
void EasyStorage <std::unordered_set<TYPE>> :: storeDataInEasyStorageClass(const std::unordered_set<TYPE>& data_)
   {
  // get iterator pointing to begin
     auto dat = data_.begin();
     long offset = Base::setPositionAndSizeAndReturnOffset ( data_.size() ) ;
  // if the new data does not fit in the actual block
     if (0 < offset)
        {
       // if there is still space in the actual block
          if ( offset < Base::getSizeOfData() && Base::actual != NULL )
             {
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                  {
                    *Base::actual = *dat;
                  }
             }
       // the data does not fit in one block
          while (Base::blockSize < (unsigned long)(offset))
             {
               Base::actual = Base::getNewMemoryBlock();
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                  {
                    *Base::actual = *dat;
                  }
               offset -= Base::blockSize;
             }
       // get a new memory block
          Base::actual = Base::getNewMemoryBlock();
        }
  // put (the rest of) the data in a new memory block
     for (; dat != data_.end(); ++dat, ++Base::actual)
        {
          *Base::actual = *dat;
        }
   }

template <class TYPE >
std::unordered_set<TYPE> EasyStorage <std::unordered_set<TYPE>> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() == 0 ) );
#endif
      std::unordered_set<TYPE> data_;
   // if there is data in the memory pool at all
      if ( Base::actual != NULL  && 0 < Base::getSizeOfData() )
         {
           TYPE* pointer = Base::getBeginningOfDataBlock();
           for (long  i=0; i < Base::getSizeOfData(); ++i )
              {
                data_.insert(*(pointer+i));
              }
         }
      return data_;
   }




/*
   ****************************************************************************************
   **       Implementations for EasyStorage <SgBitVector>                                **
   ****************************************************************************************
*/
void EasyStorage <SgBitVector> :: storeDataInEasyStorageClass(const SgBitVector& data_)
   {
  // get iterator pointing to begin
     SgBitVector::const_iterator dat = data_.begin();
     long offset = Base::setPositionAndSizeAndReturnOffset ( data_.size() ) ;
  // if the new data does not fit in the actual block
     if (0 < offset)
        {
       // if there is still space in the actual block
          if ( offset < Base::getSizeOfData() && Base::actual != NULL )
             {
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                  {
                    *Base::actual = *dat;
                  }
             }
       // the data does not fit in one block
          while (Base::blockSize < (unsigned long)(offset))
             {
               Base::actual = Base::getNewMemoryBlock();
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                  {
                    *Base::actual = *dat;
                  }
               offset -= Base::blockSize;
             }
       // get a new memory block
          Base::actual = Base::getNewMemoryBlock();
        }
  // put (the rest of) the data in a new memory block
     for (; dat != data_.end(); ++dat, ++Base::actual)
        {
          *Base::actual = *dat;
        }
   }

SgBitVector EasyStorage <SgBitVector> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() == 0 ) );
#endif
      SgBitVector data_;
   // if there is data in the memory pool at all
      if ( Base::actual != NULL  && 0 < Base::getSizeOfData() )
         {
           bool* pointer = Base::getBeginningOfDataBlock();
           for (long  i=0; i < Base::getSizeOfData(); ++i )
              {
                data_.insert(data_.end(),*(pointer+i));
              }
         }
      return data_;
   }

/*
   ****************************************************************************************
   **       Implementations for EasyStorage <std::list<std::string>>                    **
   ****************************************************************************************
*/
void EasyStorage <Rose_STL_Container<std::string>> :: storeDataInEasyStorageClass(const Rose_STL_Container<std::string>& data_)
   {
  // get iterator pointing to begin
     Rose_STL_Container<std::string>::const_iterator copy_ = data_.begin();
     long offset = Base::setPositionAndSizeAndReturnOffset ( data_.size() ) ;
  // if the new data does not fit in the actual block
     if (0 < offset)
        {
       // if there is still space in the actual block
          if ( offset < Base::getSizeOfData() && Base::actual != NULL )
             {
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++copy_)
                  {
                    actual->storeDataInEasyStorageClass(*copy_);
                  }
             }
      // the data does not fit in one block
          while (Base::blockSize < (unsigned long)(offset))
             {
               Base::actual = Base::getNewMemoryBlock();
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++copy_)
                  {
                    Base::actual->storeDataInEasyStorageClass(*copy_);
                  }
               offset -= Base::blockSize;
             }
       // get a new memory block
          Base::actual = Base::getNewMemoryBlock();
        }
  // put (the rest of) the data in a new memory block
     for ( ; copy_ != data_.end(); ++copy_, ++Base::actual )
        {
          Base::actual->storeDataInEasyStorageClass(*copy_);
        }
   }

void EasyStorage <Rose_STL_Container<std::string>>::displayEasyStorageData()
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
     assert ( Base::memoryBlockList != NULL );
#endif

  // DQ (8/24/2006): Fixed compiler warning
  // for (unsigned long i = 0; i < Base::getSizeOfData(); ++i)
     for (long i = 0; i < Base::getSizeOfData(); ++i)
        {
          long offset = (Base::positionInStaticMemoryPool+i)%Base::blockSize;
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
          assert ( Base::memoryBlockList[(Base::positionInStaticMemoryPool+i-offset)/Base::blockSize] != NULL );
#endif
       // Base::memoryBlockList[(Base::positionInStaticMemoryPool+i-offset)/Base::blockSize][offset].print();
       // Base::memoryBlockList[(Base::positionInStaticMemoryPool+i-offset)/Base::blockSize][offset].displayEasyStorageData();
          Base::memoryBlockList[(Base::positionInStaticMemoryPool+i-offset)/Base::blockSize][offset].displayStorageClassData();
          std::cout << std::endl;
        }
   }


Rose_STL_Container<std::string> EasyStorage <Rose_STL_Container<std::string>> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() == 0 ) );
#endif
      Rose_STL_Container<std::string> data_;
   // if there is any data in the pool
      if ( Base::actual != NULL  && 0 < Base::getSizeOfData() )
         {
           EasyStorage<std::string> *pointer = Base::getBeginningOfDataBlock();
           for ( long i=0; i < Base::getSizeOfData(); ++i )
              {
                data_.push_back( (pointer+i)->rebuildDataStoredInEasyStorageClass() );
              }
         }
      return data_;
   }

void EasyStorage <Rose_STL_Container<std::string>> :: arrangeMemoryPoolInOneBlock()
   {
    // calling the base class
     StorageClassMemoryManagement<EasyStorage<std::string>> :: arrangeMemoryPoolInOneBlock();
    // calling the basic type
     EasyStorage <std::string> :: arrangeMemoryPoolInOneBlock();
   }

void EasyStorage <Rose_STL_Container<std::string>> :: deleteMemoryPool()
   {
    // calling the base class
     StorageClassMemoryManagement<EasyStorage<std::string>> :: deleteMemoryPool();
    // calling the basic type
     EasyStorage <std::string> :: deleteMemoryPool();
   }


void EasyStorage <Rose_STL_Container<std::string>> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|03|",outputFileStream);
#endif
    // calling the base class
     StorageClassMemoryManagement<EasyStorage<std::string>> :: writeToFile(outputFileStream);
    // calling the basic type
     EasyStorage <std::string> :: writeToFile(outputFileStream);
   }

void EasyStorage <Rose_STL_Container<std::string>> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|03|",inputFileStream);
#endif
    // calling the base class
     StorageClassMemoryManagement<EasyStorage<std::string>> :: readFromFile (inputFileStream);
    // calling the basic type
     EasyStorage <std::string> :: readFromFile (inputFileStream);
   }



/*
   ****************************************************************************************
   **                Implementations for EasyStorage <SgName>                            **
   ****************************************************************************************
*/
/* The EasyStorage <SgName> class is not derived from the StorageClassMemoryManagement class!
   This is not necessary, since it only holds a data member EasyStorage <std::string>, which
   really contains the data ! Thus, the static data functions just have to call the methods
   on the data member type EasyStorage <std::string>!!
*/
void EasyStorage <SgName> :: storeDataInEasyStorageClass(const SgName& name)
   {
    // calling the method on the data member
       name_data.storeDataInEasyStorageClass(name.getString());
   }

SgName EasyStorage <SgName> :: rebuildDataStoredInEasyStorageClass() const
   {
    // calling the method on the data member
      return SgName (name_data.rebuildDataStoredInEasyStorageClass());
   }

void EasyStorage <SgName> :: arrangeMemoryPoolInOneBlock()
   {
    // calling the method on the data member
      EasyStorage <std::string> :: arrangeMemoryPoolInOneBlock();
   }


void EasyStorage <SgName> :: deleteMemoryPool()
   {
    // calling the method on the data member
     EasyStorage <std::string> :: deleteMemoryPool ();
   }


void EasyStorage <SgName> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|04|",outputFileStream);
#endif
    // calling the method on the data member
      EasyStorage <std::string> :: writeToFile(outputFileStream);
   }


void EasyStorage <SgName> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|04|",inputFileStream);
#endif
    // calling the method on the data member
     EasyStorage <std::string> :: readFromFile (inputFileStream);
   }


/*
   ****************************************************************************************
   **      Implementations for EasyStorageMapEntry <SgName,T>                            **
   ****************************************************************************************
*/
/* Since the class  EasyStorageMapEntry <SgName,T> is not inherited from StorageClassMemoryManagement,
   it does not have to call the static member functions of the base class!
*/
template <class T>
void EasyStorageMapEntry <SgName,T> :: storeDataInEasyStorageClass(std::pair<const SgName, T>& iter)
   {
    // calling the method on the data member
       sgNameString.storeDataInEasyStorageClass(iter.first);
       globalIndex = (T) (iter.second);
   }

template <class T>
std::pair<const SgName,T> EasyStorageMapEntry <SgName,T> :: rebuildDataStoredInEasyStorageClass() const
   {
    // calling the method on the data member
      std::pair<const SgName,T> return_pair(sgNameString.rebuildDataStoredInEasyStorageClass(), (T)(globalIndex) );
      return return_pair;
   }

template <class T>
void EasyStorageMapEntry <SgName,T> :: arrangeMemoryPoolInOneBlock()
   {
    // calling the method on the data member
      EasyStorage <SgName> :: arrangeMemoryPoolInOneBlock();
   }

template <class T>
void EasyStorageMapEntry <SgName,T> :: deleteMemoryPool()
   {
  // calling the method on the data member
     EasyStorage <SgName> :: deleteMemoryPool();
   }


template <class T>
void EasyStorageMapEntry <SgName,T> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|05|",outputFileStream);
#endif
    // calling the method on the data member
      EasyStorage <SgName> :: writeToFile(outputFileStream);
   }

template <class T>
void EasyStorageMapEntry <SgName,T> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|05|",inputFileStream);
#endif
  // calling the method on the data member
     EasyStorage <SgName> :: readFromFile (inputFileStream);
   }


/*
   ****************************************************************************************
   **      Implementations for EasyStorage <rose_hash_multimap*>                         **
   ****************************************************************************************
*/
void EasyStorage <rose_hash_multimap*> ::storeDataInEasyStorageClass(rose_hash_multimap* data_)
   {
     if ( data_ == NULL )
        {
          Base::sizeOfData = -1;
        }
     else
        {
       // store the parent pointer as unsigned long (this should better be AddrType). FixMe, also in the class declaration !
          parent = AST_FILE_IO :: getGlobalIndexFromSgClassPointer( data_->parent );
       // get starting iterator
          rose_hash_multimap::iterator copy_ = data_->begin();
          long offset = Base::setPositionAndSizeAndReturnOffset ( data_->size() ) ;
       // if the new data does not fit in the actual block
          if (0 < offset)
             {
            // if there is still space in the actual block
               if ( offset < Base::getSizeOfData() && Base::actual != NULL )
                  {
                    for (; (unsigned long) (Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++copy_)
                       {
                         Base::actual->storeDataInEasyStorageClass(*copy_);
                       }
                  }
            // the data does not fit in one block
               while (Base::blockSize < (unsigned long)(offset))
                 {
                  Base::actual = Base::getNewMemoryBlock();
                  for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++copy_)
                     {
                      Base::actual->storeDataInEasyStorageClass(*copy_);
                     }
                  offset -= Base::blockSize;
                 }
            // get new memory block, since the old one is full
               Base::actual = Base::getNewMemoryBlock();
             }
       // store the (rest of the ) data
          for ( ; copy_ != data_->end(); ++copy_, ++Base::actual )
             {
              Base::actual->storeDataInEasyStorageClass(*copy_);
             }
        }
   }


rose_hash_multimap*
EasyStorage <rose_hash_multimap*> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0 ) );
#endif
     rose_hash_multimap* return_map = NULL;
     if ( 0 <= Base::getSizeOfData() )
        {
          return_map = new rose_hash_multimap();
       // set the parent
          return_map->parent = AST_FILE_IO :: getSgClassPointerFromGlobalIndex(parent);
       // if the memory pool is valid
          if ( Base::actual != NULL  && 0 < Base::getSizeOfData() )
             {
               EasyStorageMapEntry<SgName,SgSymbol*> *pointer = Base::getBeginningOfDataBlock();
               for (int i = 0; i < Base::getSizeOfData(); ++i)
                  {
                    return_map->insert((pointer+i)->rebuildDataStoredInEasyStorageClass()) ;
                  }
             }
        }
     return return_map;
   }


void EasyStorage <rose_hash_multimap*> :: arrangeMemoryPoolInOneBlock()
   {
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: arrangeMemoryPoolInOneBlock();
      EasyStorageMapEntry <SgName,SgSymbol*> :: arrangeMemoryPoolInOneBlock();
   }


void EasyStorage <rose_hash_multimap*> :: deleteMemoryPool()
   {
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: deleteMemoryPool();
     EasyStorageMapEntry <SgName,SgSymbol*> :: deleteMemoryPool();
   }


void EasyStorage <rose_hash_multimap*> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|06|",outputFileStream);
#endif
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: writeToFile(outputFileStream);
      EasyStorageMapEntry <SgName,SgSymbol*> :: writeToFile( outputFileStream);
   }


void EasyStorage <rose_hash_multimap*> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|06|",inputFileStream);
#endif
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry <SgName,SgSymbol*> :: readFromFile (inputFileStream);
   }




/*
   ****************************************************************************************
   **      Implementations for EasyStorageMapEntry <std::string,AstAttribute*>           **
   ****************************************************************************************
*/
// Is not inherited from StorageClassMemoryManagement!!!!
void EasyStorageMapEntry <std::string,AstAttribute*> :: storeDataInEasyStorageClass (const std::string& name, AstAttribute* attr)
   {
   // call suitable methods of parent and member
      mapString.storeDataInEasyStorageClass(name);
      attributeData.storeDataInEasyStorageClass(attr->packed_data(), attr->packed_size() );
      attributeName.storeDataInEasyStorageClass( attr->attribute_class_name() );
   }


std::pair<std::string, AstAttribute*>
EasyStorageMapEntry <std::string,AstAttribute*> :: rebuildDataStoredInEasyStorageClass() const
   {
   // call suitable methods of parent and member
      AstAttribute *ret = NULL ;
   // register of the userdefined AstAttribute must be done before reading in. It will print an  error message, that contains
   // name of the attribute and some info how to register it
   // stored in the right manner but, before rebuilding it has to be registered!
      std :: map < std::string, AST_FILE_IO::CONSTRUCTOR > regAttr = AST_FILE_IO :: getRegisteredAttributes();
      std::string storedAttributeName = attributeName.rebuildDataStoredInEasyStorageClass();
      if ( regAttr.find ( storedAttributeName ) != regAttr.end() )
         {
            assert(ret != NULL); // but how could ret be non-NULL?
            ret = (ret->*regAttr[ storedAttributeName ]) ();
            assert(ret != NULL);
            ret->unpacked_data( attributeData.getSizeOfData(), attributeData.rebuildDataStoredInEasyStorageClass() );
         }
      else
         {
            std::cerr << "Attribute " << storedAttributeName << " not found registered, building default AstAttribute instead ... " << std::endl;
            ret = (ret->*regAttr["AstAttribute"]) ();
         }
      std::pair<std::string, AstAttribute*> return_pair(mapString.rebuildDataStoredInEasyStorageClass(), ret );
      return return_pair;
   }

void EasyStorageMapEntry <std::string,AstAttribute*> :: arrangeMemoryPoolInOneBlock()
   {
  // EasyStorage <std::string> occurs twice, but is only called once!!!!!!!!
     EasyStorage <std::string>  :: arrangeMemoryPoolInOneBlock();
     EasyStorage <char*>  :: arrangeMemoryPoolInOneBlock();

   }


void EasyStorageMapEntry <std::string,AstAttribute*> :: deleteMemoryPool()
   {
  // EasyStorage <std::string> occurs twice, but is only called once!!!!!!!!
     EasyStorage <std::string>  :: deleteMemoryPool();
     EasyStorage <char*>  :: deleteMemoryPool();

   }


void EasyStorageMapEntry <std::string,AstAttribute*> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|07|",outputFileStream);
#endif
  // EasyStorage <std::string> occurs twice, but is only called once!!!!!!!!
     EasyStorage <std::string>  :: writeToFile(outputFileStream);
     EasyStorage <char*>  :: writeToFile(outputFileStream);

   }


void EasyStorageMapEntry <std::string,AstAttribute*> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|07|",inputFileStream);
#endif
  // EasyStorage <std::string> occurs twice, but is only called once!!!!!!!!
     EasyStorage <std::string>  :: readFromFile (inputFileStream);
     EasyStorage <char*>  :: readFromFile (inputFileStream);

   }


/*
   ****************************************************************************************
   **      Implementations for EasyStorage <AstAttributeMechanism*>                      **
   ****************************************************************************************
*/
void EasyStorage <AstAttributeMechanism*> :: storeDataInEasyStorageClass(AstAttributeMechanism* data_)
   {
  // REMARK !!!!! Here we store a pointer that was a NULL pointer in a manner, that it
  // can be rebuilt (sizeOfData = - 1)
     if ( data_ == NULL )
        {
          Base::sizeOfData = -1;
          Base::positionInStaticMemoryPool = 0;
        }
     else
        {
      // get starting data
          std::set<std::string> first_data = data_->getAttributeIdentifiers();
          std::set<std::string>::iterator copy_ = first_data.begin();
          long offset = Base::setPositionAndSizeAndReturnOffset ( first_data.size() ) ;
       // if the new data does not fit in the actual block
          if (0 < offset)
             {
            // if there is still space in the actual block
               if ( offset < Base::getSizeOfData() && Base::actual != NULL )
                  {
                    for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++copy_)
                       {
                         Base::actual->storeDataInEasyStorageClass( *copy_ , (*data_)[*copy_]);
                       }
                  }
            // the data does not fit in one block
               while (Base::blockSize < (unsigned long)(offset))
                  {
                    Base::actual = Base::getNewMemoryBlock();
                    for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++copy_)
                       {
                         Base::actual->storeDataInEasyStorageClass(*copy_, (*data_)[*copy_]);
                       }
                    offset -= Base::blockSize;
                  }
            // get a new memory block (since the actual one is full)
               Base::actual = Base::getNewMemoryBlock();
             }
       // store the (rest of the) data
          for ( ; copy_ != first_data.end(); ++copy_, ++Base::actual )
             {
               Base::actual->storeDataInEasyStorageClass(*copy_, (*data_)[*copy_]);
             }
        }
   }


AstAttributeMechanism* EasyStorage <AstAttributeMechanism*> :: rebuildDataStoredInEasyStorageClass() const {
    AstAttributeMechanism* return_attribute{nullptr};
    // check whether data was not a NULL pointer ...
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
    ASSERT_require(Base::actualBlock <= 1);
    ASSERT_require((0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0));
#endif
    if (Base::getSizeOfData() != -1) {
        // if the memory pool is valid
        ASSERT_require(Base::actualBlock <= 1);
        if (Base::actual != nullptr && 0 < Base::getSizeOfData()) {
            ASSERT_require( Base::actualBlock == 1);
            return_attribute = new AstAttributeMechanism;
            EasyStorageMapEntry<std::string,AstAttribute*> *pointer = Base::getBeginningOfDataBlock();
            std::pair<std::string, AstAttribute*> entry;

            for (int i = 0; i < Base::getSizeOfData(); ++i) {
                entry = (pointer+i)->rebuildDataStoredInEasyStorageClass();
                return_attribute->add(entry.first, entry.second) ;
            }
        }
    }
    return return_attribute;
}

void EasyStorage <AstAttributeMechanism*> :: arrangeMemoryPoolInOneBlock()
   {
  // call suitable methods of parent and members
     StorageClassMemoryManagement < EasyStorageMapEntry<std::string,AstAttribute*>> :: arrangeMemoryPoolInOneBlock();
     EasyStorageMapEntry<std::string,AstAttribute*> :: arrangeMemoryPoolInOneBlock();
   }


void EasyStorage <AstAttributeMechanism*> :: deleteMemoryPool()
   {
  // call suitable methods of parent and members
     StorageClassMemoryManagement < EasyStorageMapEntry<std::string,AstAttribute*>> :: deleteMemoryPool();
     EasyStorageMapEntry<std::string,AstAttribute*> :: deleteMemoryPool();
   }


void EasyStorage <AstAttributeMechanism*> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|08|",outputFileStream);
#endif
  // call suitable methods of parent and members
     StorageClassMemoryManagement < EasyStorageMapEntry<std::string,AstAttribute*>> :: writeToFile(outputFileStream);
     EasyStorageMapEntry<std::string,AstAttribute*> :: writeToFile(outputFileStream);
   }


void EasyStorage <AstAttributeMechanism*> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|08|",inputFileStream);
#endif
  // call suitable methods of parent and members
     StorageClassMemoryManagement < EasyStorageMapEntry<std::string,AstAttribute*>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry<std::string,AstAttribute*> :: readFromFile (inputFileStream);
   }


/*
   ****************************************************************************************
   **      Implementations for EasyStorage <PreprocessingInfo*>                          **
   ****************************************************************************************
*/

#define DEBUG_EasyStorage_PreprocessingInfo_storeDataInEasyStorageClass 0

void EasyStorage<PreprocessingInfo*>::storeDataInEasyStorageClass(PreprocessingInfo* info) {
#if DEBUG_EasyStorage_PreprocessingInfo_storeDataInEasyStorageClass
  printf ("EasyStorage<PreprocessingInfo*>::storeDataInEasyStorageClass \n");
#endif

     ROSE_ASSERT(info != NULL);
     ROSE_ASSERT(info->get_file_info() != NULL);

#if DEBUG_EasyStorage_PreprocessingInfo_storeDataInEasyStorageClass
     printf ("In EasyStorage<PreprocessingInfo*>::storeDataInEasyStorageClass(): info->get_file_info() = %p \n",info->get_file_info());
     printf ("In EasyStorage<PreprocessingInfo*>::storeDataInEasyStorageClass(): info->get_file_info()->get_freepointer() = %p \n",info->get_file_info()->get_freepointer());
#endif

     fileInfoIndex = AST_FILE_IO::getGlobalIndexFromSgClassPointer(info->get_file_info());
#if DEBUG_EasyStorage_PreprocessingInfo_storeDataInEasyStorageClass
     printf ("Saving fileInfoIndex = %d for %p \n",fileInfoIndex, info->get_file_info());
#endif

  // get changeable pointer
     char* copy_ = info->packed();
     long offset = Base::setPositionAndSizeAndReturnOffset ( info->packed_size() ) ;
  // if the new data does not fit in the actual block
     if (0 < offset)
        {
       // if there is still space in the actual block
          if ( offset < Base::getSizeOfData()   && Base::actual != NULL )
             {
               memcpy( Base::actual, copy_, (Base::getSizeOfData()-offset)*sizeof(char) );
               copy_ += (Base::getSizeOfData()-offset);
             }
      // the data does not fit in one block
          while (Base::blockSize < (unsigned long)(offset))
             {
               Base::actual = Base::getNewMemoryBlock();
               memcpy(Base::actual, copy_, Base::blockSize*sizeof(char) );
               offset -= Base::blockSize;
               copy_ += Base::blockSize;
             }
       // put the rest of the data in a new memory block
          Base::actual = Base::getNewMemoryBlock();
          memcpy(Base::actual, copy_, offset*sizeof(char) );
          Base::actual += offset;
        }
     else
        {
       // put the the data in the memory block
          memcpy(Base::actual, copy_, Base::getSizeOfData()*sizeof(char) );
          Base::actual += Base::getSizeOfData();
        }
   }

#define DEBUG_EasyStorage_PreprocessingInfo_rebuildDataStoredInEasyStorageClass 0

PreprocessingInfo* EasyStorage<PreprocessingInfo*>::rebuildDataStoredInEasyStorageClass() const {
#if DEBUG_EasyStorage_PreprocessingInfo_rebuildDataStoredInEasyStorageClass
  printf ("EasyStorage<PreprocessingInfo*>::rebuildDataStoredInEasyStorageClass \n");
#endif

     ASSERT_this();
     PreprocessingInfo* returnInfo  = NULL;
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
     assert ( Base::actualBlock <= 1 );
     assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0 ) );
#endif
     if ( 0 < Base::getSizeOfData() )
         {
        // DQ (4/22/2006): This constructor can not build any IR node or it appears
        // that the memory pool will become corrupted resulting the a bad AST.
           returnInfo = new PreprocessingInfo();
           ROSE_ASSERT(returnInfo != NULL);

        // ROSE-1470
           Sg_File_Info * file_info = (Sg_File_Info*)(AST_FILE_IO::getSgClassPointerFromGlobalIndex(fileInfoIndex));
           returnInfo->set_file_info(file_info);

        // if there is any data in the pool at all
           if ( Base::actual != NULL  && 0 < Base::getSizeOfData() )
              {
                char* data_ = new char[Base::getSizeOfData()];
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
                assert ( Base::memoryBlockList != NULL );
                assert ( Base::actualBlock == 1 );
#endif
                memcpy(data_, Base::getBeginningOfDataBlock(), Base::getSizeOfData() * sizeof(char) );
                returnInfo->unpacked( data_ );
              }

           ROSE_ASSERT(returnInfo != NULL);
         }

     return returnInfo;
   }

void EasyStorage <PreprocessingInfo*> :: arrangeMemoryPoolInOneBlock()
   {
  // call suitable methods of parent and members
     StorageClassMemoryManagement < char > :: arrangeMemoryPoolInOneBlock();
   }


void EasyStorage <PreprocessingInfo*> :: deleteMemoryPool()
   {
  // call suitable methods of parent and members
     StorageClassMemoryManagement < char > :: deleteMemoryPool();
   }


void EasyStorage <PreprocessingInfo*> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|09|",outputFileStream);
#endif
  // call suitable methods of parent and members
     StorageClassMemoryManagement < char > :: writeToFile(outputFileStream);
   }


void EasyStorage <PreprocessingInfo*> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|09|",inputFileStream);
#endif
  // call suitable methods of parent and members
     StorageClassMemoryManagement < char > :: readFromFile (inputFileStream);
   }



/*
   ****************************************************************************************
   **       Implementations for asyStorage <CONTAINER<PreprocessingInfo*>>              **
   ****************************************************************************************
*/
template <template <class A> class CONTAINER >
void EasyStorage <CONTAINER<PreprocessingInfo*>> :: storeDataInEasyStorageClass(const CONTAINER<PreprocessingInfo*>& data_)
   {
  // get iterator pointing to begin
     typename CONTAINER<PreprocessingInfo*>::const_iterator copy_ = data_.begin();
     long offset = setPositionAndSizeAndReturnOffset ( data_.size() ) ;
  // if the new data does not fit in the actual block
     if (0 < offset)
        {
       // if there is still space in the actual block
          if ( offset < Base::getSizeOfData() && Base::actual != NULL )
             {
               for (;(unsigned long)( Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++copy_)
                  {
                    Base::actual->storeDataInEasyStorageClass(*copy_);
                  }
             }
      // the data does not fit in one block
          while (Base::blockSize < (unsigned long)(offset))
             {
               Base::actual = Base::getNewMemoryBlock();
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++copy_)
                  {
                    Base::actual->storeDataInEasyStorageClass(*copy_);
                  }
               offset -= Base::blockSize;
             }
       // get a new memory block
          Base::actual = Base::getNewMemoryBlock();
        }
  // put (the rest of) the data in a new memory block
    for ( ; copy_ != data_.end(); ++copy_, ++Base::actual )
        {
          Base::actual->storeDataInEasyStorageClass(*copy_);
        }
   }


template <template <class A> class CONTAINER >
CONTAINER<PreprocessingInfo*>
EasyStorage <CONTAINER<PreprocessingInfo*>> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() == 0 ) );
#endif
      CONTAINER<PreprocessingInfo*> data_;
   // if there is any data in the pool
      if ( Base::actual != NULL  && 0 < Base::getSizeOfData() )
         {
           EasyStorage<PreprocessingInfo*> *pointer = Base::getBeginningOfDataBlock() ;
           for ( long i=0; i < Base::getSizeOfData(); ++i )
              {
                data_.push_back( (pointer+i)->rebuildDataStoredInEasyStorageClass() );
              }
         }
      return data_;
   }

template <template <class A> class CONTAINER >
void EasyStorage <CONTAINER<PreprocessingInfo*>> :: arrangeMemoryPoolInOneBlock()
   {
    // calling the base class method
     StorageClassMemoryManagement<EasyStorage<PreprocessingInfo*>> :: arrangeMemoryPoolInOneBlock();
    // calling the basic type
     EasyStorage <PreprocessingInfo*> :: arrangeMemoryPoolInOneBlock();
   }

template <template <class A> class CONTAINER >
void EasyStorage <CONTAINER<PreprocessingInfo*>> :: deleteMemoryPool()
   {
    // calling the base class method
     StorageClassMemoryManagement<EasyStorage<PreprocessingInfo*>> :: deleteMemoryPool();
    // calling the basic type
     EasyStorage <PreprocessingInfo*> :: deleteMemoryPool();
   }


template <template <class A> class CONTAINER >
void EasyStorage <CONTAINER<PreprocessingInfo*>> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|10|",outputFileStream);
#endif
    // calling the base class method
     StorageClassMemoryManagement<EasyStorage<PreprocessingInfo*>> :: writeToFile(outputFileStream);
    // calling the basic type
     EasyStorage <PreprocessingInfo*> ::  writeToFile(outputFileStream);
   }

template <template <class A> class CONTAINER >
void EasyStorage <CONTAINER<PreprocessingInfo*>> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|10|",inputFileStream);
#endif
    // calling the base class method
     StorageClassMemoryManagement<EasyStorage<PreprocessingInfo*>> :: readFromFile (inputFileStream);
    // calling the basic type
     EasyStorage <PreprocessingInfo*> ::  readFromFile (inputFileStream);
   }

/* special implementation for  omitting compiler instantiation errors*/
void EasyStorage <std::vector<PreprocessingInfo*>> :: storeDataInEasyStorageClass(const std::vector<PreprocessingInfo*>& data_)
   {
  // get iterator pointing to begin
     std::vector<PreprocessingInfo*>::const_iterator copy_ = data_.begin();
     long offset = setPositionAndSizeAndReturnOffset ( data_.size() ) ;
  // if the new data does not fit in the actual block
     if (0 < offset)
        {
       // if there is still space in the actual block
          if ( offset < Base::getSizeOfData() && Base::actual != NULL )
             {
               for (;(unsigned long)( Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++copy_)
                  {
                    Base::actual->storeDataInEasyStorageClass(*copy_);
                  }
             }
      // the data does not fit in one block
          while (Base::blockSize < (unsigned long)(offset))
             {
               Base::actual = Base::getNewMemoryBlock();
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++copy_)
                  {
                    Base::actual->storeDataInEasyStorageClass(*copy_);
                  }
               offset -= Base::blockSize;
             }
       // get a new memory block
          Base::actual = Base::getNewMemoryBlock();
        }
  // put (the rest of) the data in a new memory block
    for ( ; copy_ != data_.end(); ++copy_, ++Base::actual )
        {
          Base::actual->storeDataInEasyStorageClass(*copy_);
        }
   }


std::vector<PreprocessingInfo*>
EasyStorage <std::vector<PreprocessingInfo*>> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() == 0 ) );
#endif
      std::vector<PreprocessingInfo*> data_;
   // if there is any data in the pool
      if ( Base::actual != NULL  && 0 < Base::getSizeOfData() )
         {
           EasyStorage<PreprocessingInfo*> *pointer = Base::getBeginningOfDataBlock() ;
           for ( long i=0; i < Base::getSizeOfData(); ++i )
              {
                data_.push_back( (pointer+i)->rebuildDataStoredInEasyStorageClass() );
              }
         }
      return data_;
   }

void EasyStorage <std::vector<PreprocessingInfo*>> :: arrangeMemoryPoolInOneBlock()
   {
    // calling the base class method
     StorageClassMemoryManagement<EasyStorage<PreprocessingInfo*>> :: arrangeMemoryPoolInOneBlock();
    // calling the basic type
     EasyStorage <PreprocessingInfo*> :: arrangeMemoryPoolInOneBlock();
   }

void EasyStorage <std::vector<PreprocessingInfo*>> :: deleteMemoryPool()
   {
    // calling the base class method
     StorageClassMemoryManagement<EasyStorage<PreprocessingInfo*>> :: deleteMemoryPool();
    // calling the basic type
     EasyStorage <PreprocessingInfo*> :: deleteMemoryPool();
   }


void EasyStorage <std::vector<PreprocessingInfo*>> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|11|",outputFileStream);
#endif
    // calling the base class method
     StorageClassMemoryManagement<EasyStorage<PreprocessingInfo*>> :: writeToFile(outputFileStream);
    // calling the basic type
     EasyStorage <PreprocessingInfo*> ::  writeToFile(outputFileStream);
   }

void EasyStorage <std::vector<PreprocessingInfo*>> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|11|",inputFileStream);
#endif
    // calling the base class method
     StorageClassMemoryManagement<EasyStorage<PreprocessingInfo*>> :: readFromFile (inputFileStream);
    // calling the basic type
     EasyStorage <PreprocessingInfo*> ::  readFromFile (inputFileStream);
   }

void EasyStorage <AttachedPreprocessingInfoType*>::displayEasyStorageData()
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
     assert ( Base::memoryBlockList != NULL );
#endif

     printf ("positionInStaticMemoryPool = %lu \n",positionInStaticMemoryPool);
     printf ("sizeOfData                 = %ld \n",sizeOfData);
     printf ("(static) filledUpTo        = %lu \n",filledUpTo);
     printf ("(static) blockSize         = %lu \n",blockSize);
     printf ("(static) actualBlock       = %u \n",actualBlock);
     printf ("(static) blocksAllocated   = %u \n",blocksAllocated);
     printf ("(static) memoryBlockList   = %p \n",memoryBlockList);
     printf ("(static) actual            = %p \n",actual);

     printf ("Iterating over the objects in the  Base::memoryBlockList size = %ld \n",Base::getSizeOfData());

  // DQ (8/24/2006): Fixed compiler warning
  // for (unsigned long i = 0; i < Base::getSizeOfData(); ++i)
     for (long i = 0; i < Base::getSizeOfData(); ++i)
        {
          long offset = (Base::positionInStaticMemoryPool+i)%Base::blockSize;
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
          assert ( Base::memoryBlockList[(Base::positionInStaticMemoryPool+i-offset)/Base::blockSize] != NULL );
#endif
       // Base::memoryBlockList[(Base::positionInStaticMemoryPool+i-offset)/Base::blockSize][offset].print();
       // Base::memoryBlockList[(Base::positionInStaticMemoryPool+i-offset)/Base::blockSize][offset].displayEasyStorageData();
          Base::memoryBlockList[(Base::positionInStaticMemoryPool+i-offset)/Base::blockSize][offset].displayStorageClassData();
          std::cout << std::endl;
        }
   }


/* special implementation for omitting compiler instantiation errors*/
void EasyStorage <AttachedPreprocessingInfoType*> :: storeDataInEasyStorageClass(AttachedPreprocessingInfoType* data_)
   {
//     printf ("EasyStorage <AttachedPreprocessingInfoType*> :: storeDataInEasyStorageClass \n");
     if (data_ == NULL)
        {
          Base::sizeOfData = -1;
        }
     else
        {
       // get iterator pointing to begin
       // AttachedPreprocessingInfoType::list<PreprocessingInfo*>::const_iterator copy_ = data_->begin();
       // Rose_STL_Container<PreprocessingInfo*>::const_iterator copy_ = data_->begin();
          AttachedPreprocessingInfoType::const_iterator copy_ = data_->begin();
          long offset = setPositionAndSizeAndReturnOffset ( data_->size() );

       // printf ("In storeDataInEasyStorageClass(): offset = %ld Base::blockSize = %ld Base::memoryBlockList = %p \n",offset,Base::blockSize,Base::memoryBlockList);
       // Can't call print here! Base::getNewMemoryBlock() needs to be called first.
       // print();

       // if the new data does not fit in the actual block
          if (0 < offset)
             {
            // if there is still space in the actual block
               if ( offset < Base::getSizeOfData() && Base::actual != NULL )
                  {
                    for (;(unsigned long)( Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++copy_)
                       {
                         Base::actual->storeDataInEasyStorageClass(*copy_);
                       }
                  }
           // the data does not fit in one block
               while (Base::blockSize < (unsigned long)(offset))
                  {
                    Base::actual = Base::getNewMemoryBlock();
                    for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++copy_)
                       {
                         Base::actual->storeDataInEasyStorageClass(*copy_);
                       }
                    offset -= Base::blockSize;
                  }
            // get a new memory block
               Base::actual = Base::getNewMemoryBlock();
             }

       // printf ("In storeDataInEasyStorageClass(): copy the rest of the data offset = %ld \n",offset);
       // displayEasyStorageData();

       // put (the rest of) the data in a new memory block
          for ( ; copy_ != data_->end(); ++copy_, ++Base::actual )
             {
               Base::actual->storeDataInEasyStorageClass(*copy_);
             }
        }
   }


AttachedPreprocessingInfoType*
EasyStorage <AttachedPreprocessingInfoType*> :: rebuildDataStoredInEasyStorageClass() const
   {
//      printf ("EasyStorage <AttachedPreprocessingInfoType*> :: rebuildDataStoredInEasyStorageClass \n");
      AttachedPreprocessingInfoType* data_ = NULL;
      if ( Base::getSizeOfData() != -1 )
         {
           data_ = new AttachedPreprocessingInfoType;
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
           assert ( Base::actualBlock <= 1 );
           assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() == 0 ) );
#endif
        // if there is any data in the pool
           if ( Base::actual != NULL  && 0 < Base::getSizeOfData() )
              {
                EasyStorage<PreprocessingInfo*> *pointer = Base::getBeginningOfDataBlock() ;
                for ( long i=0; i < Base::getSizeOfData(); ++i )
                   {
                     data_->push_back( (pointer+i)->rebuildDataStoredInEasyStorageClass() );
                   }
              }
         }
      return data_;
   }

void EasyStorage <AttachedPreprocessingInfoType*> :: arrangeMemoryPoolInOneBlock()
   {
    // calling the base class method
     StorageClassMemoryManagement<EasyStorage<PreprocessingInfo*>> :: arrangeMemoryPoolInOneBlock();
    // calling the basic type
     EasyStorage <PreprocessingInfo*> :: arrangeMemoryPoolInOneBlock();
   }

void EasyStorage <AttachedPreprocessingInfoType*> :: deleteMemoryPool()
   {
    // calling the base class method
     StorageClassMemoryManagement<EasyStorage<PreprocessingInfo*>> :: deleteMemoryPool();
    // calling the basic type
     EasyStorage <PreprocessingInfo*> :: deleteMemoryPool();
   }


void EasyStorage <AttachedPreprocessingInfoType*> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|12|",outputFileStream);
#endif
    // calling the base class method
     StorageClassMemoryManagement<EasyStorage<PreprocessingInfo*>> :: writeToFile(outputFileStream);
    // calling the basic type
     EasyStorage <PreprocessingInfo*> ::  writeToFile(outputFileStream);
   }

void EasyStorage <AttachedPreprocessingInfoType*> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|12|",inputFileStream);
#endif
    // calling the base class method
     StorageClassMemoryManagement<EasyStorage<PreprocessingInfo*>> :: readFromFile (inputFileStream);
    // calling the basic type
     EasyStorage <PreprocessingInfo*> ::  readFromFile (inputFileStream);
   }

/*
   ****************************************************************************************
   **      Implementations for EasyStorage <ROSEAttributesList>                          **
   ****************************************************************************************
*/
void EasyStorage<ROSEAttributesList> :: storeDataInEasyStorageClass ( ROSEAttributesList* info)
   {
  // call suitable methods on members
     preprocessingInfoVector.storeDataInEasyStorageClass( info->getList() );
     fileNameString.storeDataInEasyStorageClass( info->getFileName() );
     index = info->getIndex() ;
   }


ROSEAttributesList* EasyStorage<ROSEAttributesList> :: rebuildDataStoredInEasyStorageClass() const
   {
  // call suitable methods on members
     ROSEAttributesList* returnInfo = new ROSEAttributesList();
     returnInfo->getList() = preprocessingInfoVector.rebuildDataStoredInEasyStorageClass() ;
     returnInfo->setFileName( fileNameString.rebuildDataStoredInEasyStorageClass() ) ;
     returnInfo->setIndex ( index ) ;
     return returnInfo;
   }


void EasyStorage<ROSEAttributesList> :: arrangeMemoryPoolInOneBlock()
   {
  // call suitable methods on members
      EasyStorage < std::vector<PreprocessingInfo*>> :: arrangeMemoryPoolInOneBlock();
      EasyStorage < std::string > :: arrangeMemoryPoolInOneBlock();
   }


void EasyStorage<ROSEAttributesList> :: deleteMemoryPool()
   {
  // call suitable methods on members
     EasyStorage < std::vector<PreprocessingInfo*>> :: deleteMemoryPool();
     EasyStorage < std::string > :: deleteMemoryPool();
   }


void EasyStorage<ROSEAttributesList> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|13|",outputFileStream);
#endif
  // call suitable methods on members
      EasyStorage < std::vector<PreprocessingInfo*>> :: writeToFile(outputFileStream);
      EasyStorage < std::string > :: writeToFile(outputFileStream);
   }


void EasyStorage<ROSEAttributesList> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|13|",inputFileStream);
#endif
  // call suitable methods on members
     EasyStorage < std::vector<PreprocessingInfo*>> :: readFromFile (inputFileStream);
     EasyStorage < std::string > :: readFromFile (inputFileStream);
   }

/*
   ****************************************************************************************
   **      Implementations for <EasyStorageMapEntry <std::string,ROSEAttributesList*>    **
   ****************************************************************************************
*/
// Is not inherited from StorageClassMemoryManagement
void EasyStorageMapEntry <std::string,ROSEAttributesList> :: storeDataInEasyStorageClass(const std::pair<std::string, ROSEAttributesList*>& iter)
   {
     nameString.storeDataInEasyStorageClass(iter.first);
     attributesList.storeDataInEasyStorageClass(iter.second);
   }

std::pair<std::string, ROSEAttributesList*> EasyStorageMapEntry <std::string,ROSEAttributesList> :: rebuildDataStoredInEasyStorageClass() const
   {
     std::pair<std::string,ROSEAttributesList*> returnPair(nameString.rebuildDataStoredInEasyStorageClass(),
                                                           attributesList.rebuildDataStoredInEasyStorageClass());
     return returnPair;
   }

void EasyStorageMapEntry <std::string,ROSEAttributesList> :: arrangeMemoryPoolInOneBlock()
   {
     EasyStorage <std::string> :: arrangeMemoryPoolInOneBlock();
     EasyStorage <ROSEAttributesList> :: arrangeMemoryPoolInOneBlock();
   }

void EasyStorageMapEntry <std::string,ROSEAttributesList> :: deleteMemoryPool()
   {
     EasyStorage <std::string> :: deleteMemoryPool();
     EasyStorage <ROSEAttributesList> :: deleteMemoryPool();
   }


void EasyStorageMapEntry <std::string,ROSEAttributesList> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|15|",outputFileStream);
#endif
     EasyStorage <std::string> :: writeToFile(outputFileStream);
     EasyStorage <ROSEAttributesList> :: writeToFile(outputFileStream);
   }

void EasyStorageMapEntry <std::string,ROSEAttributesList> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|15|",inputFileStream);
#endif
     EasyStorage <std::string> :: readFromFile (inputFileStream);
     EasyStorage <ROSEAttributesList> :: readFromFile (inputFileStream);
   }

#if 1
/*
   ****************************************************************************************
   **      Implementations for EasyStorage ROSEAttributesListContainerPtr>               **
   ****************************************************************************************
*/
void EasyStorage <ROSEAttributesListContainerPtr> :: storeDataInEasyStorageClass(ROSEAttributesListContainerPtr data_)
   {
     if (data_ == NULL)
        {
          Base::sizeOfData = -1;
        }
     else
        {
          std::map<std::string,ROSEAttributesList*> data_vec = data_->getList();
          std::map<std::string,ROSEAttributesList*>::const_iterator dat = data_vec.begin();
          long offset = Base::setPositionAndSizeAndReturnOffset ( data_vec.size() ) ;
       // if the new data does not fit in the actual block
          if (0 < offset)
             {
            // if there is still space in the actual block
               if (offset < Base::getSizeOfData())
                  {
                    if (Base::actual != NULL)
                       {
                         for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                            {
                              Base::actual->storeDataInEasyStorageClass(*dat);
                            }
                       }
                  }
            // the data does not fit in one block
               while (Base::blockSize < (unsigned long)(offset))
                  {
                    Base::actual = Base::getNewMemoryBlock();
                    for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                       {
                         Base::actual->storeDataInEasyStorageClass(*dat);
                       }
                    offset -= Base::blockSize;
                  };
               Base::actual = Base::getNewMemoryBlock();
             }
          for (; dat != data_vec.end(); ++dat, ++Base::actual)
             {
               Base::actual->storeDataInEasyStorageClass(*dat);
             }
        }
   }


ROSEAttributesListContainerPtr
EasyStorage <ROSEAttributesListContainerPtr> :: rebuildDataStoredInEasyStorageClass() const
   {
     ROSEAttributesListContainerPtr returnPtr = NULL;
     if (Base::getSizeOfData() != -1 )
        {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
          assert ( Base::actualBlock <= 1 );
          assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0 ) );
#endif
          returnPtr = new ROSEAttributesListContainer;
          std::map<std::string,ROSEAttributesList*> data_;
       // if the memory pool is valid
          if ( Base::actual != NULL && 0 < Base::getSizeOfData() )
             {
               std::pair<std::string,ROSEAttributesList*> tempPair;
               EasyStorageMapEntry<std::string,ROSEAttributesList>* pointer = Base::getBeginningOfDataBlock();
               for ( long i=0; i < Base::getSizeOfData(); ++i )
                  {
                    assert (Base::actualBlock == 1);
                    tempPair = (pointer+i)->rebuildDataStoredInEasyStorageClass();
                    data_[tempPair.first] = tempPair.second;
                  }
             }
           returnPtr->getList() = data_;
        }
      return returnPtr;
   }


void EasyStorage <ROSEAttributesListContainerPtr> :: arrangeMemoryPoolInOneBlock()
   {
  // call suitable methods on members and base class
      StorageClassMemoryManagement<EasyStorageMapEntry<std::string,ROSEAttributesList>>:: arrangeMemoryPoolInOneBlock();
      EasyStorageMapEntry<std::string,ROSEAttributesList> :: arrangeMemoryPoolInOneBlock();
   }


void EasyStorage <ROSEAttributesListContainerPtr> :: deleteMemoryPool()
   {
  // call suitable methods on members and base class
     StorageClassMemoryManagement<EasyStorageMapEntry<std::string,ROSEAttributesList>> :: deleteMemoryPool();
      EasyStorageMapEntry<std::string,ROSEAttributesList> :: deleteMemoryPool();
   }


void EasyStorage <ROSEAttributesListContainerPtr> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|16|",outputFileStream);
#endif
  // call suitable methods on members and base class
      StorageClassMemoryManagement<EasyStorageMapEntry<std::string,ROSEAttributesList>>:: writeToFile(outputFileStream);
      EasyStorageMapEntry<std::string,ROSEAttributesList> :: writeToFile(outputFileStream);
   }


void EasyStorage <ROSEAttributesListContainerPtr> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|16|",inputFileStream);
#endif
  // call suitable methods on members and base class
     StorageClassMemoryManagement<EasyStorageMapEntry<std::string,ROSEAttributesList>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry<std::string,ROSEAttributesList> :: readFromFile (inputFileStream);
   }
#endif

/*
   ****************************************************************************************************
   **      Specialized implementation for EasyStorage <std::set<PreprocessingInfo*>> needed         **
   **      to overcome compiler's template instantiation errors.                                     **
   ****************************************************************************************************
*/

void EasyStorage <std::set<PreprocessingInfo*>> :: storeDataInEasyStorageClass(const std::set<PreprocessingInfo*>& data)
   {
  // get an iterator pointing to the beginning
     std::set<PreprocessingInfo*>::const_iterator dataIterator = data.begin();
     long offset = setPositionAndSizeAndReturnOffset ( data.size() ) ;
  // if the new data does not fit in the actual block
     if (0 < offset)
        {
       // if there is still space in the actual block
          if ( offset < Base::getSizeOfData() && Base::actual != NULL )
             {
               for (;(unsigned long)( Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dataIterator)
                  {
                    Base::actual->storeDataInEasyStorageClass(*dataIterator);
                  }
             }
      // the data does not fit in one block
          while (Base::blockSize < (unsigned long)(offset))
             {
               Base::actual = Base::getNewMemoryBlock();
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dataIterator)
                  {
                    Base::actual->storeDataInEasyStorageClass(*dataIterator);
                  }
               offset -= Base::blockSize;
             }
       // get a new memory block
          Base::actual = Base::getNewMemoryBlock();
        }
  // put (the rest of) the data in a new memory block
    for ( ; dataIterator != data.end(); ++dataIterator, ++Base::actual )
        {
          Base::actual->storeDataInEasyStorageClass(*dataIterator);
        }
   }


std::set<PreprocessingInfo*>
EasyStorage <std::set<PreprocessingInfo*>> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() == 0 ) );
#endif
      std::set<PreprocessingInfo*> data;
   // if there is any data in the pool
      if ( Base::actual != NULL  && 0 < Base::getSizeOfData() )
         {
           EasyStorage<PreprocessingInfo*> *pointer = Base::getBeginningOfDataBlock() ;
           for ( long i=0; i < Base::getSizeOfData(); ++i )
              {
                data.insert((pointer+i)->rebuildDataStoredInEasyStorageClass());
              }
         }
      return data;
   }

void EasyStorage <std::set<PreprocessingInfo*>> :: arrangeMemoryPoolInOneBlock()
   {
    // calling the base class method
     StorageClassMemoryManagement<EasyStorage<PreprocessingInfo*>> :: arrangeMemoryPoolInOneBlock();
    // calling the basic type
     EasyStorage <PreprocessingInfo*> :: arrangeMemoryPoolInOneBlock();
   }

void EasyStorage <std::set<PreprocessingInfo*>> :: deleteMemoryPool()
   {
    // calling the base class method
     StorageClassMemoryManagement<EasyStorage<PreprocessingInfo*>> :: deleteMemoryPool();
    // calling the basic type
     EasyStorage <PreprocessingInfo*> :: deleteMemoryPool();
   }


void EasyStorage <std::set<PreprocessingInfo*>> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|32|",outputFileStream);
#endif
    // calling the base class method
     StorageClassMemoryManagement<EasyStorage<PreprocessingInfo*>> :: writeToFile(outputFileStream);
    // calling the basic type
     EasyStorage <PreprocessingInfo*> ::  writeToFile(outputFileStream);
   }

void EasyStorage <std::set<PreprocessingInfo*>> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|32|",inputFileStream);
#endif
    // calling the base class method
     StorageClassMemoryManagement<EasyStorage<PreprocessingInfo*>> :: readFromFile (inputFileStream);
    // calling the basic type
     EasyStorage <PreprocessingInfo*> ::  readFromFile (inputFileStream);
   }

/*
   ****************************************************************************************************
   **      Implementations for EasyStorageMapEntry <std::string, std::set<PreprocessingInfo*>>      **
   ****************************************************************************************************
*/
// Is not inherited from StorageClassMemoryManagement
void EasyStorageMapEntry <std::string, std::set<PreprocessingInfo*>> :: storeDataInEasyStorageClass(const std::pair<std::string, std::set<PreprocessingInfo*>>& iter)
   {
     nameString.storeDataInEasyStorageClass(iter.first);
     preprocessingInfoSet.storeDataInEasyStorageClass(iter.second);
   }

std::pair<std::string, std::set<PreprocessingInfo*>> EasyStorageMapEntry <std::string, std::set<PreprocessingInfo*>> :: rebuildDataStoredInEasyStorageClass() const
   {
     std::pair<std::string, std::set<PreprocessingInfo*>> returnPair(nameString.rebuildDataStoredInEasyStorageClass(),
                                                                      preprocessingInfoSet.rebuildDataStoredInEasyStorageClass());
     return returnPair;
   }

void EasyStorageMapEntry <std::string, std::set<PreprocessingInfo*>> :: arrangeMemoryPoolInOneBlock()
   {
     EasyStorage <std::string> :: arrangeMemoryPoolInOneBlock();
     EasyStorage <std::set<PreprocessingInfo*>> :: arrangeMemoryPoolInOneBlock();
   }

void EasyStorageMapEntry <std::string, std::set<PreprocessingInfo*>> :: deleteMemoryPool()
   {
     EasyStorage <std::string> :: deleteMemoryPool();
     EasyStorage <std::set<PreprocessingInfo*>> :: deleteMemoryPool();
   }


void EasyStorageMapEntry <std::string, std::set<PreprocessingInfo*>> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|33|",outputFileStream);
#endif
     EasyStorage <std::string> :: writeToFile(outputFileStream);
     EasyStorage <std::set<PreprocessingInfo*>> :: writeToFile(outputFileStream);
   }

void EasyStorageMapEntry <std::string, std::set<PreprocessingInfo*>> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|33|",inputFileStream);
#endif
     EasyStorage <std::string> :: readFromFile (inputFileStream);
     EasyStorage <std::set<PreprocessingInfo*>> :: readFromFile (inputFileStream);
   }

/*
   *************************************************************************************************************
   **      Implementations for EasyStorage <std::map<std::string, std::set<PreprocessingInfo*>>>            **
   *************************************************************************************************************
*/
void EasyStorage <std::map<std::string, std::set<PreprocessingInfo*>>> :: storeDataInEasyStorageClass(const std::map<std::string, std::set<PreprocessingInfo*>>& data)
   {
    if (data.empty()) {
      Base::sizeOfData = -1;
    } else {
      std::map<std::string, std::set<PreprocessingInfo*>>::const_iterator dataIterator = data.begin();
      long offset = Base::setPositionAndSizeAndReturnOffset ( data.size() ) ;
   // if the new data does not fit in the actual block
      if (0 < offset)
         {
        // if there is still space in the actual block
           if (offset < Base::getSizeOfData())
              {
                if (Base::actual != NULL)
                   {
                     for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dataIterator)
                        {
                          Base::actual->storeDataInEasyStorageClass(*dataIterator);
                        }
                   }
              }
        // the data does not fit in one block
           while (Base::blockSize < (unsigned long)(offset))
              {
                Base::actual = Base::getNewMemoryBlock();
                for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dataIterator)
                   {
                     Base::actual->storeDataInEasyStorageClass(*dataIterator);
                   }
                offset -= Base::blockSize;
              }
           Base::actual = Base::getNewMemoryBlock();
         }
      for (; dataIterator != data.end(); ++dataIterator, ++Base::actual)
         {
           Base::actual->storeDataInEasyStorageClass(*dataIterator);
         }
     }
   }


std::map<std::string, std::set<PreprocessingInfo*>>
EasyStorage <std::map<std::string, std::set<PreprocessingInfo*>>> :: rebuildDataStoredInEasyStorageClass() const
   {
     std::map<std::string, std::set<PreprocessingInfo*>> returnMap;
     if (Base::getSizeOfData() != -1 )
        {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
          assert ( Base::actualBlock <= 1 );
          assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0 ) );
#endif
       // if the memory pool is valid
          if ( Base::actual != NULL && 0 < Base::getSizeOfData() )
             {
               std::pair<std::string, std::set<PreprocessingInfo*>> tempPair;
               EasyStorageMapEntry<std::string, std::set<PreprocessingInfo*>>* pointer = Base::getBeginningOfDataBlock();
               for ( long i=0; i < Base::getSizeOfData(); ++i )
                  {
                    assert (Base::actualBlock == 1);
                    tempPair = (pointer+i) -> rebuildDataStoredInEasyStorageClass();
                    returnMap[tempPair.first] = tempPair.second;
                  }
             }
        }
      return returnMap;
   }


void EasyStorage <std::map<std::string, std::set<PreprocessingInfo*>>> :: arrangeMemoryPoolInOneBlock()
   {
  // call suitable methods on members and base class
      StorageClassMemoryManagement<EasyStorageMapEntry<std::string, std::set<PreprocessingInfo*>>>:: arrangeMemoryPoolInOneBlock();
      EasyStorageMapEntry<std::string, std::set<PreprocessingInfo*>> :: arrangeMemoryPoolInOneBlock();
   }


void EasyStorage <std::map<std::string, std::set<PreprocessingInfo*>>> :: deleteMemoryPool()
   {
  // call suitable methods on members and base class
      StorageClassMemoryManagement<EasyStorageMapEntry<std::string, std::set<PreprocessingInfo*>>> :: deleteMemoryPool();
      EasyStorageMapEntry<std::string, std::set<PreprocessingInfo*>> :: deleteMemoryPool();
   }


void EasyStorage <std::map<std::string, std::set<PreprocessingInfo*>>> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|34|",outputFileStream);
#endif
  // call suitable methods on members and base class
      StorageClassMemoryManagement<EasyStorageMapEntry<std::string, std::set<PreprocessingInfo*>>>:: writeToFile(outputFileStream);
      EasyStorageMapEntry<std::string, std::set<PreprocessingInfo*>> :: writeToFile(outputFileStream);
   }


void EasyStorage <std::map<std::string, std::set<PreprocessingInfo*>>> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|34|",inputFileStream);
#endif
  // call suitable methods on members and base class
     StorageClassMemoryManagement<EasyStorageMapEntry<std::string, std::set<PreprocessingInfo*>>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry<std::string, std::set<PreprocessingInfo*>> :: readFromFile (inputFileStream);
   }

/*
   ****************************************************************************************
   **      Implementations for <EasyStorageMapEntry <std::string,int>                    **
   ****************************************************************************************
*/
// Is not inherited from StorageClassMemoryManagement
void EasyStorageMapEntry <std::string,int> :: storeDataInEasyStorageClass(const std::pair<std::string, const int >& iter)
   {
     nameString.storeDataInEasyStorageClass(iter.first);
     index = iter.second;
   }

std::pair<std::string, int > EasyStorageMapEntry <std::string,int> :: rebuildDataStoredInEasyStorageClass() const
   {
     std::pair<std::string,int> returnPair(nameString.rebuildDataStoredInEasyStorageClass(),index);
     return returnPair;
   }

void EasyStorageMapEntry <std::string,int> :: arrangeMemoryPoolInOneBlock()
   {
     EasyStorage <std::string> :: arrangeMemoryPoolInOneBlock();
   }

void EasyStorageMapEntry <std::string,int> :: deleteMemoryPool()
   {
     EasyStorage <std::string> :: deleteMemoryPool();
   }


void EasyStorageMapEntry <std::string,int> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|17|",outputFileStream);
#endif
     EasyStorage <std::string> :: writeToFile(outputFileStream);
   }

void EasyStorageMapEntry <std::string,int> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|17|",inputFileStream);
#endif
     EasyStorage <std::string> :: readFromFile (inputFileStream);
   }

/*
   ****************************************************************************************
   **      Implementations for <EasyStorageMapEntry <std::string,uint64_t>                    **
   ****************************************************************************************
*/
// Is not inherited from StorageClassMemoryManagement
void EasyStorageMapEntry <std::string,uint64_t> :: storeDataInEasyStorageClass(const std::pair<std::string, const uint64_t >& iter)
   {
     nameString.storeDataInEasyStorageClass(iter.first);
     hash = iter.second;
   }

std::pair<std::string, uint64_t > EasyStorageMapEntry <std::string,uint64_t> :: rebuildDataStoredInEasyStorageClass() const
   {
     std::pair<std::string,uint64_t> returnPair(nameString.rebuildDataStoredInEasyStorageClass(),hash);
     return returnPair;
   }

void EasyStorageMapEntry <std::string,uint64_t> :: arrangeMemoryPoolInOneBlock()
   {
     EasyStorage <std::string> :: arrangeMemoryPoolInOneBlock();
   }

void EasyStorageMapEntry <std::string,uint64_t> :: deleteMemoryPool()
   {
     EasyStorage <std::string> :: deleteMemoryPool();
   }


void EasyStorageMapEntry <std::string,uint64_t> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|37|",outputFileStream);
#endif
     EasyStorage <std::string> :: writeToFile(outputFileStream);
   }

void EasyStorageMapEntry <std::string,uint64_t> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|37|",inputFileStream);
#endif
     EasyStorage <std::string> :: readFromFile (inputFileStream);
   }


/*
   ****************************************************************************************
   **      Implementations for EasyStorageMapEntry <int, std::string>                    **
   ****************************************************************************************
*/
// Is not inherited from StorageClassMemoryManagement
void EasyStorageMapEntry <int, std::string> :: storeDataInEasyStorageClass(const std::pair<const int, std::string>& iter)
   {
     index = iter.first;
     nameString.storeDataInEasyStorageClass(iter.second);
   }

std::pair<int, std::string> EasyStorageMapEntry <int, std::string> :: rebuildDataStoredInEasyStorageClass() const
   {
     std::pair<int, std::string> returnPair(index, nameString.rebuildDataStoredInEasyStorageClass());
     return returnPair;
   }

void EasyStorageMapEntry <int, std::string> :: arrangeMemoryPoolInOneBlock()
   {
     EasyStorage <std::string> :: arrangeMemoryPoolInOneBlock();
   }

void EasyStorageMapEntry <int, std::string> :: deleteMemoryPool()
   {
     EasyStorage <std::string> :: deleteMemoryPool();
   }


void EasyStorageMapEntry <int,std::string> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|18|",outputFileStream);
#endif
     EasyStorage <std::string> :: writeToFile(outputFileStream);
   }

void EasyStorageMapEntry <int,std::string> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|18|",inputFileStream);
#endif
     EasyStorage <std::string> :: readFromFile (inputFileStream);
   }


/*
   ****************************************************************************************
   **           Implementations for <EasyStorageMapEntry <SgNode*,int>             **
   ****************************************************************************************
*/
// Is not inherited from StorageClassMemoryManagement
void EasyStorageMapEntry <SgNode*,int> :: storeDataInEasyStorageClass(const std::pair<SgNode*, const int >& iter)
   {
  // nameString.storeDataInEasyStorageClass(iter.first);
     global_id = AST_FILE_IO::getGlobalIndexFromSgClassPointer(iter.first);
     index = iter.second;
   }

std::pair<SgNode*, int > EasyStorageMapEntry <SgNode*,int> :: rebuildDataStoredInEasyStorageClass() const
   {
  // DQ (10/5/2006): We need to get the pointer value from the global_id, but for now just set to NULL pointer!
  // std::pair<SgNode*,int> returnPair(nameString.rebuildDataStoredInEasyStorageClass(),index);
     std::pair<SgNode*,int> returnPair(NULL,index);
     return returnPair;
   }

void EasyStorageMapEntry <SgNode*,int> :: arrangeMemoryPoolInOneBlock()
   {
  // EasyStorage <SgNode*> :: arrangeMemoryPoolInOneBlock();
   }

void EasyStorageMapEntry <SgNode*,int> :: deleteMemoryPool()
   {
  // EasyStorage <SgNode*> :: deleteMemoryPool();
   }


void EasyStorageMapEntry <SgNode*,int> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|19|",outputFileStream);
#endif
  // EasyStorage <SgNode*> :: writeToFile(outputFileStream);
   }

void EasyStorageMapEntry <SgNode*,int> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|19|",inputFileStream);
#endif
  // EasyStorage <SgNode*> :: readFromFile (inputFileStream);
   }



/*
   ****************************************************************************************
   **      Implementations for EasyStorage <std::map<int,std::string>>                 **
   ****************************************************************************************
*/
void EasyStorage <std::map<int,std::string>> :: storeDataInEasyStorageClass(const std::map<int,std::string>& data_)
   {
     std::map<int,std::string>::const_iterator dat = data_.begin();
     long offset = Base::setPositionAndSizeAndReturnOffset ( data_.size() ) ;
  // if the new data does not fit in the actual block
     if (0 < offset)
        {
       // if there is still space in the actual block
          if (offset < Base::getSizeOfData())
             {
               if (Base::actual != NULL)
                  {
                    for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                       {
                         Base::actual->storeDataInEasyStorageClass(*dat);
                       }
                  }
             }
       // the data does not fit in one block
          while (Base::blockSize < (unsigned long)(offset))
             {
               Base::actual = Base::getNewMemoryBlock();
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                  {
                    Base::actual->storeDataInEasyStorageClass(*dat);
                  }
               offset -= Base::blockSize;
             }
          Base::actual = Base::getNewMemoryBlock();
        }
     for (; dat != data_.end(); ++dat, ++Base::actual)
        {
          Base::actual->storeDataInEasyStorageClass(*dat);
        }
   }

std::map<int,std::string>
EasyStorage <std::map<int,std::string>> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0 ) );
#endif
      std::map<int,std::string> data_;
   // if the memory pool is valid
      std::pair <int, std::string> tempPair;
      if ( Base::actual != NULL && 0 < Base::getSizeOfData() )
         {
           EasyStorageMapEntry<int,std::string>* pointer = Base::getBeginningOfDataBlock();
           for ( long i=0; i < Base::getSizeOfData(); ++i )
              {
                assert (Base::actualBlock == 1);
                tempPair = (pointer+i)->rebuildDataStoredInEasyStorageClass();
                data_[tempPair.first] = tempPair.second;
              }
         }
      return data_;

   }

void EasyStorage <std::map<int,std::string>> :: arrangeMemoryPoolInOneBlock()
   {
     StorageClassMemoryManagement<EasyStorageMapEntry<int, std::string>> :: arrangeMemoryPoolInOneBlock();
     EasyStorageMapEntry<int, std::string> :: arrangeMemoryPoolInOneBlock();
   }

void EasyStorage <std::map<int,std::string>> :: deleteMemoryPool()
   {
     StorageClassMemoryManagement<EasyStorageMapEntry<int, std::string>> :: deleteMemoryPool();
     EasyStorageMapEntry<int, std::string> :: deleteMemoryPool();
   }


void EasyStorage <std::map<int,std::string>> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|20|",outputFileStream);
#endif
     StorageClassMemoryManagement<EasyStorageMapEntry<int, std::string>> :: writeToFile(outputFileStream);
     EasyStorageMapEntry<int, std::string> :: writeToFile(outputFileStream);
   }

void EasyStorage <std::map<int,std::string>> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|20|",inputFileStream);
#endif
     StorageClassMemoryManagement<EasyStorageMapEntry<int, std::string>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry<int, std::string> :: readFromFile (inputFileStream);
   }




/*
   ****************************************************************************************
   **      Implementations for EasyStorage <std::map<std::string,int>>                 **
   ****************************************************************************************
*/
void EasyStorage <std::map<std::string,int>> :: storeDataInEasyStorageClass(const std::map<std::string,int>& data_)
   {
     std::map<std::string,int>::const_iterator dat = data_.begin();
     long offset = Base::setPositionAndSizeAndReturnOffset ( data_.size() ) ;
  // if the new data does not fit in the actual block
     if (0 < offset)
        {
       // if there is still space in the actual block
          if (offset < Base::getSizeOfData())
             {
               if (Base::actual != NULL)
                  {
                    for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                       {
                         Base::actual->storeDataInEasyStorageClass(*dat);
                       }
                  }
             }
       // the data does not fit in one block
          while (Base::blockSize < (unsigned long)(offset))
             {
               Base::actual = Base::getNewMemoryBlock();
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                  {
                    Base::actual->storeDataInEasyStorageClass(*dat);
                  }
               offset -= Base::blockSize;
             }
          Base::actual = Base::getNewMemoryBlock();
        }
     for (; dat != data_.end(); ++dat, ++Base::actual)
        {
          Base::actual->storeDataInEasyStorageClass(*dat);
        }
   }

std::map<std::string,int> EasyStorage <std::map<std::string,int>> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0 ) );
#endif
      std::map<std::string,int> data_;
   // if the memory pool is valid
      if ( Base::actual != NULL && 0 < Base::getSizeOfData() )
         {
           std::pair<std::string,int> tempPair;
           EasyStorageMapEntry<std::string,int>* pointer = Base::getBeginningOfDataBlock();
           for ( long i=0; i < Base::getSizeOfData(); ++i )
              {
                assert (Base::actualBlock == 1);
                tempPair = (pointer+i)->rebuildDataStoredInEasyStorageClass();
                data_[tempPair.first] = tempPair.second;
              }
         }
      return data_;

   }

void EasyStorage <std::map<std::string,int>> :: arrangeMemoryPoolInOneBlock()
   {
     StorageClassMemoryManagement<EasyStorageMapEntry<std::string, int>>:: arrangeMemoryPoolInOneBlock();
     EasyStorageMapEntry<std::string,int> :: arrangeMemoryPoolInOneBlock();
   }

void EasyStorage <std::map<std::string,int>> :: deleteMemoryPool()
   {
     StorageClassMemoryManagement<EasyStorageMapEntry<std::string, int>> :: deleteMemoryPool();
     EasyStorageMapEntry<std::string,int> :: deleteMemoryPool();
   }

void EasyStorage <std::map<std::string,int>> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|21|",outputFileStream);
#endif
     StorageClassMemoryManagement<EasyStorageMapEntry<std::string, int>>:: writeToFile(outputFileStream);
     EasyStorageMapEntry<std::string,int> :: writeToFile(outputFileStream);
   }

void EasyStorage <std::map<std::string,int>> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|21|",inputFileStream);
#endif
     StorageClassMemoryManagement<EasyStorageMapEntry<std::string, int>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry<std::string,int> :: readFromFile (inputFileStream);
   }

/*
   ****************************************************************************************
   **      Implementations for EasyStorage <std::map<std::string, uint64_t>>                 **
   ****************************************************************************************
*/
void EasyStorage <std::map<std::string,uint64_t>> :: storeDataInEasyStorageClass(const std::map<std::string,uint64_t>& data_)
   {
     std::map<std::string,uint64_t>::const_iterator dat = data_.begin();
     long offset = Base::setPositionAndSizeAndReturnOffset ( data_.size() ) ;
  // if the new data does not fit in the actual block
     if (0 < offset)
        {
       // if there is still space in the actual block
          if (offset < Base::getSizeOfData())
             {
               if (Base::actual != NULL)
                  {
                    for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                       {
                         Base::actual->storeDataInEasyStorageClass(*dat);
                       }
                  }
             }
       // the data does not fit in one block
          while (Base::blockSize < (unsigned long)(offset))
             {
               Base::actual = Base::getNewMemoryBlock();
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                  {
                    Base::actual->storeDataInEasyStorageClass(*dat);
                  }
               offset -= Base::blockSize;
             }
          Base::actual = Base::getNewMemoryBlock();
        }
     for (; dat != data_.end(); ++dat, ++Base::actual)
        {
          Base::actual->storeDataInEasyStorageClass(*dat);
        }
   }

std::map<std::string,uint64_t> EasyStorage <std::map<std::string,uint64_t>> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0 ) );
#endif
      std::map<std::string,uint64_t> data_;
   // if the memory pool is valid
      if ( Base::actual != NULL && 0 < Base::getSizeOfData() )
         {
           std::pair<std::string,uint64_t> tempPair;
           EasyStorageMapEntry<std::string,uint64_t>* pointer = Base::getBeginningOfDataBlock();
           for ( long i=0; i < Base::getSizeOfData(); ++i )
              {
                assert (Base::actualBlock == 1);
                tempPair = (pointer+i)->rebuildDataStoredInEasyStorageClass();
                data_[tempPair.first] = tempPair.second;
              }
         }
      return data_;

   }

void EasyStorage <std::map<std::string,uint64_t>> :: arrangeMemoryPoolInOneBlock()
   {
     StorageClassMemoryManagement<EasyStorageMapEntry<std::string, uint64_t>>:: arrangeMemoryPoolInOneBlock();
     EasyStorageMapEntry<std::string,uint64_t> :: arrangeMemoryPoolInOneBlock();
   }

void EasyStorage <std::map<std::string,uint64_t>> :: deleteMemoryPool()
   {
     StorageClassMemoryManagement<EasyStorageMapEntry<std::string, uint64_t>> :: deleteMemoryPool();
     EasyStorageMapEntry<std::string,uint64_t> :: deleteMemoryPool();
   }

void EasyStorage <std::map<std::string,uint64_t>> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|38|",outputFileStream);
#endif
     StorageClassMemoryManagement<EasyStorageMapEntry<std::string, uint64_t>>:: writeToFile(outputFileStream);
     EasyStorageMapEntry<std::string,uint64_t> :: writeToFile(outputFileStream);
   }

void EasyStorage <std::map<std::string,uint64_t>> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|38|",inputFileStream);
#endif
     StorageClassMemoryManagement<EasyStorageMapEntry<std::string, uint64_t>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry<std::string,uint64_t> :: readFromFile (inputFileStream);
   }

/*
   ****************************************************************************************
   **           Implementations for EasyStorage <std::map<SgNode*,int>>          **
   ****************************************************************************************
*/
void EasyStorage <std::map<SgNode*,int>> :: storeDataInEasyStorageClass(const std::map<SgNode*,int>&) {
}

std::map<SgNode*,int> EasyStorage <std::map<SgNode*,int>> :: rebuildDataStoredInEasyStorageClass() const {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
    ASSERT_require(Base::actualBlock <= 1);
    ASSERT_require((0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0));
#endif
    return std::map<SgNode*,int>{};
}

void EasyStorage <std::map<SgNode*,int>> :: arrangeMemoryPoolInOneBlock() {
}

void EasyStorage <std::map<SgNode*,int>> :: deleteMemoryPool() {
}

void EasyStorage <std::map<SgNode*,int>> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|22|",outputFileStream);
#endif
     StorageClassMemoryManagement<EasyStorageMapEntry<SgNode*, int>>:: writeToFile(outputFileStream);
     EasyStorageMapEntry<SgNode*,int> :: writeToFile(outputFileStream);
   }

void EasyStorage <std::map<SgNode*,int>> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|22|",inputFileStream);
#endif
     StorageClassMemoryManagement<EasyStorageMapEntry<SgNode*, int>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry<SgNode*,int> :: readFromFile (inputFileStream);
   }


/*
   ****************************************************************************************
   **      Implementations for EasyStorage <std::map<SgNode*,std::string>>             **
   ****************************************************************************************
*/
void EasyStorage <std::map<SgNode*,std::string>> :: storeDataInEasyStorageClass(const std::map<SgNode*,std::string>& /*data_*/) {
}

std::map<SgNode*,std::string>
EasyStorage <std::map<SgNode*,std::string>> :: rebuildDataStoredInEasyStorageClass() const {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
    ASSERT_require(Base::actualBlock <= 1);
    ASSERT_require((0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0));
#endif
    return std::map<SgNode*,std::string>{};
}

void EasyStorage <std::map<SgNode*,std::string>> :: arrangeMemoryPoolInOneBlock() {
}

void EasyStorage <std::map<SgNode*,std::string>> :: deleteMemoryPool() {
}

void EasyStorage <std::map<SgNode*,std::string>> :: writeToFile(std::ostream& outputFileStream) {
#if FILE_IO_MARKER
    AST_FILE_IO_MARKER::writeMarker("|23|",outputFileStream);
#endif
}

void EasyStorage <std::map<SgNode*,std::string>> :: readFromFile (std::istream& inputFileStream) {
#if FILE_IO_MARKER
    AST_FILE_IO_MARKER::readMarker("|23|",inputFileStream);
#endif
}

/*
***************************************************************************************************
   **      Implementations for EasyStorage <std::map<int,std::map<SgNode*,std::string>>>      **
***************************************************************************************************
*/
void EasyStorage <std::map<SgNode*,std::map<SgNode*,std::string>>> :: storeDataInEasyStorageClass(const std::map<SgNode*,std::map<SgNode*,std::string>>& data_)
   {
  // DQ (3/13/2019): Comment out so that we can debug the compiling issues
#if 0
     std::map<SgNode*,std::string>::const_iterator dat = data_.begin();
     long offset = Base::setPositionAndSizeAndReturnOffset ( data_.size() ) ;
  // if the new data does not fit in the actual block
     if (0 < offset)
        {
       // if there is still space in the actual block
          if (offset < Base::getSizeOfData())
             {
               if (Base::actual != NULL)
                  {
                    for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                       {
                         Base::actual->storeDataInEasyStorageClass(*dat);
                       }
                  }
             }
       // the data does not fit in one block
          while (Base::blockSize < (unsigned long)(offset))
             {
               Base::actual = Base::getNewMemoryBlock();
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                  {
                    Base::actual->storeDataInEasyStorageClass(*dat);
                  }
               offset -= Base::blockSize;
             }
          Base::actual = Base::getNewMemoryBlock();
        }
     for (; dat != data_.end(); ++dat, ++Base::actual)
        {
          Base::actual->storeDataInEasyStorageClass(*dat);
        }
#else
     ROSE_UNUSED(data_);
#endif
   }

std::map<SgNode*,std::map<SgNode*,std::string>>
EasyStorage <std::map<SgNode*,std::map<SgNode*,std::string>>> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0 ) );
#endif
      std::map<SgNode*,std::map<SgNode*,std::string>> data_;

  // DQ (3/13/2019): Comment out so that we can debug the linking issues
#if 0
   // if the memory pool is valid
      std::pair <SgNode*, std::string> tempPair;
      if ( Base::actual != NULL && 0 < Base::getSizeOfData() )
         {
           EasyStorageMapEntry<SgNode*,std::string>* pointer = Base::getBeginningOfDataBlock();
           for ( long i=0; i < Base::getSizeOfData(); ++i )
              {
                assert (Base::actualBlock == 1);
                tempPair = (pointer+i)->rebuildDataStoredInEasyStorageClass();
                data_[tempPair.first] = tempPair.second;
              }
         }
#endif
      return data_;

   }

void EasyStorage <std::map<SgNode*,std::map<SgNode*,std::string>>> :: arrangeMemoryPoolInOneBlock()
   {
  // DQ (3/13/2019): Comment out so that we can debug the linking issues
#if 0
     StorageClassMemoryManagement<EasyStorageMapEntry<SgNode*, std::string>> :: arrangeMemoryPoolInOneBlock();
     EasyStorageMapEntry<SgNode*, std::string> :: arrangeMemoryPoolInOneBlock();
#endif
   }

void EasyStorage <std::map<SgNode*,std::map<SgNode*,std::string>>> :: deleteMemoryPool()
   {
  // DQ (3/13/2019): Comment out so that we can debug the linking issues
#if 0
     StorageClassMemoryManagement<EasyStorageMapEntry<SgNode*, std::string>> :: deleteMemoryPool();
     EasyStorageMapEntry<SgNode*, std::string> :: deleteMemoryPool();
#endif
   }


void EasyStorage <std::map<SgNode*,std::map<SgNode*,std::string>>> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|23|",outputFileStream);
#endif

  // DQ (3/13/2019): Comment out so that we can debug the linking issues
#if 0
     StorageClassMemoryManagement<EasyStorageMapEntry<SgNode*, std::string>> :: writeToFile(outputFileStream);
     EasyStorageMapEntry<SgNode*, std::string> :: writeToFile(outputFileStream);
#endif
   }

void EasyStorage <std::map<SgNode*,std::map<SgNode*,std::string>>> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|23|",inputFileStream);
#endif

  // DQ (3/13/2019): Comment out so that we can debug the linking issues
#if 0
     StorageClassMemoryManagement<EasyStorageMapEntry<SgNode*, std::string>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry<SgNode*, std::string> :: readFromFile (inputFileStream);
#endif
   }


/*
***************************************************************************************************
   **      Implementations for EasyStorage <std::map<int,std::unordered_map<SgNode*,std::string>>>      **
***************************************************************************************************
*/
void EasyStorage <std::map<SgNode*,std::unordered_map<SgNode*,std::string>>> :: storeDataInEasyStorageClass(const std::map<SgNode*,std::unordered_map<SgNode*,std::string>>& data_)
   {
  // DQ (3/13/2019): Comment out so that we can debug the compiling issues
#if 0
     std::unordered_map<SgNode*,std::string>::const_iterator dat = data_.begin();
     long offset = Base::setPositionAndSizeAndReturnOffset ( data_.size() ) ;
  // if the new data does not fit in the actual block
     if (0 < offset)
        {
       // if there is still space in the actual block
          if (offset < Base::getSizeOfData())
             {
               if (Base::actual != NULL)
                  {
                    for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                       {
                         Base::actual->storeDataInEasyStorageClass(*dat);
                       }
                  }
             }
       // the data does not fit in one block
          while (Base::blockSize < (unsigned long)(offset))
             {
               Base::actual = Base::getNewMemoryBlock();
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                  {
                    Base::actual->storeDataInEasyStorageClass(*dat);
                  }
               offset -= Base::blockSize;
             }
          Base::actual = Base::getNewMemoryBlock();
        }
     for (; dat != data_.end(); ++dat, ++Base::actual)
        {
          Base::actual->storeDataInEasyStorageClass(*dat);
        }
#else
     ROSE_UNUSED(data_);
#endif
   }

std::map<SgNode*,std::unordered_map<SgNode*,std::string>>
EasyStorage <std::map<SgNode*,std::unordered_map<SgNode*,std::string>>> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0 ) );
#endif
      std::map<SgNode*,std::unordered_map<SgNode*,std::string>> data_;

  // DQ (3/13/2019): Comment out so that we can debug the linking issues
#if 0
   // if the memory pool is valid
      std::pair <SgNode*, std::string> tempPair;
      if ( Base::actual != NULL && 0 < Base::getSizeOfData() )
         {
           EasyStorageMapEntry<SgNode*,std::string>* pointer = Base::getBeginningOfDataBlock();
           for ( long i=0; i < Base::getSizeOfData(); ++i )
              {
                assert (Base::actualBlock == 1);
                tempPair = (pointer+i)->rebuildDataStoredInEasyStorageClass();
                data_[tempPair.first] = tempPair.second;
              }
         }
#endif
      return data_;

   }

void EasyStorage <std::map<SgNode*,std::unordered_map<SgNode*,std::string>>> :: arrangeMemoryPoolInOneBlock()
   {
  // DQ (3/13/2019): Comment out so that we can debug the linking issues
#if 0
     StorageClassMemoryManagement<EasyStorageMapEntry<SgNode*, std::string>> :: arrangeMemoryPoolInOneBlock();
     EasyStorageMapEntry<SgNode*, std::string> :: arrangeMemoryPoolInOneBlock();
#endif
   }

void EasyStorage <std::map<SgNode*,std::unordered_map<SgNode*,std::string>>> :: deleteMemoryPool()
   {
  // DQ (3/13/2019): Comment out so that we can debug the linking issues
#if 0
     StorageClassMemoryManagement<EasyStorageMapEntry<SgNode*, std::string>> :: deleteMemoryPool();
     EasyStorageMapEntry<SgNode*, std::string> :: deleteMemoryPool();
#endif
   }


void EasyStorage <std::map<SgNode*,std::unordered_map<SgNode*,std::string>>> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|23|",outputFileStream);
#endif

  // DQ (3/13/2019): Comment out so that we can debug the linking issues
#if 0
     StorageClassMemoryManagement<EasyStorageMapEntry<SgNode*, std::string>> :: writeToFile(outputFileStream);
     EasyStorageMapEntry<SgNode*, std::string> :: writeToFile(outputFileStream);
#endif
   }

void EasyStorage <std::map<SgNode*,std::unordered_map<SgNode*,std::string>>> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|23|",inputFileStream);
#endif

  // DQ (3/13/2019): Comment out so that we can debug the linking issues
#if 0
     StorageClassMemoryManagement<EasyStorageMapEntry<SgNode*, std::string>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry<SgNode*, std::string> :: readFromFile (inputFileStream);
#endif
   }



/*
   ****************************************************************************************
   **      Implementations for EasyStorage < std::unordered_map<SgNode*,std::string>>             **
   ****************************************************************************************
*/
void EasyStorage < std::unordered_map<SgNode*,std::string>> :: storeDataInEasyStorageClass(const std::unordered_map<SgNode*,std::string>& /*data_*/)
   {
  // DQ (10/6/2006): Comment out so that we can debug the compiling issues
#if 0
     std::unordered_map<SgNode*,std::string>::const_iterator dat = data_.begin();
     long offset = Base::setPositionAndSizeAndReturnOffset ( data_.size() ) ;
  // if the new data does not fit in the actual block
     if (0 < offset)
        {
       // if there is still space in the actual block
          if (offset < Base::getSizeOfData())
             {
               if (Base::actual != NULL)
                  {
                    for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                       {
                         Base::actual->storeDataInEasyStorageClass(*dat);
                       }
                  }
             }
       // the data does not fit in one block
          while (Base::blockSize < (unsigned long)(offset))
             {
               Base::actual = Base::getNewMemoryBlock();
               for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++dat)
                  {
                    Base::actual->storeDataInEasyStorageClass(*dat);
                  }
               offset -= Base::blockSize;
             }
          Base::actual = Base::getNewMemoryBlock();
        }
     for (; dat != data_.end(); ++dat, ++Base::actual)
        {
          Base::actual->storeDataInEasyStorageClass(*dat);
        }
#endif
   }

std::unordered_map<SgNode*,std::string>
EasyStorage < std::unordered_map<SgNode*,std::string>> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0 ) );
#endif
      std::unordered_map<SgNode*,std::string> data_;

  // DQ (10/6/2006): Comment out so that we can debug the linking issues
#if 0
   // if the memory pool is valid
      std::pair <SgNode*, std::string> tempPair;
      if ( Base::actual != NULL && 0 < Base::getSizeOfData() )
         {
           EasyStorageMapEntry<SgNode*,std::string>* pointer = Base::getBeginningOfDataBlock();
           for ( long i=0; i < Base::getSizeOfData(); ++i )
              {
                assert (Base::actualBlock == 1);
                tempPair = (pointer+i)->rebuildDataStoredInEasyStorageClass();
                data_[tempPair.first] = tempPair.second;
              }
         }
#endif
      return data_;

   }

void EasyStorage < std::unordered_map<SgNode*,std::string>> :: arrangeMemoryPoolInOneBlock()
   {
  // DQ (10/6/2006): Comment out so that we can debug the linking issues
#if 0
     StorageClassMemoryManagement<EasyStorageMapEntry<SgNode*, std::string>> :: arrangeMemoryPoolInOneBlock();
     EasyStorageMapEntry<SgNode*, std::string> :: arrangeMemoryPoolInOneBlock();
#endif
   }

void EasyStorage < std::unordered_map<SgNode*,std::string>> :: deleteMemoryPool()
   {
  // DQ (10/6/2006): Comment out so that we can debug the linking issues
#if 0
     StorageClassMemoryManagement<EasyStorageMapEntry<SgNode*, std::string>> :: deleteMemoryPool();
     EasyStorageMapEntry<SgNode*, std::string> :: deleteMemoryPool();
#endif
   }


void EasyStorage < std::unordered_map<SgNode*,std::string>> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|23|",outputFileStream);  // PP \todo update marker
#endif

  // DQ (10/6/2006): Comment out so that we can debug the linking issues
#if 0
     StorageClassMemoryManagement<EasyStorageMapEntry<SgNode*, std::string>> :: writeToFile(outputFileStream);
     EasyStorageMapEntry<SgNode*, std::string> :: writeToFile(outputFileStream);
#endif
   }

void EasyStorage < std::unordered_map<SgNode*,std::string>> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|23|",inputFileStream);  // PP \todo update marker
#endif

  // DQ (10/6/2006): Comment out so that we can debug the linking issues
#if 0
     StorageClassMemoryManagement<EasyStorageMapEntry<SgNode*, std::string>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry<SgNode*, std::string> :: readFromFile (inputFileStream);
#endif
   }



//#ifdef ROSE_USE_NEW_GRAPH_NODES

// ****************************************************************************************
// **      Implementations for EasyStorage < rose_graph_hash_multimap* >                 **
// ****************************************************************************************

#if 0
void EasyStorage <rose_graph_hash_multimap*> ::storeDataInEasyStorageClass(rose_graph_hash_multimap* data_)
   {
     if ( data_ == NULL )
        {
          Base::sizeOfData = -1;
        }
     else
        {
#if 0
       // store the parent pointer as unsigned long (this should better be AddrType). FixMe, also in the class declaration !
          parent = AST_FILE_IO :: getGlobalIndexFromSgClassPointer( data_->parent );
       // get staring iterator
          rose_hash::unordered_multimap<SgName, SgSymbol*, hash_Name, eqstr>::iterator copy_ = data_->begin();
          long offset = Base::setPositionAndSizeAndReturnOffset ( data_->size() ) ;
       // if the new data does not fit in the actual block
          if (0 < offset)
             {
            // if there is still space in the actual block
               if ( offset < Base::getSizeOfData() && Base::actual != NULL )
                  {
                    for (; (unsigned long) (Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++copy_)
                       {
                         Base::actual->storeDataInEasyStorageClass(*copy_);
                       }
                  }
            // the data does not fit in one block
               while (Base::blockSize < (unsigned long)(offset))
                 {
                  Base::actual = Base::getNewMemoryBlock();
                  for (; (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize; ++Base::actual, ++copy_)
                     {
                      Base::actual->storeDataInEasyStorageClass(*copy_);
                     }
                  offset -= Base::blockSize;
                 }
            // get new memory block, since the old one is full
               Base::actual = Base::getNewMemoryBlock();
             }
       // store the (rest of the ) data
          for ( ; copy_ != data_->end(); ++copy_, ++Base::actual )
             {
              Base::actual->storeDataInEasyStorageClass(*copy_);
             }
#else
       // local else
       // DQ (4/23/2009): Incrementally adding support for new graph IR nodes in ROSE.
          printf ("Error: support for file IO on graph nodes not implemented! \n");
          ROSE_ABORT();
#endif
        }
   }


rose_graph_hash_multimap*
EasyStorage <rose_graph_hash_multimap*> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0 ) );
#endif

     rose_graph_hash_multimap* return_map = NULL;

#if 0
     if ( 0 <= Base::getSizeOfData() )
        {
          return_map = new rose_hash_multimap();
       // set the parent
          return_map->parent = AST_FILE_IO :: getSgClassPointerFromGlobalIndex(parent);
       // if the memory pool is valid
          if ( Base::actual != NULL  && 0 < Base::getSizeOfData() )
             {
               EasyStorageMapEntry<SgName,SgSymbol*> *pointer = Base::getBeginningOfDataBlock();
               for (int i = 0; i < Base::getSizeOfData(); ++i)
                  {
                    return_map->insert((pointer+i)->rebuildDataStoredInEasyStorageClass()) ;
                  }
             }
        }
#else
     // local else
  // DQ (4/23/2009): Incrementally adding support for new graph IR nodes in ROSE.
     printf ("Error: support for file IO on graph nodes not implemented! \n");
     ROSE_ABORT();
#endif
     return return_map;
   }


void EasyStorage <rose_graph_hash_multimap*> :: arrangeMemoryPoolInOneBlock()
   {
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: arrangeMemoryPoolInOneBlock();
      EasyStorageMapEntry <SgName,SgSymbol*> :: arrangeMemoryPoolInOneBlock();
   }


void EasyStorage <rose_graph_hash_multimap*> :: deleteMemoryPool()
   {
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: deleteMemoryPool();
     EasyStorageMapEntry <SgName,SgSymbol*> :: deleteMemoryPool();
   }


void EasyStorage <rose_graph_hash_multimap*>::writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|24|",outputFileStream);
#endif
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: writeToFile(outputFileStream);
      EasyStorageMapEntry <SgName,SgSymbol*> :: writeToFile( outputFileStream);
   }


void EasyStorage <rose_graph_hash_multimap*> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|24|",inputFileStream);
#endif
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry <SgName,SgSymbol*> :: readFromFile (inputFileStream);
   }
#else
// else belongs to #if0

void EasyStorage <rose_graph_hash_multimap> ::storeDataInEasyStorageClass(const rose_graph_hash_multimap&)
   {
  // DQ (4/23/2009): Incrementally adding support for new graph IR nodes in ROSE.
     printf ("Error: support for file IO on graph nodes not implemented! \n");
     ROSE_ABORT();
   }


rose_graph_hash_multimap
EasyStorage <rose_graph_hash_multimap> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0 ) );
#endif

     rose_graph_hash_multimap return_map;

  // DQ (4/23/2009): Incrementally adding support for new graph IR nodes in ROSE.
     printf ("Error: support for file IO on graph nodes not implemented! \n");
     ROSE_ABORT();

     return return_map;
   }


void EasyStorage <rose_graph_hash_multimap> :: arrangeMemoryPoolInOneBlock()
   {
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: arrangeMemoryPoolInOneBlock();
      EasyStorageMapEntry <SgName,SgSymbol*> :: arrangeMemoryPoolInOneBlock();
   }


void EasyStorage <rose_graph_hash_multimap> :: deleteMemoryPool()
   {
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: deleteMemoryPool();
     EasyStorageMapEntry <SgName,SgSymbol*> :: deleteMemoryPool();
   }


void EasyStorage <rose_graph_hash_multimap>::writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|24|",outputFileStream);
#endif
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: writeToFile(outputFileStream);
      EasyStorageMapEntry <SgName,SgSymbol*> :: writeToFile( outputFileStream);
   }


void EasyStorage <rose_graph_hash_multimap> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|24|",inputFileStream);
#endif
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry <SgName,SgSymbol*> :: readFromFile (inputFileStream);
   }
#endif
// endif of #if0



#if 0
// DQ (4/30/2009): Removed these in favor of the hash_multimap using the SgGraphEdge class.
// ****************************************************************************************
// **      Implementations for EasyStorage < rose_undirected_graph_hash_multimap* >                 **
// ****************************************************************************************


// DQ (4/25/2009): Modified interface to not use pointer to hash_map.
void
EasyStorage <rose_undirected_graph_hash_multimap> ::storeDataInEasyStorageClass(const rose_undirected_graph_hash_multimap& data_)
   {
  // DQ (4/23/2009): Incrementally adding support for new graph IR nodes in ROSE.
     printf ("Error: support for file IO on graph nodes not implemented! \n");
     ROSE_ABORT();
   }

// Return by value (consistant with other functions in this file)
rose_undirected_graph_hash_multimap
EasyStorage <rose_undirected_graph_hash_multimap> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0 ) );
#endif

     rose_undirected_graph_hash_multimap return_map;

  // DQ (4/23/2009): Incrementally adding support for new graph IR nodes in ROSE.
     printf ("Error: support for file IO on graph nodes not implemented! \n");
     ROSE_ABORT();
   }


void EasyStorage <rose_undirected_graph_hash_multimap> :: arrangeMemoryPoolInOneBlock()
   {
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: arrangeMemoryPoolInOneBlock();
      EasyStorageMapEntry <SgName,SgSymbol*> :: arrangeMemoryPoolInOneBlock();
   }


void EasyStorage <rose_undirected_graph_hash_multimap> :: deleteMemoryPool()
   {
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: deleteMemoryPool();
     EasyStorageMapEntry <SgName,SgSymbol*> :: deleteMemoryPool();
   }

void EasyStorage <rose_undirected_graph_hash_multimap>::writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|25|",outputFileStream);
#endif
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: writeToFile(outputFileStream);
      EasyStorageMapEntry <SgName,SgSymbol*> :: writeToFile( outputFileStream);
   }


void EasyStorage <rose_undirected_graph_hash_multimap> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|25|",inputFileStream);
#endif
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry <SgName,SgSymbol*> :: readFromFile (inputFileStream);
   }
#endif
// end if #if 0

#if 0
// DQ (4/30/2009): Removed these in favor of the hash_multimap using the SgGraphEdge class.
// ****************************************************************************************
// **      Implementations for EasyStorage < rose_directed_graph_hash_multimap* >                 **
// ****************************************************************************************

void EasyStorage <rose_directed_graph_hash_multimap> ::storeDataInEasyStorageClass(const rose_directed_graph_hash_multimap& data_)
   {
  // DQ (4/23/2009): Incrementally adding support for new graph IR nodes in ROSE.
     printf ("Error: support for file IO on graph nodes not implemented! \n");
     ROSE_ABORT();
   }


rose_directed_graph_hash_multimap
EasyStorage <rose_directed_graph_hash_multimap> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0 ) );
#endif

     rose_directed_graph_hash_multimap return_map;

  // DQ (4/23/2009): Incrementally adding support for new graph IR nodes in ROSE.
     printf ("Error: support for file IO on graph nodes not implemented! \n");
     ROSE_ABORT();
   }


void EasyStorage <rose_directed_graph_hash_multimap> :: arrangeMemoryPoolInOneBlock()
   {
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: arrangeMemoryPoolInOneBlock();
      EasyStorageMapEntry <SgName,SgSymbol*> :: arrangeMemoryPoolInOneBlock();
   }


void EasyStorage <rose_directed_graph_hash_multimap> :: deleteMemoryPool()
   {
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: deleteMemoryPool();
     EasyStorageMapEntry <SgName,SgSymbol*> :: deleteMemoryPool();
   }

void EasyStorage <rose_directed_graph_hash_multimap>::writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|26|",outputFileStream);
#endif
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: writeToFile(outputFileStream);
      EasyStorageMapEntry <SgName,SgSymbol*> :: writeToFile( outputFileStream);
   }


void EasyStorage <rose_directed_graph_hash_multimap> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|26|",inputFileStream);
#endif
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry <SgName,SgSymbol*> :: readFromFile (inputFileStream);
   }
#endif
// end of #if0



// *****************************************************************************************
// **      Implementations for EasyStorage < rose_graph_node_edge_hash_multimap* >        **
// *****************************************************************************************

void EasyStorage <rose_graph_node_edge_hash_multimap> ::storeDataInEasyStorageClass(const rose_graph_node_edge_hash_multimap&)
   {
  // DQ (4/23/2009): Incrementally adding support for new graph IR nodes in ROSE.
     printf ("Error: support for file IO on graph nodes not implemented! \n");
     ROSE_ABORT();
   }


rose_graph_node_edge_hash_multimap
EasyStorage <rose_graph_node_edge_hash_multimap> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0 ) );
#endif

     rose_graph_node_edge_hash_multimap return_map;

  // DQ (4/23/2009): Incrementally adding support for new graph IR nodes in ROSE.
     printf ("Error: support for file IO on graph nodes not implemented! \n");
     ROSE_ABORT();
   }


void EasyStorage <rose_graph_node_edge_hash_multimap> :: arrangeMemoryPoolInOneBlock()
   {
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: arrangeMemoryPoolInOneBlock();
      EasyStorageMapEntry <SgName,SgSymbol*> :: arrangeMemoryPoolInOneBlock();
   }


void EasyStorage <rose_graph_node_edge_hash_multimap> :: deleteMemoryPool()
   {
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: deleteMemoryPool();
     EasyStorageMapEntry <SgName,SgSymbol*> :: deleteMemoryPool();
   }

void EasyStorage <rose_graph_node_edge_hash_multimap>::writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|27|",outputFileStream);
#endif
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: writeToFile(outputFileStream);
      EasyStorageMapEntry <SgName,SgSymbol*> :: writeToFile( outputFileStream);
   }

void EasyStorage <rose_graph_node_edge_hash_multimap> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|27|",inputFileStream);
#endif
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<SgName,SgSymbol*>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry <SgName,SgSymbol*> :: readFromFile (inputFileStream);
   }

void EasyStorage <rose_graph_integer_node_hash_map> ::storeDataInEasyStorageClass(const rose_graph_integer_node_hash_map &) {
    ASSERT_require2(false, "Error: support for file IO on graph nodes not implemented!\n");
}

rose_graph_integer_node_hash_map
EasyStorage <rose_graph_integer_node_hash_map> :: rebuildDataStoredInEasyStorageClass() const {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
    ASSERT_require(Base::actualBlock <= 1);
    ASSERT_require((0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0));
#endif
     ASSERT_require2(false, "Error: support for file IO on graph nodes not implemented!\n");
     return rose_graph_integer_node_hash_map{};
}

void EasyStorage <rose_graph_integer_node_hash_map> :: arrangeMemoryPoolInOneBlock()
   {
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<int,SgGraphNode*>> :: arrangeMemoryPoolInOneBlock();
      EasyStorageMapEntry <SgName,SgSymbol*> :: arrangeMemoryPoolInOneBlock();
   }


void EasyStorage <rose_graph_integer_node_hash_map> :: deleteMemoryPool()
   {
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<int,SgGraphNode*>> :: deleteMemoryPool();
     EasyStorageMapEntry <int,SgGraphNode*> :: deleteMemoryPool();
   }


void EasyStorage <rose_graph_integer_node_hash_map>::writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|29|",outputFileStream);
#endif
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<int,SgGraphNode*>> :: writeToFile(outputFileStream);
      EasyStorageMapEntry <int,SgGraphNode*> :: writeToFile( outputFileStream);
   }


void EasyStorage <rose_graph_integer_node_hash_map> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|29|",inputFileStream);
#endif
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<int,SgGraphNode*>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry <int,SgGraphNode*> :: readFromFile (inputFileStream);
   }

void EasyStorage <rose_graph_integer_edge_hash_map> ::storeDataInEasyStorageClass(const rose_graph_integer_edge_hash_map &/*data_*/)
   {
  // DQ (4/23/2009): Incrementally adding support for new graph IR nodes in ROSE.
     printf ("Error: support for file IO on graph nodes not implemented! \n");
     ROSE_ABORT();
   }


rose_graph_integer_edge_hash_map
EasyStorage <rose_graph_integer_edge_hash_map> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0 ) );
#endif

     rose_graph_integer_edge_hash_map return_map;

  // DQ (4/23/2009): Incrementally adding support for new graph IR nodes in ROSE.
     printf ("Error: support for file IO on graph nodes not implemented! \n");
     ROSE_ABORT();
   }


void EasyStorage <rose_graph_integer_edge_hash_map> :: arrangeMemoryPoolInOneBlock()
   {
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<int,SgGraphEdge*>> :: arrangeMemoryPoolInOneBlock();
      EasyStorageMapEntry <SgName,SgSymbol*> :: arrangeMemoryPoolInOneBlock();
   }


void EasyStorage <rose_graph_integer_edge_hash_map> :: deleteMemoryPool()
   {
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<int,SgGraphEdge*>> :: deleteMemoryPool();
     EasyStorageMapEntry <int,SgGraphEdge*> :: deleteMemoryPool();
   }


void EasyStorage <rose_graph_integer_edge_hash_map>::writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|30|",outputFileStream);
#endif
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<int,SgGraphEdge*>> :: writeToFile(outputFileStream);
      EasyStorageMapEntry <int,SgGraphEdge*> :: writeToFile( outputFileStream);
   }


void EasyStorage <rose_graph_integer_edge_hash_map> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|30|",inputFileStream);
#endif
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<int,SgGraphEdge*>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry <int,SgGraphEdge*> :: readFromFile (inputFileStream);
   }
//#endif
//end of condition new_graph

//#ifdef ROSE_USE_NEW_GRAPH_NODES
// DQ (5/1/2009): Added support
// ****************************************************************************************
// **      Implementations for EasyStorage < rose_graph_integer_edge_hash_multimap >          **
// ****************************************************************************************

void EasyStorage <rose_graph_integer_edge_hash_multimap> ::storeDataInEasyStorageClass(const rose_graph_integer_edge_hash_multimap &)
   {
  // DQ (4/23/2009): Incrementally adding support for new graph IR nodes in ROSE.
     printf ("Error: support for file IO on graph nodes not implemented! \n");
     ROSE_ABORT();
   }


rose_graph_integer_edge_hash_multimap
EasyStorage <rose_graph_integer_edge_hash_multimap> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0 ) );
#endif

     rose_graph_integer_edge_hash_multimap return_map;

  // DQ (4/23/2009): Incrementally adding support for new graph IR nodes in ROSE.
     printf ("Error: support for file IO on graph nodes not implemented! \n");
     ROSE_ABORT();
   }


void EasyStorage <rose_graph_integer_edge_hash_multimap> :: arrangeMemoryPoolInOneBlock()
   {
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<int,SgGraphEdge*>> :: arrangeMemoryPoolInOneBlock();
      EasyStorageMapEntry <SgName,SgSymbol*> :: arrangeMemoryPoolInOneBlock();
   }


void EasyStorage <rose_graph_integer_edge_hash_multimap> :: deleteMemoryPool()
   {
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<int,SgGraphEdge*>> :: deleteMemoryPool();
     EasyStorageMapEntry <int,SgGraphEdge*> :: deleteMemoryPool();
   }


void EasyStorage <rose_graph_integer_edge_hash_multimap>::writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|30|",outputFileStream);
#endif
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<int,SgGraphEdge*>> :: writeToFile(outputFileStream);
      EasyStorageMapEntry <int,SgGraphEdge*> :: writeToFile( outputFileStream);
   }


void EasyStorage <rose_graph_integer_edge_hash_multimap> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|30|",inputFileStream);
#endif
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<int,SgGraphEdge*>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry <int,SgGraphEdge*> :: readFromFile (inputFileStream);
   }
//#endif
// end of condition new_graph

/*
   ****************************************************************************************
   **           Implementations for <EasyStorageMapEntry <int,SgGraphNode*>              **
   ****************************************************************************************
*/
// Is not inherited from StorageClassMemoryManagement
void EasyStorageMapEntry <int,SgGraphNode*> :: storeDataInEasyStorageClass(const std::pair<const int, SgGraphNode*>& iter)
   {
  // nameString.storeDataInEasyStorageClass(iter.first);
     index = iter.first;
     global_id = AST_FILE_IO::getGlobalIndexFromSgClassPointer(iter.second);
   }

std::pair<int,SgGraphNode*> EasyStorageMapEntry <int,SgGraphNode*> :: rebuildDataStoredInEasyStorageClass() const
   {
  // DQ (10/5/2006): We need to get the pointer value from the global_id, but for now just set to NULL pointer!
  // std::pair<SgNode*,int> returnPair(nameString.rebuildDataStoredInEasyStorageClass(),index);
     std::pair<int,SgGraphNode*> returnPair(index,NULL);
     return returnPair;
   }

void EasyStorageMapEntry <int,SgGraphNode*> :: arrangeMemoryPoolInOneBlock()
   {
  // EasyStorage <SgNode*> :: arrangeMemoryPoolInOneBlock();
   }

void EasyStorageMapEntry <int,SgGraphNode*> :: deleteMemoryPool()
   {
  // EasyStorage <SgNode*> :: deleteMemoryPool();
   }


void EasyStorageMapEntry <int,SgGraphNode*> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|31|",outputFileStream);
#endif
  // EasyStorage <SgNode*> :: writeToFile(outputFileStream);
   }

void EasyStorageMapEntry <int,SgGraphNode*> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|31|",inputFileStream);
#endif
  // EasyStorage <SgNode*> :: readFromFile (inputFileStream);
   }

/*
   ****************************************************************************************
   **           Implementations for <EasyStorageMapEntry <int,SgGraphEdge*>              **
   ****************************************************************************************
*/
// Is not inherited from StorageClassMemoryManagement
void EasyStorageMapEntry <int,SgGraphEdge*> :: storeDataInEasyStorageClass(const std::pair<const int, SgGraphEdge*>& iter)
   {
  // nameString.storeDataInEasyStorageClass(iter.first);
     index = iter.first;
     global_id = AST_FILE_IO::getGlobalIndexFromSgClassPointer(iter.second);
   }

std::pair<int,SgGraphEdge*> EasyStorageMapEntry <int,SgGraphEdge*> :: rebuildDataStoredInEasyStorageClass() const
   {
  // DQ (10/5/2006): We need to get the pointer value from the global_id, but for now just set to NULL pointer!
  // std::pair<SgNode*,int> returnPair(nameString.rebuildDataStoredInEasyStorageClass(),index);
     std::pair<int,SgGraphEdge*> returnPair(index,NULL);
     return returnPair;
   }

void EasyStorageMapEntry <int,SgGraphEdge*> :: arrangeMemoryPoolInOneBlock()
   {
  // EasyStorage <SgNode*> :: arrangeMemoryPoolInOneBlock();
   }

void EasyStorageMapEntry <int,SgGraphEdge*> :: deleteMemoryPool()
   {
  // EasyStorage <SgNode*> :: deleteMemoryPool();
   }


void EasyStorageMapEntry <int,SgGraphEdge*> :: writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|31|",outputFileStream);
#endif
   }

void EasyStorageMapEntry <int,SgGraphEdge*> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|31|",inputFileStream);
#endif
   }


void EasyStorage <rose_graph_string_integer_hash_multimap> ::storeDataInEasyStorageClass(const rose_graph_string_integer_hash_multimap &/*data_*/)
   {
  // DQ (4/23/2009): Incrementally adding support for new graph IR nodes in ROSE.
     printf ("Error: support for file IO on graph nodes not implemented! \n");
     ROSE_ABORT();
   }

rose_graph_string_integer_hash_multimap
EasyStorage <rose_graph_string_integer_hash_multimap> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0 ) );
#endif

     rose_graph_string_integer_hash_multimap return_map;

  // DQ (4/23/2009): Incrementally adding support for new graph IR nodes in ROSE.
     printf ("Error: support for file IO on graph nodes not implemented! \n");
     ROSE_ABORT();
   }


void EasyStorage <rose_graph_string_integer_hash_multimap> :: arrangeMemoryPoolInOneBlock()
   {
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<std::string,int>> :: arrangeMemoryPoolInOneBlock();
      EasyStorageMapEntry <std::string,int> :: arrangeMemoryPoolInOneBlock();
   }


void EasyStorage <rose_graph_string_integer_hash_multimap> :: deleteMemoryPool()
   {
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<std::string,int>> :: deleteMemoryPool();
     EasyStorageMapEntry <std::string,int> :: deleteMemoryPool();
   }


void EasyStorage <rose_graph_string_integer_hash_multimap>::writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|29|",outputFileStream);
#endif
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<std::string,int>> :: writeToFile(outputFileStream);
      EasyStorageMapEntry <std::string,int> :: writeToFile( outputFileStream);
   }


void EasyStorage <rose_graph_string_integer_hash_multimap> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|29|",inputFileStream);
#endif
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<std::string,int>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry <std::string,int> :: readFromFile (inputFileStream);
   }

void EasyStorage <rose_graph_integerpair_edge_hash_multimap> ::storeDataInEasyStorageClass(const rose_graph_integerpair_edge_hash_multimap &/*data_*/)
   {
  // DQ (4/23/2009): Incrementally adding support for new graph IR nodes in ROSE.
     printf ("Error: support for file IO on graph nodes not implemented! \n");
     ROSE_ABORT();
   }

rose_graph_integerpair_edge_hash_multimap
EasyStorage <rose_graph_integerpair_edge_hash_multimap> :: rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
      assert ( Base::actualBlock <= 1 );
      assert ( (0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() <= 0 ) );
#endif

     rose_graph_integerpair_edge_hash_multimap return_map;

  // DQ (4/23/2009): Incrementally adding support for new graph IR nodes in ROSE.
     printf ("Error: support for file IO on graph nodes not implemented! \n");
     ROSE_ABORT();
   }


void EasyStorage <rose_graph_integerpair_edge_hash_multimap> :: arrangeMemoryPoolInOneBlock()
   {
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<std::string,int>> :: arrangeMemoryPoolInOneBlock();
      EasyStorageMapEntry <std::string,int> :: arrangeMemoryPoolInOneBlock();
   }


void EasyStorage <rose_graph_integerpair_edge_hash_multimap> :: deleteMemoryPool()
   {
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<std::string,int>> :: deleteMemoryPool();
     EasyStorageMapEntry <std::string,int> :: deleteMemoryPool();
   }


void EasyStorage <rose_graph_integerpair_edge_hash_multimap>::writeToFile(std::ostream& outputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::writeMarker("|29|",outputFileStream);
#endif
   // call suitable methods of parent and member
      StorageClassMemoryManagement <EasyStorageMapEntry<std::string,int>> :: writeToFile(outputFileStream);
      EasyStorageMapEntry <std::string,int> :: writeToFile( outputFileStream);
   }


void EasyStorage <rose_graph_integerpair_edge_hash_multimap> :: readFromFile (std::istream& inputFileStream)
   {
#if FILE_IO_MARKER
     AST_FILE_IO_MARKER::readMarker("|29|",inputFileStream);
#endif
   // call suitable methods of parent and member
     StorageClassMemoryManagement <EasyStorageMapEntry<std::string,int>> :: readFromFile (inputFileStream);
     EasyStorageMapEntry <std::string,int> :: readFromFile (inputFileStream);
   }
//#endif
//end of condition new_graph

/*
   ****************************************************************************************
   **      Implementations for EasyStorageMapEntry <SgSharedVector<TYPE>>               **
   ****************************************************************************************
*/

#ifdef ROSE_ENABLE_BINARY_ANALYSIS
template <class TYPE>
void EasyStorage <SgSharedVector<TYPE>> :: storeDataInEasyStorageClass(const SgSharedVector<TYPE>& data_)
{
    typename SgSharedVector<TYPE>::const_iterator dat = data_.begin();
    long offset = Base::setPositionAndSizeAndReturnOffset ( data_.size() ) ;

    if (0 < offset) {
        /* The new data does not fit in the actual block, but store what we can in the actual block. */
        if (offset < Base::getSizeOfData() && Base::actual != NULL) {
            for (/*void*/;
                 (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize;
                 ++Base::actual, ++dat) {
                *Base::actual = *dat;
            }
        }

        /* Put data in additional blocks if it did not fit in the previous block. */
        while (Base::blockSize < (unsigned long)(offset)) {
            Base::actual = Base::getNewMemoryBlock();
            for (/*void*/;
                 (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize;
                 ++Base::actual, ++dat) {
                *Base::actual = *dat;
            }
            offset -= Base::blockSize;
        }

        /* get a new memory block because we've filled up previous blocks */
        Base::actual = Base::getNewMemoryBlock();
    }

    /* put (the rest of) the data in a (new) memory block */
    for (/*void*/; dat != data_.end(); ++dat, ++Base::actual)
        *Base::actual = *dat;
}

template <class TYPE>
SgSharedVector<TYPE> EasyStorage <SgSharedVector<TYPE>> :: rebuildDataStoredInEasyStorageClass() const
{
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
    assert(Base::actualBlock <= 1);
    assert((0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() == 0));
#endif

    if (Base::actual != nullptr && Base::getSizeOfData() > 0) {
        TYPE *pool = new TYPE[Base::getSizeOfData()];
        TYPE *pointer = Base::getBeginningOfDataBlock();
        for (long i=0; i<Base::getSizeOfData(); ++i)
            pool[i] = pointer[i];
        return SgSharedVector<TYPE>(pool, Base::getSizeOfData());
    }

    return SgSharedVector<TYPE>();
}
#endif

/*
   ****************************************************************************************
   **      Implementations for EasyStorageMapEntry<ExtentMap>                            **
   ****************************************************************************************
*/

#ifdef ROSE_ENABLE_BINARY_ANALYSIS
void EasyStorage<ExtentMap>::storeDataInEasyStorageClass(const ExtentMap& emap)
{
    /* Since first and second elements of the value_type pair are identical (both Rose::BinaryAnalysis::Address), we store them in a
     * single pool that is twice as large as the number of pairs in the ExtentMap. */
    std::vector<Rose::BinaryAnalysis::Address> data_;
    for (ExtentMap::const_iterator ei=emap.begin(); ei!=emap.end(); ++ei) {
        data_.push_back(ei->first.first());
        data_.push_back(ei->first.last());
    }

    std::vector<Rose::BinaryAnalysis::Address>::const_iterator dat = data_.begin();
    long offset = Base::setPositionAndSizeAndReturnOffset(data_.size());
    if (0 < offset) {
        /* The new data does not fit in the actual block, but store what we can in the actual block. */
        if (offset < Base::getSizeOfData() && Base::actual != NULL) {
            for (/*void*/;
                 (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize;
                 ++Base::actual, ++dat) {
                *Base::actual = *dat;
            }
        }

        /* Put data in additional blocks if it did not fit in the previous block. */
        while (Base::blockSize < (unsigned long)(offset)) {
            Base::actual = Base::getNewMemoryBlock();
            for (/*void*/;
                 (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize;
                 ++Base::actual, ++dat) {
                *Base::actual = *dat;
            }
            offset -= Base::blockSize;
        }

        /* get a new memory block because we've filled up previous blocks */
        Base::actual = Base::getNewMemoryBlock();
    }

    /* put (the rest of) the data in a (new) memory block */
    for (/*void*/; dat != data_.end(); ++dat, ++Base::actual)
        *Base::actual = *dat;
}

ExtentMap EasyStorage<ExtentMap>::rebuildDataStoredInEasyStorageClass() const
   {
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
     assert(Base::actualBlock <= 1);
     assert((0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() == 0));
     assert(0 == Base::getSizeOfData() % 2); /* vector holds key/value pairs of the ExtentMap */
#endif

     ExtentMap emap;

// DQ (10/21/2010): This macro prevents the ExtentMap::insert() function from being undefined
// when binary analysis is not supported (via language only options in configure).
#ifdef ROSE_ENABLE_BINARY_ANALYSIS
     if (Base::actual != nullptr && Base::getSizeOfData() > 0)
        {
          Rose::BinaryAnalysis::Address *pointer = Base::getBeginningOfDataBlock();
          for (long i=0; i<Base::getSizeOfData(); i+=2)
              emap.insert(Extent::inin(pointer[i+0], pointer[i+1]));
        }
#endif

     return emap;
   }
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementations for AddressIntervalSet
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ROSE_ENABLE_BINARY_ANALYSIS
void
EasyStorage<Rose::BinaryAnalysis::AddressIntervalSet>::storeDataInEasyStorageClass(const Rose::BinaryAnalysis::AddressIntervalSet &set)
{
    // Since the set's elements are of type Sawyer::Container::Interval<Rose::BinaryAnalysis::Address> we can store the interval
    // endpoints in a single pool that is twice as large as the number of intervals in the set.
    std::vector<Rose::BinaryAnalysis::Address> data_;
    BOOST_FOREACH (const Rose::BinaryAnalysis::AddressInterval &interval, set.intervals()) {
        assert(!interval.isEmpty());
        data_.push_back(interval.least());
        data_.push_back(interval.greatest());
    }

    std::vector<Rose::BinaryAnalysis::Address>::const_iterator dat = data_.begin();
    long offset = Base::setPositionAndSizeAndReturnOffset(data_.size());
    if (0 < offset) {
        /* The new data does not fit in the actual block, but store what we can in the actual block. */
        if (offset < Base::getSizeOfData() && Base::actual != NULL) {
            for (/*void*/;
                 (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize;
                 ++Base::actual, ++dat) {
                *Base::actual = *dat;
            }
        }

        /* Put data in additional blocks if it did not fit in the previous block. */
        while (Base::blockSize < (unsigned long)(offset)) {
            Base::actual = Base::getNewMemoryBlock();
            for (/*void*/;
                 (unsigned long)(Base::actual - Base::getBeginningOfActualBlock()) < Base::blockSize;
                 ++Base::actual, ++dat) {
                *Base::actual = *dat;
            }
            offset -= Base::blockSize;
        }

        /* get a new memory block because we've filled up previous blocks */
        Base::actual = Base::getNewMemoryBlock();
    }

    /* put (the rest of) the data in a (new) memory block */
    for (/*void*/; dat != data_.end(); ++dat, ++Base::actual)
        *Base::actual = *dat;
}

Rose::BinaryAnalysis::AddressIntervalSet
EasyStorage<Rose::BinaryAnalysis::AddressIntervalSet>::rebuildDataStoredInEasyStorageClass() const
{
#if STORAGE_CLASS_MEMORY_MANAGEMENT_CHECK
    assert(Base::actualBlock <= 1);
    assert((0 < Base::getSizeOfData() && Base::actual != nullptr) || (Base::getSizeOfData() == 0));
    assert(0 == Base::getSizeOfData() % 2);             // vector holds interval endpoints or
#endif

    Rose::BinaryAnalysis::AddressIntervalSet retval;
    if (Base::actual != nullptr && Base::getSizeOfData() > 0) {
        Rose::BinaryAnalysis::Address *pointer = Base::getBeginningOfDataBlock();
        for (long i=0; i<Base::getSizeOfData(); i+=2) {
            Rose::BinaryAnalysis::Address lo = pointer[i+0];
            Rose::BinaryAnalysis::Address hi = pointer[i+1];
            assert(lo <= hi);
            retval.insert(Rose::BinaryAnalysis::AddressInterval::hull(lo, hi));
        }
    }
    return retval;
}
#endif
