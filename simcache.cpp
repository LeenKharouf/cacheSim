/*
CS-UY 2214
Adapted from Jeff Epstein
Starter code for E20 cache Simulator
simcache.cpp
*/
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

#ifndef CACHE
    #define CACHE 0
    #include "Cache.h"  // Include the header file for Cache class
#endif

#ifndef ROW
    #define ROW 0
    #include "Row.h"    // Include the header file for Row class
#endif

#ifndef BLOCK
    #define BLOCK 0
    #include "Block.h"  // Include the header file for Block class
#endif

// Some helpful constant values that we'll be using.
size_t const static NUM_REGS = 8;
size_t const static MEM_SIZE = 1<<13;
size_t const static REG_SIZE = 1<<16;

using namespace std;

void print_cache_config(const string &cache_name, int size, int assoc, int blocksize, int num_rows) {
    cout << "Cache " << cache_name << " has size " << size <<
        ", associativity " << assoc << ", blocksize " << blocksize <<
        ", rows " << num_rows << endl;
}

void print_log_entry(const string &cache_name, const string &status, uint16_t &pc, uint16_t addr, int row){
    cout << left << setw(8) << cache_name + " " + status <<  right <<
        " pc:" << setw(5) << pc <<
        "\taddr:" << setw(5) << addr <<
        "\trow:" << setw(4) << row << endl;
}

void load_machine_code(ifstream &f, uint16_t mem[]) {
    regex machine_code_re("^ram\\[(\\d+)\\] = 16'b(\\d+);.*$");
    size_t expectedaddr = 0;
    string line;
    while (getline(f, line)) {
        smatch sm;
        if (!regex_match(line, sm, machine_code_re)) {
            cerr << "Can't parse line: " << line << endl;
            exit(1);
        }
        size_t addr = stoi(sm[1], nullptr, 10);
        uint16_t instr = stoi(sm[2], nullptr, 2); // converts instruction from binary to decimal

        if (addr != expectedaddr) {
            cerr << "Memory addresses encountered out of sequence: " << addr << endl;
            exit(1);
        }
        if (addr >= MEM_SIZE) {
            cerr << "Program too big for memory" << endl;
            exit(1);
        }
        expectedaddr ++;
        mem[addr] = instr;
    }
    return;
}

void threeRegInstr(uint16_t instruction, uint16_t regs[], uint16_t memory[], uint16_t &pc, unsigned opcode){
    uint16_t regSrcA = (instruction>>10) & 7;   
    uint16_t regSrcB = (instruction>>7) & 7;  
    uint16_t regDst = (instruction>>4) & 7; 
    unsigned four_LSB = instruction & 15; // store 4 LSB for 3 register argument instructions
    
    if(four_LSB == 0b0000){ // add instruction
        regs[regDst] = (regs[regSrcA] + regs[regSrcB]);            
    } 

    else if(four_LSB == 0b0001){ // sub instruction   
        regs[regDst] = (regs[regSrcA] - regs[regSrcB]);
    } 

    else if(four_LSB == 0b0010){ // or instruction
        regs[regDst] = regs[regSrcA] | regs[regSrcB];
    } 

    else if(four_LSB == 0b0011){ // and instruction
        regs[regDst] = regs[regSrcA] & regs[regSrcB];
    } 

    else if(four_LSB == 0b0100){ // slt instruction
        if(regs[regSrcA]<regs[regSrcB]){
            regs[regDst] = 1;
        }else{
            regs[regDst] = 0;
        }
    }
    else if(four_LSB == 0b1000){ // jr instruction
        // take 13LSB of reg
        uint16_t reg = (instruction>>10) & 7;   
        pc = (regs[reg])-1; // subtracting to take into account the increment after
    } 
    pc = pc + 1; // increment pc
    return;
}


