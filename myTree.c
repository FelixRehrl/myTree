        #include "myTree.h"

        void printIndentation(int indent) {
            for (int i = 0; i < indent; i++) {
                printf("    "); // Use 4 spaces for each level of indentation
            }
        }

        bool isDirectory(const char *filename) {
            struct stat st;
            if (stat(filename, &st) == 0) {
                if (S_ISDIR(st.st_mode)) {
                    return true;
                } else {
                    return false;
                }
            } else {
                printf("Failed to stat file %s\n", filename);
                return false;
            }
        }

        void printHelp() {
            printf("Usage: myTree [options] <path>\n");
            printf("Options:\n");
            printf("  --help: Show help message\n");
            printf("  -a: Show hidden files and directories\n");
            printf("  -d: Show directories only\n");
            printf("  -f: Show files only\n");
            printf("  -p: Show permissions\n");
            printf("  -s: Show sizes\n");
            printf("  -u: Show UID\n");
            printf("  -g: Show GID\n");
            printf("  -D: Show device and inode numbers\n");
            printf("  --inodes: Show inode numbers only\n");
            printf("  -r: Sort entries in reverse order\n");
            printf("  -t: Sort entries by modification time\n");
            printf("  --dirsfirst: Sort directories first\n");
            printf("  -L <level>: Limit the depth of the tree to <level>\n");
        }

        void printPermissions(mode_t mode) {
            printf((S_ISDIR(mode)) ? "d" : "-");
            printf((mode & S_IRUSR) ? "r" : "-");
            printf((mode & S_IWUSR) ? "w" : "-");
            printf((mode & S_IXUSR) ? "x" : "-");
            printf((mode & S_IRGRP) ? "r" : "-");
            printf((mode & S_IWGRP) ? "w" : "-");
            printf((mode & S_IXGRP) ? "x" : "-");
            printf((mode & S_IROTH) ? "r" : "-");
            printf((mode & S_IWOTH) ? "w" : "-");
            printf((mode & S_IXOTH) ? "x" : "-");
        }

        void printSize(off_t size) {
            const char *units[] = {"B", "KB", "MB", "GB", "TB"};
            int i = 0;
            while (size > 1024 && i < sizeof(units) / sizeof(units[0])) {
                size /= 1024;
                i++;
            }
            printf("%ld %s", (long)size, units[i]);
        }

        time_t getFileModificationTime(const char *path) {
         struct stat statbuf;
            if (stat(path, &statbuf) == -1) {
            // Error occurred, handle it
            perror("stat");
            return -1;
            }
            return statbuf.st_mtime;
        }

       // Takes a filepath in input and prints the name of the file plus 
       // the additional data according to the flags given
       void printFile(const char *filename, int depth) {
            struct stat st;
             char *filename_copy = strdup(filename);
            if (stat(filename, &st) == 0) {
            // Print indentation with | and - characters
            for (int i = 0; i < depth - 1; i++) {
            printf("|   ");
            }

            if (depth > 0) {
            printf("|-- ");
           }
        
            printf("%s", basename(filename_copy));

            if (showPermissions) {
                printf("   ");
                printPermissions(st.st_mode);
             }

            if (showUID) {
                printf("   %d", st.st_uid);
            }

            if (showGID) {
                printf("   %d", st.st_gid);
            }

            if (showSize) {
                printf("   ");
                printSize(st.st_size);
            }

            if( showModificationTime) {
                printf("   %ld", st.st_mtime);
            }

            if (showInodes) {
                printf("   %ld ", (long)st.st_ino);
            }
            } else {
            printf("Failed to stat file %s\n", filename);
             }

            printf("\n");

            free(filename_copy);
       }
  

