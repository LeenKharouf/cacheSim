CAO Project 2

Leen Kharouf  
leenkharouf@nyu.edu  

State of Work: Completed. Passes all test cases and additional tests have been conducted.

Resources Used in Developing the Program:

- Online compiler and debugger
- E20 manual
- Visual Studio Code



Design Decisions:

- Queues: I used queues as my data structure to simulate the LRU eviction process

- Dynamic Memory Management: 
	Using pointers for dynamic memory allocation and flexible object creation. Allows me to pass pointers to objects to prevent making different copies, and instead updating the same one.

- Pointer Considerations: 
	Checks for null pointers to enhance program reliability and prevent segmentation faults.


Strengths:


1) Comments: The code is well-commented, making it easy for anyone to understand.


2) Flexibility: Dynamic memory allocation and pointers allow flexibility when creating objects

3) Efficient Memory Usage: Storing pointers in the queue enhances memory efficiency, particularly with large data structures.



Weaknesses:

- The program is only a simulation of how Caches work - the hits and misses are accurate but the valid bit was not needed in my program to simulate the Cache, but could have been implemented better 


- Complexity: Memory management with pointers requires careful attention to ensure proper allocation and deallocation, which can cause errors
