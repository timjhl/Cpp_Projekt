#ifndef BATCHGENERATOR_H
#define BATCHGENERATOR_H
#include "header.h"

using namespace std;

class BatchGenerator {
private:
    string outputfile;
    bool hideshell;
    string application;
    vector<Entry> entries;
    bool firstIt = true;
public:
    BatchGenerator();
    bool readJsonFile(const std::string& filename);
    bool parseCommandLine(int argc, char *argv[]);
    void generateBatchFile();
    void printHelp();
    bool keyLineCount(const std::string& filename, const std::string& key, size_t& lineCount);
    
}; // end of class BatchGenerator

#endif // BATCHGENERATOR_H