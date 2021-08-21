#include <stdlib.h>
#include <stdio.h>
#include <wordexp.h>

FILE* fileOpen(const char* filename, const char* openmode)
{
    wordexp_t expandedName;
    FILE* fileHandle; // output points to file to write calculate results to
    wordexp(filename, &expandedName, 0);
    fileHandle = fopen(expandedName.we_wordv[0], openmode);
    wordfree(&expandedName);
    return fileHandle;

}
