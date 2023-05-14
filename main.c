#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*
This is your cache lab. It consists of implementing a cache simulator given various parameters, and then watching it perform under different circumstances. To make things simple, you'll only be dealing with reads.

When your simulator starts, it should take from stdin (so no files are opened/closed/used in the project code) input in the following format:

The first four lines contain the fundamental parameters of the cache, one per line. All four are integers:
S
E
B
m
The next line contains the replacement policy. The only ones you need are LFU and LRU. The replacement policy will be entered as either LFU or LRU, so read three chars here.

The next two lines contains the fundamental parameters of the hardware: the hit time (h) and the miss penalty (p). Both are integers:
h
p
That was the input. The runtime information follows immediately. All further lines contain integer addresses that your simulation attempts to read from main memory through the cache, one per line. The simulation stops when the "address" you read is -1.

You should output all information to stdout in the following format:

For each address you read (except the final -1), output a line containing the address itself (in hex form), followed by a space, followed by a single character: H for cache hit or M for cache miss.

Once you read -1, output on a single line the miss rate of the cache that you have calculated, followed by a space, followed by the total cycles spent during the simulation. Remember to complete this last output line with a newline character, then exit the simulation.

Grading policy for the homework: 5 for correct implementation of a direct-mapped cache, 5 for correct implementation of a set associative cache, 5 for correct implementation of a fully associative cache.


Input 3 for testing:

4
2
8
8
LRU
1
30
20
22
40
80
24
82
-1
 */


struct cache;
struct set;
struct line;

typedef struct {
    int valid;
    int tag;
    int cyclesSinceUsed;
    int* accessed;
} line;

char replacementPolicy = 'r';
int frequencyRange = 100;
int hitTime = 0;
int missPenalty = 0;
int totalMisses = 0;
int totalInputs = 0;
int totalCycles = 0;

typedef struct {
    int numLines;
    line** lines;
} set;

typedef struct {
    set** sets;
    int numberOfSets;
    int numberOfTagBits;
    int numberOfSetBits;
} cache;

int sumArray(int* numbers, int length) {
    int sum = 0;
    for (int i = 0; i < length; i++)
        sum = sum + numbers[i];
    return sum;
}

void shiftAndAddAccessed(line* l, int addition) {
    for (int i = 0; i < frequencyRange - 1; i++) {
        l->accessed[i] = l->accessed[i+1];
    }
    l->accessed[frequencyRange-1] = addition;
}

void loadAddress(cache* c, int setID, int tagID) {
    set* s = c->sets[setID];
    line** lines = s->lines;
    for (int i = 0; i < s->numLines; i++) {
        if (lines[i]->valid != 1) {
            lines[i]->valid = 1;
            lines[i]->tag = tagID;
            if (replacementPolicy == 'R')
                lines[i]->cyclesSinceUsed = 0;
            else {
                for (int j = 0; j < frequencyRange - 1; j++) {
                    lines[i]->accessed[j] = 0;
                }
                lines[i]->accessed[frequencyRange-1] = 1;
            }
            return;
        }
    }
    if (replacementPolicy == 'R') {
        int LRUIndex = 0;
        for (int i = 0; i < s->numLines; i++) {
            if (lines[i]->cyclesSinceUsed > lines[LRUIndex]->cyclesSinceUsed)
                LRUIndex = i;
        }
        lines[LRUIndex]->tag = tagID;
        lines[LRUIndex]->cyclesSinceUsed = 0;
    } else {
        int LFUIndex = 0;
        for (int i = 0; i < s->numLines; i++) {
            int sumI = sumArray(lines[i]->accessed, frequencyRange);
            int sumIndex = sumArray(lines[LFUIndex]->accessed, frequencyRange);
            if (sumI < sumIndex) {
                LFUIndex = i;
            }
        }
        lines[LFUIndex]->tag = tagID;
        for (int i = 0; i < frequencyRange - 1; i++) {
            lines[LFUIndex]->accessed[i] = 0;
        }
        lines[LFUIndex]->accessed[frequencyRange - 1] = 1;
    }
}

line* createLine() {
    line* lin = (line*)malloc(sizeof(line));
    lin->accessed = malloc(sizeof(int) * frequencyRange);
    lin->valid = 0;
    return lin;
}

set* createSet(int e, int b, int m) {
    set* set1 = (set*)malloc(sizeof(set));
    set1->numLines = e;
    set1->lines = malloc(sizeof(set)*e);
    for (int i = 0; i < e; i++) {
        set1->lines[i] = createLine();
    }
    return set1;
}

