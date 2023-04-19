#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>
#include <libgen.h>


// Global flags
int showHidden = 0;
int showDirectoriesOnly = 0;
int showFilesOnly = 0;
int showPermissions = 0;
int showModificationTime = 0;
int showSize = 0;
int showUID = 0;
int showGID = 0;
int showInodes = 0;
int sortByName = 0;
int sortByTime = 0;
int showDirsFirst = 0;
int maxDepth = -1;




struct Entry {
    char name[256];
    int type;
    time_t mtime;
};

int MAX_ENTRIES = 100;
int TYPE_DIRECTORY = 2; 
int TYPE_FILE =  1; 

struct Entry entries[1000];
int num_entries = 0;


struct Directory {
    int id; 
    struct Entry entries[100];
    int num_entries;
};

struct Directory directories [1000]; 
int num_directories = 0;


void parseTree(const char *path, int indent); 
bool isDirectory(const char *filename); 
void printHelp(); 
void printFile(const char *filename, int depth);   
void printSize(off_t size); 
void printPermissions(mode_t mode); 
void printFile(const char *filename, int depth); 
void printDirectoriesRec(const char *path, int depth); 
time_t getFileModificationTime(const char *path); 



