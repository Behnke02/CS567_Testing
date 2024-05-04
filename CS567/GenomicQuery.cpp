#include <iostream>
#include <sys/time.h>
#include <cmath>
#define QUERY_LENGTH 16

using namespace std;

class LLNode
{
    public:

    //First 12 characters of fragment for finding index
    char querySequence[13];
    //Last 4 characters of fragment in radix form for unique identifier
    unsigned int radixValue;
    //Pointer to next node in linked list
    LLNode *nextNode;

    //Default Constructor for node type
    //Sets all class variables to default values
    LLNode();

    //Initialization Constructor for node type
    //Sets all class variables to provided values
    LLNode(const char *sequence, unsigned int value);

    //Function to recursively deallocate linked list
    LLNode *clearList(LLNode *wkgPtr);
};

class HashLL
{   
    public:

    //Pointer to start of linked list
    LLNode* headPtr;

    //Constructor for linked list type
    //Sets headPtr to NULL
    HashLL();

    //Function to append node to end of linked list
    unsigned int insert(const LLNode &newNode);

    //Starter function to clear linked list
    void clearList();

    //Function to search for matching value in linked list
    LLNode *search(unsigned int searchVal);
};

class Queries_HT
{
    public:

    //Pointer to file containing query data
    FILE *queryFilePointer;

    //Total number of queries in hash table
    unsigned int numQueries;

    //Number of indices in hash table array
    unsigned int hashTableSize;

    //Hash table array of linked list pointers
    HashLL *hashArray;

    //Default Constructor for Hash Table class
    //Sets all class variables to default values
    Queries_HT();

    //Initialization Constructor for Hash Table class
    //Sets all class variables and initializes hash array
    Queries_HT(FILE *queryFile, int hashSize);

    //Destructor for Hash Table class
    //Deallocates memory from linked lists and hash array
    ~Queries_HT();

    //Function to search hash array for matching values
    bool searchHash(char *searchValue, unsigned int srcIndex);

    //Function to insert new sequence into hash table
    unsigned int insertSequence(char *newValue);

    //Function to convert character fragments into base-10 integer
    unsigned int convertToRadix(char *originalString, unsigned int index, unsigned int length);

    //Function to fill hash array using query data
    unsigned int fillHashes(bool timerFlag);

    private:

    //Function to find hash index for insertion/searching
    //Dependency: convertToRadix
    unsigned int findHashIndex(char *valueString, unsigned int index, unsigned int length);
};

int compareString(const char *oneStr, const char *otherStr);
unsigned int getStringLength(const char *testStr);
void copyString(char *destStr, const char *sourceStr);

HashLL::HashLL()
{
    //Set headPtr to NULL
    headPtr = NULL;
}

unsigned int HashLL::insert(const LLNode &newNode)
{
    //Allocate memory for new node
    LLNode *newPtr = new LLNode(newNode);

    //Check for list empty
    if(headPtr == NULL)
    {
        //Set head to new node
        headPtr = newPtr;
    }
    else
    {
        //Initialize working pointer
        LLNode *wkgPtr = headPtr;

        //Loop to end of linked list
        while(wkgPtr->nextNode != NULL)
        {
            wkgPtr = wkgPtr->nextNode;
        }
        //Append new node to end of list
        wkgPtr->nextNode = newPtr;

        //Return collision detected and resolved
        return 1u;
    }

    //Return no collision detected
    return 0u;
}

void HashLL::clearList()
{
    //Call recursive function to clear list
    (HashLL* )(headPtr->clearList(headPtr));
}

LLNode* HashLL::search(unsigned int searchVal)
{
    //Initialize working pointer
    LLNode *wkgPtr = headPtr;

    //Loop until match found or end of list
    while(wkgPtr != NULL)
    {
        //Check for matching value
        if(searchVal == wkgPtr->radixValue)
        {
            //Return pointer to matching node
            return wkgPtr;
        }
        //Move to next node
        wkgPtr = wkgPtr->nextNode;
    }
    //No match found, return NULL
    return NULL;
}

LLNode::LLNode()
{
    //Set all values to default
    querySequence[0] = '\0';
    radixValue = 0;
    nextNode = NULL;
}

LLNode::LLNode(const char* sequence, unsigned int value)
{
    //Set all values to provided data
    copyString(querySequence, sequence);
    radixValue = value;
    nextNode = NULL;
}

LLNode* LLNode::clearList(LLNode *wkgPtr)
{
    //Check for end of list
    if(nextNode != NULL)
    {
        //Recursively iterate to end of list
        nextNode = clearList(nextNode);
    }
    //Deallocate memory and return NULL
    delete wkgPtr;
    return NULL;
}

Queries_HT::Queries_HT()
{
    //Set all values to defaults
    queryFilePointer = NULL;
    numQueries = 0;
    hashTableSize = 0;
    hashArray = NULL;
}