cache* createCache(int s, int e, int b, int m) {
    cache* c = (cache*)malloc(sizeof(cache));
    c->sets = malloc(sizeof(set)*s);
    int numSetBits = ceil(log2(s));
    c->numberOfSetBits = numSetBits;
    c->numberOfTagBits = m - (numSetBits + (ceil(log2(b))));
    c->numberOfSets = s;
    for (int i = 0; i < s; i++) {
        c->sets[i] = createSet(e, b, m);
    }
    return c;
}


int convertToBase2(int number) {
    int sum = 0;
    int multiplier = 1;

    while (number != 0) {
        int remainder = number % 2;
        number = number / 2;
        sum = sum + multiplier * remainder;
        multiplier = multiplier * 10;
    }
    return sum;
}


int hexDigit(char c) {
    int result = 0;
    switch (c) {
        case 'a':
        case 'A':
            result = 10;
            break;
        case 'b':
        case 'B':
            result = 11;
            break;
        case 'c':
        case 'C':
            result = 12;
            break;
        case 'd':
        case 'D':
            result = 13;
            break;
        case 'e':
        case 'E':
            result = 14;
            break;
        case 'f':
        case 'F':
            result = 15;
            break;
        default:
            if (isdigit(c))
                result = 1;
            break;
    }
    return result;
}



int parseHex(char* hex, int length) {
    int sum = 0;
    int multiplier = 16*(length - 1);

    for (int i = 0; i < length; i++) {
        int value = hexDigit(hex[i]);
        if (value == 1)
            value = hex[i] - '0';
        sum = sum + value*multiplier;
        multiplier = multiplier/16;
    }
    return sum;
}

int numDigits(int x) {
    return (x == 0 ? 1 : (int)(log10(x)+1));
}



int read(cache* c, int address, int addressSize) {
    int value = 0;
    int length = numDigits(address);
    int digits[addressSize];
    for (int i = 0; i < addressSize - length; i++){
        digits[i] = 0;
    }

    int subtract = 0;
    for (int i = 0; i < length; i++) {
        int x = address;
        for (int j = 0; j < length-i-1; j++) {
            x = x/10;
        }
        x = x - subtract;
        subtract = (subtract + x) * 10;
        digits[addressSize-length+i] = x;
    }
    int tagBits = c->numberOfTagBits;
    int setBits = c->numberOfSetBits;
    int tagID = 0;
    int setID = 0;
    int multiplier = 1;

    for (int i = 0; i < tagBits; i++) {
        tagID = tagID + digits[i]*multiplier;
        multiplier = multiplier*2;
    }
    multiplier = 1;
    for (int i = 0; i < setBits; i++) {
        setID = setID + digits[tagBits + i]*multiplier;
        multiplier = multiplier*2;
    }
    set* set1 = c->sets[setID];
    for (int i = 0; i < set1->numLines; i++) {
        set1->lines[i]->cyclesSinceUsed++;
        if (set1->lines[i]->valid) {
            if (set1->lines[i]->tag == tagID) {
                set1->lines[i]->cyclesSinceUsed = 0;
                shiftAndAddAccessed(set1->lines[i], 1);
                value = 1;
            } else {
                shiftAndAddAccessed(set1->lines[i], 0);
            }
        }
    }
    if (value){
        return value;
    }

    loadAddress(c, setID, tagID);
    return 0;
}



int main() {
    int setsS;
    int linesE;
    int blockSizeB;
    int addressSize;

    char input[20];

    int digits;
    scanf("%d", &setsS);
    scanf("%d", &linesE);
    scanf("%d", &blockSizeB);
    scanf("%d", &addressSize);

    char repPolicy[3];
    scanf("%s", repPolicy);
    if (repPolicy[1] == 'R') {
        replacementPolicy = 'R';
    }

    scanf("%d", &hitTime);

    scanf("%d", &missPenalty);

    cache* c = createCache(setsS, linesE, blockSizeB, addressSize);

    int expectedDigits = addressSize/4;
    char *address = malloc(sizeof (char)*expectedDigits+1);

    scanf("%s", address);
    printf("\n");
    while (!(address[0] == '-' && address[1] == '1')){
        totalInputs++;
        int base10 = parseHex(address, expectedDigits);
        int base2 = convertToBase2(base10);

        if (read(c, base2, addressSize)){
            totalCycles = totalCycles +hitTime;
            printf("%s", address);
            printf(" H\n");
        } else {
            totalMisses++;
            totalCycles = totalCycles + hitTime + missPenalty;
            printf("%s", address);
            printf(" M\n");
        }
        scanf("%s", address);
    }
    printf("%d %d\n",totalMisses*100/totalInputs,totalCycles);
    free(address);
    return 0;
}