#include "port.h"

#define FOLDER "/disk/"


void printFiles(){

    printf("\n[>] Getting data from the %s folder \n",FOLDER);
    FSM_DIR* Dir = fs_tempfs_dir('T', FOLDER);
    if (Dir == NULL || Dir->Ready == 0){
        printf(" |--- [ERR] Failed to get folder information\n");
        free(Dir);
        return;
    }
    size_t Sizes = 0;
    for (int i = 0; i < Dir->Count; i++){
        printf("%s\t%s\t\t%s\n",
            " |--- 0000-00-00 00:00",
            (Dir->Files[i].Type == 5?"<DIR> ":"<FILE>"),
            Dir->Files[i].Name
        );
        Sizes += Dir->Files[i].Size;
    }
    printf(" |\n |--- Files: %d | Folders: %d | All: %d\n", Dir->CountFiles, Dir->CountDir, Dir->Count);
    printf(" |--- Folder size: %d mb. | %d kb. | %d b.\n", (Sizes != 0?(Sizes/1024/1024):0), (Sizes != 0?(Sizes/1024):0), Sizes);

    free(Dir);
}

int main()
{
    printf("TempFS - A simple file system with support for reading and writing files.\n");

    printf("TEMPFS_PACKAGE : %d\n", sizeof(TEMPFS_PACKAGE));

    dpm_init();
    int detect = fs_tempfs_detect('T');
    if (detect == 0){
        printf(" |--- [ERR] Could not verify validity on TempFS\n");
        //fs_tempfs_format('T'); ///< Эта строка форматирует диск под разметку TempFS
        return 1;
    }
    printf(" |--- [OK] Successful validation of TempFS\n");



    char* label = malloc(32);
    memset(label, 0 , 32);
    fs_tempfs_label('T',label);

    printf(" |--- Label: %s\n",label);

    printFiles();
/**
    int create_file = fs_tempfs_create('R',"/datafilefs/temp.txt",0);
    printf("create_file: %d\n", create_file);

    int tempwr = fs_tempfs_write('R',"/datafilefs/temp.txt", 0, strlen(TEST_STRING), TEST_STRING);

    printf("tempwr: %d\n", tempwr);
    */
    return 0;
}
