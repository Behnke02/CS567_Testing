

#include <deepstate/DeepState.hpp>

using namespace deepstate;

#include "GenomicQueryExMain.cpp"
#include <fcntl.h>
#include <fstream>

#define MAX_GENOME_LENGTH 1000000000
#define MAX_STR_LEN 17
#define INDEX_LENGTH 12
#define IDENTIFIER_LENGTH 4

TEST(String, StringLength)
{
    char *string = DeepState_CStr_C(MAX_STR_LEN, "ACGTN");
    ASSERT_EQ(getStringLength(string), MAX_STR_LEN);
}

TEST(String, CompareStings)
{
    ASSERT_EQ(compareString("", ""), 0);
    ASSERT_EQ(compareString("ATGGCGGTCCAGAAT", "ATGGCGGTCCAGAAT"), 0);
}

TEST(Hash, RadixConvert)
{
    Queries_HT TestHash;
    char *zeroString = DeepState_CStr_C(MAX_STR_LEN, "N");
    ASSERT_EQ(TestHash.convertToRadix(zeroString, 0, INDEX_LENGTH), 0u);
}

TEST(Hash, FindIndex)
{
    Queries_HT TestHash;
    TestHash.numQueries = 1;
    char *zeroString = DeepState_CStr_C(MAX_STR_LEN, "N");
    ASSERT_EQ(TestHash.findHashIndex(zeroString, 0, IDENTIFIER_LENGTH), 0u);
}

TEST(Program, Construction)
{
    int fd;
    FILE *queryFile;
    fd = open("trimmedQuery.txt", O_RDONLY | O_EXCL);

    ASSERT_NE(fd, -1);
    queryFile = fopen("trimmedQuery.txt", "r");
    Queries_HT sixtyMSize = Queries_HT(queryFile, 60000000);
    unsigned int numCollisions = sixtyMSize.fillHashes(false);
    ASSERT_GT(sixtyMSize.numQueries, 0);

    fclose(queryFile);
 
}

TEST(Program, Execution)
{
    int randomQuery = DeepState_MinInt(1);
    int randomGenomeLength = DeepState_MinInt(QUERY_LENGTH);
    char **queries = (char**)malloc(sizeof(char*) * randomQuery);
    char *genome = (char*)malloc(sizeof(char) * randomGenomeLength);
    std::ofstream tempQueryFile("testQueryFile.txt", std::ofstream::binary);
    std::ofstream tempGenomeFile("testGenomeFile.txt", std::ofstream::binary);
    for(int index = 0; index < randomQuery; index++)
    {
        queries[index] = (char*)malloc(sizeof(char) * QUERY_LENGTH);
        queries[index] = DeepState_CStr_C(QUERY_LENGTH, "ACGNT");
        tempQueryFile.write(queries[index], sizeof(char)*QUERY_LENGTH);
    }
    genome = DeepState_CStr_C(randomGenomeLength, "ACGNT");
    tempGenomeFile.write(genome, sizeof(char)*randomGenomeLength);

    tempGenomeFile.close();
    tempQueryFile.close();

    FILE *queryFile = fopen("testQueryFile.txt", "r");
    FILE *genomeFile = fopen("testGenomeFile.txt", "r");

    Queries_HT sixtyMSize = Queries_HT(queryFile, 60000000);
    unsigned int numCollisions = sixtyMSize.fillHashes(false);

    ASSERT_GE(sixtyMSize.numQueries, randomQuery) << sixtyMSize.numQueries << randomQuery;

    unsigned int genomeLength = 0;
        unsigned int genomeIndex = 0;
        int intChar = fgetc(genomeFile);
        while(intChar != EOF)
        {
            //Increment length of genome
            genomeLength += (intChar > 64 && intChar < 91);
            intChar = fgetc(genomeFile);
        }
        ASSERT_EQ(genomeLength, randomGenomeLength) << randomGenomeLength;
        char *genomeString = new char[genomeLength+1];
        fseek(genomeFile, 0, SEEK_SET);
        intChar = fgetc(genomeFile);

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

    unsigned int numSubstrings = genomeLength - QUERY_LENGTH + 1;
    unsigned int numMatches = 0;

    for(unsigned int index = 0; index < numSubstrings; index++)
        {
            //Check for successful search
            if(sixtyMSize.searchHash(genomeString, index))
            {
                //Increment number of found matches
                numMatches += 1;
            }
        }

    ASSERT_GT(numMatches, 0);

    fclose(genomeFile);
    fclose(queryFile);

    free(genome);
    for(int index = 0; index < randomQuery; index++)
    {
        free(queries[index]);
    }
    free(queries);
}

/*
TEST(Program, Execution)
{
    int fdQuery, fdGenome;
    fdQuery = open("trimmedQuery.txt", O_RDONLY | O_EXCL);
    fdGenome = open("trimmedGenome.txt", O_RDONLY | O_EXCL);

    ASSERT_NE(fdQuery, -1);
    ASSERT_NE(fdGenome, -1);

    FILE *queryFile = fopen("trimmedQuery.txt", "r");
    FILE *genomeFile = fopen("trimmedGenome.txt", "r");

    Queries_HT sixtyMSize = Queries_HT(queryFile, 60000000);
    unsigned int numCollisions = sixtyMSize.fillHashes(false);

    unsigned int genomeLength = 0;
        unsigned int genomeIndex = 0;
        int intChar = fgetc(genomeFile);
        while(intChar != EOF)
        {
            //Increment length of genome
            genomeLength += (intChar > 64 && intChar < 91);
            intChar = fgetc(genomeFile);
        }
        ASSERT_GT(genomeLength, 0);
        char *genomeString = new char[genomeLength+1];
        fseek(genomeFile, 0, SEEK_SET);
        intChar = fgetc(genomeFile);

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

    unsigned int numSubstrings = genomeLength - QUERY_LENGTH + 1;
    unsigned int numMatches = 0;

    for(unsigned int index = 0; index < numSubstrings; index++)
        {
            //Check for successful search
            if(sixtyMSize.searchHash(genomeString, index))
            {
                //Increment number of found matches
                numMatches += 1;
            }
        }

    ASSERT_GT(numMatches, 0);

    fclose(genomeFile);
    fclose(queryFile);

}
*/