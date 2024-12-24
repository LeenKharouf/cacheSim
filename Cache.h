#ifndef IMPORTS
    #define IMPORTS 0
    #include <cstddef>
    #include <iostream>
    #include <string>
    #include <vector>
    #include <fstream>
    #include <limits>
    #include <regex>
    #include <cstdlib>
    #include <iomanip>
    #include <string>
    #include <iostream>
    #include <queue>
#endif

#ifndef ROW
    #define ROW 0
    #include "Row.h"    // Include the header file for Row class
#endif

#ifndef BLOCK
    #define BLOCK 0
    #include "Block.h"  // Include the header file for Block class
#endif

using namespace std;


class Cache {
    private:
        int numRows;
        Row** rows; // pointer to an array of pointers to row objects
        int assoc;
        int blockSize;
        int size;
        
    public:

        Cache(int cacheSize, int associativity, int blckSize){
            blockSize = blckSize;
            size = cacheSize;
            assoc = associativity;
            if ((associativity * blckSize) == 0){
                numRows = 0;
            }else{
                numRows = size/(associativity * blckSize);
            }
            // assign the pointer to an array of pointers to row objects
            rows = new Row*[numRows]; 
            // each pointer points to ONE row object

            // create a Row object for the number of rows 
            for(int i=0; i < numRows; i++){
                rows[i] = new Row(associativity, i);
            }

        }

        int getAssoc(){ return assoc;}

        int getNumRows(){ return numRows;}

        int getSize(){ return size;}

        int getBlockSize(){ return blockSize;}

        bool read(uint16_t address){
            int blockID;
            int tag;
            // edge cases - just in case
            if (blockSize == 0){
                return false;
            }else{
                blockID = address / blockSize;
            }
            if (numRows == 0){
                return false;
            }else{
                tag = blockID / numRows;
            }
            
            uint16_t rowNum = blockID % numRows; // calculate row number 

            Row& myRow = getRowAt(rowNum); // get the correct Row object 

            // if row contains this Block (check block tag)
            if(myRow.containsBlock2(tag)){
                return true;
            }
            else{
                return false;
            }
        }

        bool write(uint16_t address){
            int blockID;
            int tag;
            // calculate blockID
            if (blockSize == 0){
                return false;
            }else{
                blockID = address / blockSize;
            }
            if (numRows == 0){
                return false;
            }else{
                tag = blockID / numRows;
            }
            // calculate row number 
            uint16_t rowNum = blockID % numRows;
            // get the correct Row object 
            Row& myRow = getRowAt(rowNum); 

            myRow.writeBlock(tag);

            return true;
        }
        
        // get row object given row number
        Row& getRowAt(int num) {
            return *rows[num];
        }

};