#include "../includes/header.h"
#include "../includes/includes.h"
#include "../includes/BatchGenerator.h"


int main(int argc, char *argv[]) 
{
    BatchGenerator batchGenerator;
    batchGenerator.parseCommandLine(argc, argv);
    return 0;
}   