Queries_HT::Queries_HT(FILE *queryFile, int hashSize)
{
    //Set all values to provided data where applicable
    queryFilePointer = queryFile;
    numQueries = 0;
    hashTableSize = hashSize;
    hashArray = new HashLL[hashTableSize];

    //Initialize linked lists in all hash indices
    for(unsigned int index = 0; index < hashTableSize; index++)
    {
        hashArray[index] = HashLL();
    }
}

Queries_HT::~Queries_HT()
{
    //Deallocate hash array
    delete[] hashArray;
}

bool Queries_HT::searchHash(char *genomeString, unsigned int srcIndex)
{
    //Using 12 characters, calculate radix int to get hash index
    //(Using all 16 in fragment could cause potential overflow issues)
    unsigned int radixValue = findHashIndex(genomeString, srcIndex, 12);


    //Search for matching radix value in table
    return (hashArray[radixValue].search(convertToRadix(genomeString, srcIndex + 12, 4)) != NULL);
}

unsigned int Queries_HT::insertSequence(char *newValue)
{

    //Using 12 characters, calculate radix int to get hash index
    //(Using all 16 in fragment could cause potential overflow issues)
    unsigned int radixValue = findHashIndex(newValue, 0, 12);

    //Create temp string for insertion in new node
    char hashHalf[13];
    hashHalf[12] = '\0';
    int stringIndex = 0;
    int index;
    for(index = 0; index < 12; index++, stringIndex++)
    {
        hashHalf[index] = newValue[stringIndex];
        hashHalf[index+1] = '\0';
    }

    //Insert new node at given hash index
    numQueries += 1;
    return hashArray[radixValue].insert(LLNode(hashHalf, convertToRadix(newValue, 12, 4)));
}

unsigned int Queries_HT::convertToRadix(char *originalString, unsigned int index, unsigned int length)
{
    //Initialize intermediate variables
    unsigned int finalValue = 0;
    int intermediateValue;
    int baseValue = 1;
    unsigned int counter = 0;
    
    //While provided length of fragment not reached
    do
    {
        //Check provided character and set base-5 value
        switch(originalString[index])
        {
            case 'N':
                intermediateValue = 0;
                break;
            case 'A':
                intermediateValue = 1;
                break;
            case 'T':
                intermediateValue = 2;
                break;
            case 'C':
                intermediateValue = 3;
                break;
            case 'G':
                intermediateValue = 4;
                break;
        }
        //Increment final base-10 value
        finalValue += intermediateValue * baseValue;
        //Multiply power variable
        baseValue *= 5;
        //Increment counters
        index++;
        counter++;
    } while (counter != length);

    //Return base-10 radix value
    return finalValue;
    
}

unsigned int Queries_HT::findHashIndex(char *valueString, unsigned int index, unsigned int length)
{
    //Convert valueString to radix form
    //Range fit resulting radix int to fit in hash table
    return (convertToRadix(valueString, index, length) % hashTableSize);
}

unsigned int Queries_HT::fillHashes(bool timerFlag)
{
    //Initialize variables
    unsigned int numCollisions = 0u;
    struct timeval searchStartTime, searchEndTime;
    int startSec, endSec, startUSec, endUSec;

    //Check for timer start
    if(timerFlag)
    {
        gettimeofday( &searchStartTime, NULL);
        startSec = searchStartTime.tv_sec;
        startUSec = searchStartTime.tv_usec;
    }

    //Set intChar to start of query file
    int intChar = fseek(queryFilePointer, 0, SEEK_SET);
    intChar = fgetc(queryFilePointer);
    int index = 0;
    char temp[17];
    temp[16] = '\0';

    //Loop until end of file
    while(intChar != EOF)
    {
        //Check for current character in alphabet
        if(intChar >= 65 && intChar <= 90)
        {
            temp[index] = (char)(intChar);
            index++;
        }

        //Check for full fragment
        if(index == 16)
        {
            numCollisions += insertSequence(temp);
            numQueries += 1;
            index = 0;
        }
        //Move to next character
        intChar = fgetc(queryFilePointer);
    }

    //Check for timer end
    if(timerFlag)
    {
        gettimeofday( &searchEndTime, NULL);
        endSec = searchEndTime.tv_sec;
        endUSec = searchEndTime.tv_usec;

        int secDiff = (endSec - startSec);
        int secUDiff = (endUSec = startUSec);

        cout << "It took " << secDiff << "." << secUDiff 
                    << " seconds to fill the hash table" << endl;
    }

    return numCollisions;
}

int compareString(const char *oneStr, const char *otherStr)
{
    //Initialize function/variables
    int diff, index = 0;

    //Loop to end of shortest string
    while(oneStr[index] != '\0'
         && otherStr[index] != '\0')
    {
        //Get difference in characters
        diff = oneStr[index] - otherStr[index];
        //Check for difference between characters
        if(diff != 0)
        {
            //Difference found, return difference
            return diff;
        }
        //Increment index
        index++;
    }
    //End Loop

    //Return difference in lengths if exists
        //Function: getStringLength
    return getStringLength(oneStr) - getStringLength(otherStr);
}

