#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 512 * sizeof(char)
#define ALLIGN 512

#ifdef NO_CACHE
#define FLAG_READ O_RDWR | O_DIRECT
#define FLAG_WRITE O_RDWR | O_CREAT | O_DIRECT
#else
#define FLAG_READ O_RDWR
#define FLAG_WRITE O_RDWR | O_CREAT
#endif


char* getFileName(char* filename, int index) 
{
    char strIndex[3];
    snprintf(strIndex, sizeof(strIndex), "%d", index);

    char* result = (char*)malloc(strlen(filename) + strlen(strIndex) + strlen(".txt") + 1);
    strcpy(result, filename);
    strcat(result, strIndex);
    strcat(result, ".txt");
    return result;
}

int openFile(char* filename, int mode)
{
    int file = open(filename, mode);
    if(file != -1)
    {
        
        printf("File: ");
        printf(filename);
        printf(" is open\n");
        return file;
    }
    else 
    {
        printf("File: ");
        printf(filename);
        printf(" is not open\n");
        return 0;
    }
}

void closeFile(int file, char* filename)
{
    #ifdef CLEAR_CACHE
    posix_fadvise(file, 0, 0, POSIX_FADV_DONTNEED);
    #endif
    int error = close(file);
    if (error < 0)
    {
        printf("File error: ");
        printf("%d", error);
        printf("\n");
    }
    else 
    {
        printf("File: ");
        printf(filename);
        printf(" is close\n");
    }
}

void printError(char* error)
{
	printf("Error ");
    perror(error);
    printf(" ");
    printf("%d", errno);
    printf("\n");
}

void* getBuffer(size_t size) 
{
    void * buffer = 0;
    #ifdef NO_CACHE
    if (posix_memalign(&buffer, ALLIGN, size))
    {
    	printf("%d", errno);
        printf("\n");
    	return 0;
    }
    else 
    {
        return buffer;
    }
    #else
    buffer = malloc(size);
    if(!buffer)
    {
        printf("%d", errno);
        printf("\n");
    	return 0;
    }
    else 
    {
        return buffer;
    }
    #endif
}

void processFile(int index)
{
    char* filename1 = getFileName("testFile_", index);
    char* filename2 = getFileName("testFile_", index + 1);

    int oldFile = openFile(filename1, FLAG_READ);
    int newFile = openFile(filename2, FLAG_WRITE);

    if(oldFile && newFile)
    {
    	void * buffer = getBuffer(BUFFER_SIZE);
        if (buffer)
        {
            while(int rResult = read(oldFile, buffer, 512))
            {
                if(rResult < 0)
                {
                    printError("read");
                    closeFile(oldFile, filename1);
                    closeFile(newFile, filename2);
                    free(filename1);
                    free(filename2);
                    free(buffer);
                    return;
                }
                int wResult = write(newFile, buffer, 512);
                if(wResult < 0)
                {
                    printError("Write");
                    closeFile(oldFile, filename1);
                    closeFile(newFile, filename2);
                    free(filename1);
                    free(filename2);
                    free(buffer);
                    return;
                }
            }
            closeFile(oldFile, filename1);
            closeFile(newFile, filename2);
            free(buffer);
        }
    }

    free(filename1);
    free(filename2);
}

int main(int argc, char** argv)
{
    if (argc < 2)
        return 1;

    for (int i = 0; i < atoi(argv[1]); ++i) 
    {
        processFile(i);
    }
    return 0;
}