void twoRegInstr(uint16_t instruction, uint16_t regs[], uint16_t memory[], uint16_t &pc, unsigned opcode, Cache* L1cache, Cache* L2cache, int L1blockSize, int L2blockSize){

    uint16_t imm = instruction & 127; // 7 bit imm zero extended 

    if ((imm & (1 << 6))==64) { // if MSB is 1, sign extend
        // sign extend
        imm = imm | 0b1111111110000000;
    }

    uint16_t regA = (instruction>>10) & 7;
    uint16_t regB = (instruction>>7) & 7;  

    uint16_t address = (imm + regs[regA])%8192; // for using in lw and sw

    int L1blockID = address/L1blockSize;
    int L1numRows = L1cache->getNumRows();
    int L1rowNum = L1blockID % L1numRows;


    if (opcode == 0b111){ // slti
        if(regs[regA]<imm){
            regs[regB] = 1;
        }else{
            regs[regB] = 0;
        }
    }
    else if (opcode == 0b100){ // lw
        regs[regB] = memory[address]; // max memory index = 8191
        // cout << "I'm inside LW instruction!" << endl;

        string L1status;

        if (L2cache == nullptr) { // if only L1
            // Looks for address in L1 cache 
            bool L1hit = L1cache->read(address);

            if (L1hit){
                L1status = "HIT";

            }else{
                L1status = "MISS";
                L1cache->write(address);
            }
            // PRINT L1
            print_log_entry("L1", L1status, pc, address, L1rowNum);
        }
        else{ // L1 and L2 cache

            int L2blockID = address/L2blockSize;
            int L2numRows = L2cache->getNumRows();
            int L2rowNum = L2blockID % L2numRows;

            bool L1hit = L1cache->read(address);
            string L2status;

            if (L1hit){ // L1 HIT
                L1status = "HIT";

            }else{  // L1 MISS
                L1status = "MISS";
                L1cache->write(address); // WRITE THROUGH TO L1
                
                bool L2hit = L2cache->read(address); // GO TO L2

                if (L2hit){
                    L2status = "HIT";
                }else{
                    L2status = "MISS";
                    L2cache->write(address); // WRITE THROUGH TO L2
                }

            }

            // PRINT L1
            print_log_entry("L1", L1status, pc, address, L1rowNum);
            
            // PRINT L2 IF L1 IS A MISS
            if(!L1hit){
                print_log_entry("L2", L2status, pc, address, L2rowNum);
            }
        }        

    
    }else if (opcode == 0b101){ // sw  
        memory[address] = regs[regB];

        string status = "SW";

        L1cache->write(address);
        print_log_entry("L1", status, pc, address, L1rowNum);

        if (L2cache != nullptr) { // if only L1
 
            int L2blockID = address/L2blockSize;
            int L2numRows = L2cache->getNumRows();
            int L2rowNum = L2blockID % L2numRows;

            L2cache->write(address);
            print_log_entry("L2", status, pc, address, L2rowNum);
        }        
    }     
    else if (opcode == 0b110){ // jeq 
        uint16_t rel_imm = imm;  

        if(regs[regA] == regs[regB]){
            pc = rel_imm + pc; // excluded '+1' as pc is incremented at the end
        }
    } 
    else if (opcode == 0b001){ // addi
        regs[regB] = (imm + regs[regA]);
    }
    pc = pc + 1; // increment pc        

    return;
}

void NoRegInstr(uint16_t instruction, uint16_t regs[], uint16_t memory[], uint16_t &pc, unsigned opcode, bool &endProgram){
    if(instruction == ((2<<13) | pc)){
        endProgram = true;
    }
    else if (opcode == 0b010){ // j
        uint16_t imm = instruction & 8191;   
        pc = imm; // store 16 bit imm into pc
    } 
    else{ // jal
        uint16_t imm = instruction & 8191;
        regs[7] = pc + 1; //store the next 16 bit pc into $7
        pc = imm; // jump to pc = imm
    } 
    return;
}


