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

#ifndef BLOCK
    #define BLOCK 0
    #include "Block.h"  // Include the header file for Block class
#endif

using namespace std;

class Row {
private:
    queue<Block*> blockQueue;
    int associativity;
    int rowNum;

public:
    Row(int assoc, int num){
        associativity = assoc;
        rowNum = num;
    }

    ~Row() {
        while (!blockQueue.empty()) {
            delete blockQueue.front();
            blockQueue.pop();
        }
    }

    // Get the front block from the queue
    Block* getFrontBlock() const {
        if (!blockQueue.empty()) {
            return blockQueue.front();
        }
        return nullptr;
    }

    // Remove and return the front block from the queue
    Block* dequeueBlock() {
        if (!blockQueue.empty()) {
            Block* frontBlock = blockQueue.front();
            blockQueue.pop();
            return frontBlock;
        }
        return nullptr;
    }
    
    int getAssoc(){
        return associativity;
    }
    

    bool containsBlock2(int tag){
        size_t queueSize = blockQueue.size(); // store the size of queue
        Block* frontBlock;
        Block* foundBlock = nullptr;
        // iterate through the queue
        // cout << "SIZE OF QUEUE: " << queueSize << endl;

        // cout << "TAG: " << tag << endl;

        for(int i = 0; i < queueSize; i++){
            frontBlock = blockQueue.front(); // store front block
            blockQueue.pop(); // dequeue block
            // cout << "FRONT BLOCK TAG:" << frontBlock->getBlockTag() << endl;
            if((frontBlock->getBlockTag()) == tag){ // check tag
                foundBlock = frontBlock;
                continue;
            }
            
            enqueueBlock(frontBlock);
        }
        // cout << endl;
        if(foundBlock != nullptr){
            enqueueBlock(foundBlock);
            return true;
        }else{
            return false;
        }
    }

    void writeBlock(int tag) {
        size_t queueSize = blockQueue.size(); // Store the size before the loop

        if (containsBlock2(tag)){ // if row contains block, exit
            return;
        }
        if(queueSize == associativity){ // if queue/row is full
            blockQueue.pop(); // dequeue the front block to add a new block
        } 
        // add a new block
        enqueueBlock(new Block(tag));
        // works for both cases: 
        // 1) if not full, block isnt there, just add 
        // 2) if it was full, we popped an element in the prev step, so now we add
    }

    // Enqueue a new block into the back of the queue
    void enqueueBlock(Block* block) {
        blockQueue.push(block);
    }
};