unsigned int getStringLength(const char *testStr)
{
    //Initialize function/variables
    unsigned int index = 0;

    //Loop to end of string
    while(testStr[index] != '\0')
    {
        //Update index
        index++;
    }
    //End Loop

    //Return index (Max string index is [index-1])
    return index;
}

void copyString(char *destStr, const char *sourceStr)
{
    //Initialize function/variables
    int index = 0;

    //Check that source string is not identical to dest string
    if(compareString(destStr, sourceStr) != 0)
    {
        //Loop to end of source string
        while(sourceStr[index] != '\0' && index < '\0')
        {
            //Assign characters to end of dest string
            destStr[index] = sourceStr[index];

            //Update Index
            index++;

            //set temp end of dest string
            destStr[index] = '\0';
        }
        //End Loop
    }
}

int main(int argc, char **argv)
{
    //Initialize variables
    FILE *genomeFile = fopen(argv[1], "r");
    FILE *queryFile = fopen(argv[2], "r");
    bool collisionTimerFlag = false;
    bool searchTimerFlag = false;

    char *genomeString = NULL;

    int argIndex = 3;

    int intChar;

    unsigned int numCollisions = 0;

    struct timeval searchStartTime, searchEndTime;
    int startSec, endSec, startUSec, endUSec;

    unsigned int numMatches = 0;

    while (argIndex < argc)
    {
        if(compareString(argv[argIndex], "-c") == 0)
        {
            collisionTimerFlag = true;
        }
        else if(compareString(argv[argIndex], "-s") == 0)
        {
            searchTimerFlag = true;
        }
        argIndex += 1;
    }

    //PART ONE

    if(genomeFile != NULL && queryFile != NULL)
    {

    //Populate each table with query dataset
    //Count number of collisions and time to populate each table
        cout << "Creating and filling hash table with size 60 million" << endl;
        Queries_HT sixtyMSize = Queries_HT(queryFile, 60000000);
        numCollisions = sixtyMSize.fillHashes(collisionTimerFlag);
        cout << numCollisions << " collisions were found populating a table of 60 million" << endl;

    //PART TWO

    //Read in genome
    
        cout << "Reading Genome File" << endl;
        unsigned int genomeLength = 0;
        unsigned int genomeIndex = 0;
        intChar = fgetc(genomeFile);
        while(intChar != EOF)
        {
            //Increment length of genome
            genomeLength += (intChar > 64 && intChar < 91);
            intChar = fgetc(genomeFile);
        }
        genomeString = new char[genomeLength+1];
        fseek(genomeFile, 0, SEEK_SET);
        intChar = fgetc(genomeFile);

        cout << "Filling Genome String" << endl;
        while(intChar != EOF)
        {
            if(intChar > 64 && intChar < 91)
            {
                //Increment length of genome
                genomeString[genomeIndex] = (char)(intChar);
                genomeIndex++;
                genomeString[genomeIndex+1] = '\0';
            }
            intChar = fgetc(genomeFile);
        }

        cout << "Searching Genome String" << endl;

        //For each 16-mer in genome, search for match in query table
        unsigned int numSubstrings = genomeLength - QUERY_LENGTH + 1;
        cout << numSubstrings << " Substrings to search" << endl;

        //Check for start timer
        if(searchTimerFlag)
        {
            gettimeofday( &searchStartTime, NULL);
            startSec = searchStartTime.tv_sec;
            startUSec = searchStartTime.tv_usec;
        }
        
        //Loop for total number of substrings in genome
        for(unsigned int index = 0; index < numSubstrings; index++)
        {
            //Check for successful search
            if(sixtyMSize.searchHash(genomeString, index))
            {
                //Increment number of found matches
                numMatches += 1;
                if(numMatches > 0 && numMatches < 16)
                {
                    unsigned int srcIndex = index;
                    char tempPrint[17];
                    tempPrint[16] = '\0';
                    for(unsigned int tempIndex = 0; tempIndex < 16; tempIndex++, srcIndex++)
                    {
                        tempPrint[tempIndex] = genomeString[srcIndex];
                    }
                    cout << "Fragment " << numMatches << " " << tempPrint << endl;
                }
            }
        }
        cout << numMatches << " matches found" << endl;

        //Check for end timer
        if(searchTimerFlag)
        {
            gettimeofday( &searchEndTime, NULL);
            endSec = searchEndTime.tv_sec;
            endUSec = searchEndTime.tv_usec;

            int secDiff = (endSec - startSec);
            int secUDiff = (endUSec - startUSec);

            cout << "It took " << secDiff << "." << secUDiff 
                    << " seconds to search the hash table" << endl;
        }

        cout << "Clearing Hash" << endl;
    }
    else
    {
        cout << "Genome or Query File not found" << endl;
        cout << "Please check your file names and try again" << endl;
        return 1;
    }
    cout << "Program End" << endl;
    return 0;
}
