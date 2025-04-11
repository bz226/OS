#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define VIRTUAL_ADDRESS 16
#define PHYSICAL_ADDRESS 10


typedef struct {
    unsigned int valid;
    unsigned int referenced;
    unsigned int modified;
    unsigned int frameNumber;
} PageTableEntry;


void initializeEntry(PageTableEntry *pageTable, int i) {
    pageTable[i].valid = 0;
    pageTable[i].referenced = 0;
    pageTable[i].modified = 0;
    pageTable[i].frameNumber = -1;
}

/*
   NRU replacement:
   - We look at all physical frames, checking the pages in them.
   - For each page, form a “class” by (2*R + M).
   - Keep track of the page with the lowest class.
   - If multiple have the same class, choose the one in the lowest physical frame index.
   - After we have a victim, we replace it with the new VPN.
   - Mark the old page’s entry invalid.
*/
void NRU(PageTableEntry *pageTable, int physicalPages, int *physicalMemory, int vpn) {
    int victimIndex = -1;
    int lowestClassFound = 4; 

    for (int i = 0; i < physicalPages; i++) {
        int currentVPN = physicalMemory[i];
        int classVal = 2 * pageTable[currentVPN].referenced + pageTable[currentVPN].modified;

        if (classVal < lowestClassFound ||
            (classVal == lowestClassFound && i < victimIndex)) {
            victimIndex = i;
            lowestClassFound = classVal;
        }
    }

    int victimVPN = physicalMemory[victimIndex];
    physicalMemory[victimIndex] = vpn;
    initializeEntry(pageTable, victimVPN);
}


int periodic(int num, int increment, PageTableEntry *pageTable, int virtualPageCount) {
    if (increment == num) {
        for (int j = 0; j < virtualPageCount; j++) {
            pageTable[j].referenced = 0;
        }
        increment = 0;
    }
    return increment;
}


void print_result(int readCount, int writeCount, double faultPercent,
                  int physicalPages, int *physicalMemory) {
    printf("readcounts =%d\n", readCount);
    printf("writecounts =%d\n", writeCount);
    printf("percentage of page faults %.2f\n", faultPercent);

    for (int i = 0; i < physicalPages; i++) {
        if (physicalMemory[i] != -1) {
            printf("mem[%d]:%x\n", i, physicalMemory[i]);
        } else {
            printf("mem[%d]:ffffffff\n", i);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <tracefile> <page_size> <rbit_reset_interval>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        fprintf(stderr, "Error: cannot open file %s\n", argv[1]);
        return 1;
    }

    int pageSize = atoi(argv[2]);
    int offsetBits = (int)log2(pageSize);

    int num = atoi(argv[3]);

    int virtualPageCount = 16;
    int physicalPageCount = 10;

    int *all_vpn = NULL;
    int *all_op = NULL;  
    int count = 0;

    int readCount = 0;
    int writeCount = 0;

    int address;
    int op;

    while (fscanf(fp, "%x %d", &address, &op) == 2) {
        all_vpn = realloc(all_vpn, sizeof(int) * (count + 1));
        all_op = realloc(all_op, sizeof(int) * (count + 1));

        all_vpn[count] = address >> offsetBits;
        all_op[count] = op;

        if (op == 1) {
            writeCount++;
        } else {
            readCount++;
        }
        count++;
    }
    fclose(fp);

    PageTableEntry *pageTable = malloc(virtualPageCount * sizeof(PageTableEntry));
    for (int i = 0; i < virtualPageCount; i++) {
        initializeEntry(pageTable, i);
    }

    int *physicalMemory = malloc(physicalPageCount * sizeof(int));
    for (int i = 0; i < physicalPageCount; i++) {
        physicalMemory[i] = -1;
    }

    int incr = 0;     // increments each memory access
    int faults = 0;   // how many page faults happened

    // Process each memory access
    for (int i = 0; i < count; i++) {
        int vpn = all_vpn[i];
        int operation = all_op[i];  // 0 = read, 1 = write

        // If already in memory
        if (pageTable[vpn].valid) {
            // Set R=1
            pageTable[vpn].referenced = 1;
            // If write, set M=1
            if (operation == 1) {
                pageTable[vpn].modified = 1;
            }
        } else {
            // Page fault
            faults++;
            // Find empty slot if any
            int foundEmpty = 0;
            for (int j = 0; j < physicalPageCount; j++) {
                if (physicalMemory[j] == -1) {
                    // Fill it
                    physicalMemory[j] = vpn;
                    foundEmpty = 1;
                    break;
                }
            }
            // If none found, do NRU
            if (!foundEmpty) {
                NRU(pageTable, physicalPageCount, physicalMemory, vpn);
            }
            // Mark this page valid
            pageTable[vpn].valid = 1;
            pageTable[vpn].referenced = 1;
            if (operation == 1) {
                pageTable[vpn].modified = 1;
            }
        }

        incr++;
        // Possibly reset R bits after “num” accesses
        incr = periodic(num, incr, pageTable, virtualPageCount);
    }

    // Calculate the fraction of accesses that caused page faults
    double faultPercent = (double)faults / count;

    // Print final stats
    print_result(readCount, writeCount, faultPercent, physicalPageCount, physicalMemory);

    // // Cleanup
    // free(all_vpn);
    // free(all_op);
    // free(pageTable);
    // free(physicalMemory);

    return 0;
}
