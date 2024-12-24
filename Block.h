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

using namespace std;


class Block {
    private:
        bool valid; // 1 or 0
        uint16_t tag;

    public:

        Block(int Tag){ // create new block with given tag (created when added to cache)
            tag = Tag;
            valid = 1;
        }

        uint16_t getBlockTag(){
            return tag;
        }

        void setTag(uint16_t t){
            tag = t;
        }

        void setVB(bool vb){
            valid = vb;
        }
    
        bool getVB(){
            return valid;
        }

};