void printDirectoriesRec(const char *path, int depth) {

    // Reading the directory
    DIR* dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    // The idea is to create a Directory structure for each directory in which we 
    // store all the etnries we come across. This way, we can sort the directories 
    // without the recursive call to mess up the pathing.
    struct Directory * directory = &directories[num_directories];
    directory->id = num_directories;
    directory->num_entries = 0;

    struct dirent* entry;
    // Read entries in the directory
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".." entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construct full path of the entry
        char fullpath[PATH_MAX];
        snprintf(fullpath, PATH_MAX, "%s/%s", path, entry->d_name);

        // Check if entry is a directory
        if (isDirectory(fullpath)) {
            
                struct Entry* entry_info = &directory->entries[directory->num_entries];
                strcpy(entry_info->name, entry->d_name);
                entry_info->type = TYPE_DIRECTORY;
                entry_info->mtime = getFileModificationTime(fullpath); // Get directory modification time
                directory->num_entries++;
            

        } else {
            // Entry is a file
            if (!showDirectoriesOnly) {
                struct Entry* entry_info = &directory->entries[directory->num_entries];
                strcpy(entry_info->name, entry->d_name);
                entry_info->type = TYPE_FILE;
                entry_info->mtime = getFileModificationTime(fullpath); // Get file modification time
                directory->num_entries++;
            }
        }
    }
    closedir(dir);


    ///////////////// SORT DÍRECTORIES ////////////////////

    if (sortByName) {
        // Sort entries by name with bubble sort
        for (int i = 0; i < directory->num_entries - 1; i++) {
            for (int j = 0; j < directory->num_entries - i - 1; j++) {
                if (strcmp(directory->entries[j].name, directory->entries[j + 1].name) > 0) {
                    struct Entry temp = directory->entries[j];
                    directory->entries[j] = directory->entries[j + 1];
                    directory->entries[j + 1] = temp;
                }
            }
        }
    } else if (sortByTime) {
        // Sort entries by modification time with bubble sort
        for (int i = 0; i < directory->num_entries - 1; i++) {
            for (int j = 0; j < directory->num_entries - i - 1; j++) {
                if (directory->entries[j].mtime > directory->entries[j + 1].mtime) {
                    struct Entry temp = directory->entries[j];
                    directory->entries[j] = directory->entries[j + 1];
                    directory->entries[j + 1] = temp;
                }
            }
        }
    }

    
    //////////////////////////// PRINT FILES ////////////////////////////

    ///// DIRSFIRST  ///////
    if (showDirsFirst) {
        // Print directories first
        for (int i = 0; i < directory->num_entries; i++) {
            char fullpath[PATH_MAX];
            snprintf(fullpath, PATH_MAX, "%s/%s", path, directory->entries[i].name);

            if (directory->entries[i].type == TYPE_DIRECTORY) {
                if(!showFilesOnly) { printFile(fullpath, depth); }
                num_directories++;
                printDirectoriesRec(fullpath, depth + 1);
            }
        }

        // Print files after directories
        for (int i = 0; i < directory->num_entries; i++) {
            char fullpath[PATH_MAX];
            snprintf(fullpath, PATH_MAX, "%s/%s", path, directory->entries[i].name);

            if (directory->entries[i].type == TYPE_FILE) {
                printFile(fullpath, depth);
            }
        }

    /////// PRINT IN USUAL ORDER /////// 
    } else {

        // Order of files printing doesn't matter
        for (int i = 0; i < directory->num_entries; i++) {

            char fullpath[PATH_MAX];
            snprintf(fullpath, PATH_MAX, "%s/%s", path, directory->entries[i].name);

            if (directory->entries[i].type == TYPE_DIRECTORY) {
                // Check if we only want to show files
                if(!showFilesOnly) {printFile(fullpath, depth); }
                num_directories++;
                printDirectoriesRec(fullpath, depth + 1);

            } else if (directory->entries[i].type == TYPE_FILE) {
                printFile(fullpath, depth);
            }
        }

    }
        
        
        }





int main(int argc, char const *argv[]) {
        
        const char *path = "/home/felix/Documents/C";

        // Handle all the options
        for (int i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-help") == 0) {
                    printHelp();
                    return 0;
                } else if (strcmp(argv[i], "-a") == 0) {
                    showHidden = 1;
                } else if (strcmp(argv[i], "-d") == 0) {
                    showDirectoriesOnly = 1;
                } else if (strcmp(argv[i], "-f") == 0) {
                    showFilesOnly = 1;
                } else if (strcmp(argv[i], "-p") == 0) {
                    showPermissions = 1;
                } else if (strcmp(argv[i], "-s") == 0) {
                    showSize = 1;
                } else if (strcmp(argv[i], "-u") == 0) {
                    showUID = 1;
                } else if (strcmp(argv[i], "-g") == 0) {
                    showGID = 1;
                } else if (strcmp(argv[i], "-D") == 0) {
                    showModificationTime = 1;
                } else if (strcmp(argv[i], "--inodes") == 0) {
                    showInodes = 1;
                } else if (strcmp(argv[i], "-r") == 0) {
                    sortByName = 1;
                } else if (strcmp(argv[i], "-t") == 0) {
                    sortByTime = 1;
                } else if (strcmp(argv[i], "--dirsfirst") == 0) {
                    showDirsFirst = 1;
                } else if (strcmp(argv[i], "-L") == 0) {
                    if (i + 1 < argc) {
                        maxDepth = atoi(argv[i + 1]);
                        i++;
                    } else {
                        printf("Invalid usage of -L option\n");
                        return 1;
                    }
                } else if (argv[i][0] == '-') {
                    printf("Unknown option: %s\n", argv[i]);
                    return 1;
                } 
            }

            printDirectoriesRec(path, 0);

            return 0;

        };




        