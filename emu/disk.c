#include "../port.h"

#define DISK_PATH "disk.img"

FILE *disk;
size_t disk_size = 0;

DPM_Disk DPM_Disks[32] = {0};

/**
 * @brief Проверка наличии файла по пути
 *
 * @param Path - Путь, который проверяем
 * @param Name - Имя файла с полным путем для проверки
 *
 * @return int - 1 если путь есть
 */
int fsm_isPathToFile(const char* Path,const char* Name){
	char* zpath = pathinfo(Name, PATHINFO_DIRNAME);					///< Получаем родительскую папку элемента
	char* bpath = pathinfo(Name, PATHINFO_BASENAME);				///< Получаем имя файла (или пустоту если папка)
	bool   isCheck1 = strcmpn(zpath,Path);				            ///< Проверяем совпадение путей
	bool   isCheck2 = strlen(bpath) == 0;				            ///< Проверяем, что путе нет ничего лишнего (будет 0, если просто папка)
	bool   isCheck3 = str_contains(Name, Path);	                    ///< Проверяем наличие, вхождения путя
	size_t c1 = str_cdsp2(Path,'\\');
	size_t c2 = str_cdsp2(Name,'\\');
	size_t c3 = str_cdsp2(Path,'/');
	size_t c4 = str_cdsp2(Name,'/');

	bool   isCheck4 = ((c2 - c1) == 1) && (c4 == c3);
	bool   isCheck5 = ((c4 - c3) == 1) && (c2 == c1);
	bool isPassed = ((isCheck1 && !isCheck2 && isCheck3) || (!isCheck1 && isCheck2 && isCheck3 && (isCheck4 || isCheck5)));
	free(zpath);
	free(bpath);
	return isPassed;
}


void get_disk_size() {
    printf(" |--- [>] Attempt to open to get the file size from %s\n", DISK_PATH);
    FILE *file = fopen(DISK_PATH, "rb");    // Открываем файл в режиме чтения бинарных данных
    if (file == NULL){
            printf(" |     |--- [ERR] An error occurred while retrieving the file size\n");
            printf(" |\n");
            return;
    }
    fseek(file, 0, SEEK_END);               // Устанавливаем позицию указателя файла в конец файла
    disk_size = ftell(file);                // Получаем текущую позицию указателя файла, что является размером файла
    fclose(file);                           // Закрываем файл
    printf(" |     |--- [OK] File size: %d\n", (int) disk_size);
    printf(" |\n");
}


/**
 * @brief [DPM] Считывание данных с диска
 *
 * @param Letter - Буква для считывания
 * @param Offset - Отступ для считывания
 * @param Size - Кол-во байт данных для считывания
 * @param Buffer - Буфер куда будет идти запись
 *
 * @return Кол-во прочитанных байт
 */
size_t dpm_read(char Letter, size_t Offset, size_t Size, void* Buffer){
    /// Взято с https://github.com/pimnik98/SayoriOS/blob/main/kernel/src/drv/disk/dpm.c
    /// Но в примере будет игнорироваться буква диска и переписано для работы с файлом
    int go = fseek(disk, Offset, SEEK_SET);
    if (Offset > disk_size){
        printf("[WARN] It was not possible to move to the %d position, due to: exceeding the limits of the disk volume\n", (unsigned int) Offset);
        return 0;
    } else if (go != 0){
        printf("[WARN] Failed to move to position %d, due to: error moving using fseek\n", (unsigned int) Offset);
        return 0;
    }
    size_t read = fread(Buffer, 1,Size, disk);
    return read;
}


/**
 * @brief [DPM] Запись данных на диск
 *
 * @param Letter - Буква
 * @param size_t Offset - Отступ
 * @param size_t Size - Кол-во байт данных для записи
 * @param Buffer - Буфер откуда будет идти запись
 *
 * @return size_t - Кол-во записанных байт
 */
size_t dpm_write(char Letter, size_t Offset, size_t Size, void* Buffer){
    /// Взято с https://github.com/pimnik98/SayoriOS/blob/main/kernel/src/drv/disk/dpm.c
    /// Но в примере будет игнорироваться буква диска и переписано для работы с файлом
    int go = fseek(disk, Offset, SEEK_SET);
    if (Offset > disk_size){
        printf("[WARN] It was not possible to move to the %d position, due to: exceeding the limits of the disk volume\n", (unsigned int) Offset);
        return 0;
    } else if (go != 0){
        printf("[WARN] Failed to move to position %d, due to: error moving using fseek\n", (unsigned int) Offset);
        return 0;
    }
    size_t write = fwrite(Buffer, 1, Size, disk);
    return write;
}

size_t dpm_get_size(char Letter){
    return disk_size;
}

void* dpm_metadata_read(char Letter){
	int Index = Letter - 65;

	Index = (Index > 32 ? Index - 32 : Index);
	Index = (Index < 0 || Index > 25 ? 0 : Index);

	//if (DPM_Disks[Index].Ready == 0 || DPM_Disks[Index].Status == 0)
		//return 0;

	return DPM_Disks[Index].Reserved;
}

void dpm_metadata_write(char Letter, uint32_t Addr){
	int Index = Letter - 65;

	Index = (Index > 32 ? Index - 32 : Index);
	Index = (Index < 0 || Index > 25 ? 0 : Index);

	DPM_Disks[Index].Reserved = (void*)Addr;
}


void dpm_init(){
    /// Инициализируем типа диск, для этого мы будем использовать файл
    printf("\n[>] Attempt to load disk \"%s\"\n", DISK_PATH);
    disk = fopen(DISK_PATH, "r+b");
    if (disk == NULL) {
        printf (" |--- [ERR] File disk.img no found!\n");
        exit(1);
    }
    get_disk_size();
    printf(" |--- [OK] Loaded disk \"%s\"\n", DISK_PATH);
    printf(" |--- [OK] Size disk %d b. | %d kb. | %d mb.\n", (int) disk_size, (int) disk_size/1024, (int) disk_size/1024/1024);
}