int main(int argc, char *argv[]) {
    /*
        Parse the command-line arguments
    */
    char *filename = nullptr;
    bool do_help = false;
    bool arg_error = false;
    string cache_config;

    for (int i=1; i<argc; i++) {
        string arg(argv[i]);
        if (arg.rfind("-",0)==0) {
            if (arg== "-h" || arg == "--help")
                do_help = true;
            else if (arg=="--cache") {
                i++;
                if (i>=argc)
                    arg_error = true;
                else
                    cache_config = argv[i];
            }
            else
                arg_error = true;
        } else {
            if (filename == nullptr)
                filename = argv[i];
            else
                arg_error = true;
        }
    }
    /* Display error message if appropriate */
    if (arg_error || do_help || filename == nullptr) {
        cerr << "usage " << argv[0] << " [-h] [--cache CACHE] filename" << endl << endl;
        cerr << "Simulate E20 cache" << endl << endl;
        cerr << "positional arguments:" << endl;
        cerr << "  filename    The file containing machine code, typically with .bin suffix" << endl<<endl;
        cerr << "optional arguments:"<<endl;
        cerr << "  -h, --help  show this help message and exit"<<endl;
        cerr << "  --cache CACHE  Cache configuration: size,associativity,blocksize (for one"<<endl;
        cerr << "                 cache) or"<<endl;
        cerr << "                 size,associativity,blocksize,size,associativity,blocksize"<<endl;
        cerr << "                 (for two caches)"<<endl;
        return 1;
    }

    int L1size;
    int L1assoc;
    int L1blocksize;
    int L2size;
    int L2assoc;
    int L2blocksize;

    Cache* L1cache = nullptr; // Declare L1cache as a pointer
    Cache* L2cache = nullptr; // Declare L2cache as a pointer
    bool L2 = false;

    /* parse cache config */
    if (cache_config.size() > 0) {
        vector<int> parts;
        size_t pos;
        size_t lastpos = 0;
        while ((pos = cache_config.find(",", lastpos)) != string::npos) {
            parts.push_back(stoi(cache_config.substr(lastpos,pos)));
            lastpos = pos + 1;
        }
        parts.push_back(stoi(cache_config.substr(lastpos)));
        if (parts.size() == 3) { // if L1 only
            L1size = parts[0];
            L1assoc = parts[1];
            L1blocksize = parts[2];
            int L1numRows;

            L1cache = new Cache(L1size, L1assoc, L1blocksize); // CREATE L1 CACHE OBJECT
            print_cache_config("L1", L1cache->getSize(), L1cache->getAssoc(), L1blocksize, L1cache->getNumRows());

        } else if (parts.size() == 6) {
            L1size = parts[0];
            L1assoc = parts[1];
            L1blocksize = parts[2];
            L2size = parts[3];
            L2assoc = parts[4];
            L2blocksize = parts[5];

            int L1numRows;
            int L2numRows;
           
            L2 = true;

            L1cache = new Cache(L1size, L1assoc, L1blocksize); // CREATE L1 CACHE OBJECT
            L2cache = new Cache(L2size, L2assoc, L2blocksize); // CREATE L2 CACHE OBJECT
            
            print_cache_config("L1", L1cache->getSize(), L1cache->getAssoc(), L1blocksize, L1cache->getNumRows());
            print_cache_config("L2", L2cache->getSize(), L2cache->getAssoc(), L2blocksize, L2cache->getNumRows());

        } else {
            cerr << "Invalid cache config"  << endl;
            return 1;
        }
    }


     // checks if file opens
    ifstream f(filename); // open file specified by filename
    if (!f.is_open()) { // error if can't open file
        cerr << "Can't open file " << filename << endl;
        return 1;
    }

    uint16_t memory[MEM_SIZE] = {0}; // Initialize all 8912 elements of memory to 0
    load_machine_code(f, memory); // loading machine code into memory array
    uint16_t pc = 0; // initialising pc to 0
    uint16_t regs[NUM_REGS] = {0, 0, 0, 0, 0, 0, 0, 0}; // creating an array of 8 registers, initialising all register values to 0
    size_t memQuantity = 128; // we want to print 128
    bool endProgram = false; // set to 'true' when halt instruction is reached
    uint16_t instruction = memory[pc%8192]; // instruction initialised
    
    while (!endProgram) {
        instruction = memory[pc%8192]; // instruction loaded from memory into 'instruction' at every iteration
        unsigned opcode = (instruction >> 13) & 7; // store opcode (4 MSB)
        
        if (opcode == 0b000){ // add, sub, or, and, slt, jr -> THREE REGISTER INSTRUCTIONS
            threeRegInstr(instruction, regs, memory, pc, opcode);
        }
        else if ((opcode == 0b111)|(opcode == 0b110)|(opcode == 0b001)|(opcode == 0b100)|(opcode == 0b101)){
            twoRegInstr(instruction, regs, memory, pc, opcode, L1cache, L2cache, L1blocksize, L2blocksize);
            // slti, lw, sw, jeq, addi -> TWO REGISTER INSTRUCTIONS

        }
        else{ // j, jal -> NO REGISTER INSTRUCTIONS
            NoRegInstr(instruction, regs, memory, pc, opcode, endProgram);
        }
        regs[0] = 0;
    }

    // cout << "I am deleting the caches!" << endl;
    delete L1cache;
    delete L2cache;

    return 0;
}
