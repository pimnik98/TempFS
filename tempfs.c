#include "port.h"

FSM_DIR* Cache = {0};

TEMPFS_Cache* __TCache__ = {0}; /// Кэш файловой системы
// Часть функционала будет переписана для работы именно с этим кэшем
// Если есть желание поддерживать несколько смонтированных файловых систем
// Тогда рекомендуется,сделать поддержку это внутри вашей ОС
// К примеру, в драйвере разделов, есть специальная позиция
// где можно оставить именно ссылку на этот кэш и после на него ссылаться
// просто в нужных местах замените переменную __TCache__ на необходимую


/**
 * @brief Функция для проверка подписи
 */
int fs_tempfs_func_checkSign(uint16_t sign1, uint16_t sign2){
    return (sign1 + sign2 == 0x975f?1:0);                                   /// Проверяем подпись на основе сумме значений
}

int fs_tempfs_tcache_update(const char Disk){
    tfs_log("[>] TCache update...\n");
    if (__TCache__ == 0){
        __TCache__ = malloc(sizeof(TEMPFS_Cache));
        if (__TCache__ == NULL){
            return 1;
        }
    }
    memset(__TCache__,  0, sizeof(TEMPFS_Cache));
    if (__TCache__->Boot != 0){
        tfs_log(" |--- Free boot (0x%x)...\n", __TCache__->Boot);
        free(__TCache__->Boot);
    }

    if (__TCache__->Files != 0){
        tfs_log(" |--- Free files (0x%x)...\n", __TCache__->Files);
        for (uint32_t i = 0; i < __TCache__->CountFiles; i++) {
            //free(__TCache__->Files[i]);
        }
        free(__TCache__->Files);
    }
    tfs_log(" |--- Malloc Boot...\n");
    __TCache__->Boot = malloc(sizeof(TEMPFS_BOOT));
    if (__TCache__->Boot == NULL){
        return 1;
    }
    tfs_log(" |  |--- Addr Boot: 0x%x\n", __TCache__->Boot);
    tfs_log(" |  |--- Zero Boot...\n");
    memset(__TCache__->Boot, 0, sizeof(TEMPFS_BOOT));
    tfs_log(" |  |--- Read Boot...\n");
    int read = dpm_read(Disk, 0, sizeof(TEMPFS_BOOT), __TCache__->Boot);
    if (read != sizeof(TEMPFS_BOOT)){
        tfs_log(" |       |--- Read: %d\n", read);
        return 2;
    }

    if (fs_tempfs_func_checkSign(__TCache__->Boot->Sign1, __TCache__->Boot->Sign2) != 1){
        tfs_log(" |      |--- Error check signature\n");
        return 3;
    }

    if (__TCache__->Boot->CountFiles > 0){
        __TCache__->Files = malloc(sizeof(TEMPFS_ENTITY) * __TCache__->Boot->CountFiles);
        if (__TCache__->Files == NULL){
            return 1;
        }
        memset(__TCache__->Files, 0, sizeof(TEMPFS_ENTITY) * __TCache__->Boot->CountFiles);
        size_t offset = 512;
        size_t inx = 0;
        tfs_log(" |--- [>] Enter while\n");
        while(1){
            if (inx + 1 > __TCache__->Boot->CountFiles) break;
            tfs_log(" |     |--- [>] %d | %d\n", inx + 1, __TCache__->Boot->CountFiles);
            int eread = dpm_read(Disk, offset, sizeof(TEMPFS_ENTITY), &__TCache__->Files[inx]);
            tfs_log(" |     |     |--- [>] Disk read\n");
            tfs_log(" |     |     |     |--- Offset: %d\n", offset);
            tfs_log(" |     |     |     |--- Size: %d\n", sizeof(TEMPFS_ENTITY));
            offset += sizeof(TEMPFS_ENTITY);                                                    /// Добавляем отступ для следующего поиска
            if (eread != sizeof(TEMPFS_ENTITY)){                                                /// Проверка на кол-во прочитаных байт
                tfs_log(" |      |    |--- Failed to load enough bytes for data.!\n");
                break;                                                                              /// Выходим с цикла, так как не удалось загрузить достаточно байт для данных.
            }
            if (__TCache__->Files[inx].Status != TEMPFS_ENTITY_STATUS_READY){
                tfs_log(" |           |--- No data.!\n");
                continue;                                                                           /// Пропускаем блок информации, тк тут нет информации
            }

            tfs_log(" |     |     |--- [>] File info\n");
            tfs_log(" |     |     |     |--- Name: %s\n", __TCache__->Files[inx].Name);
            tfs_log(" |     |     |     |--- Path: %s\n", __TCache__->Files[inx].Path);
            tfs_log(" |     |     |     |--- Size: %d\n", __TCache__->Files[inx].Size);
            tfs_log(" |     |     |     |--- Type: %s\n", (__TCache__->Files[inx].Type == TEMPFS_ENTITY_TYPE_FOLDER?"Folder":"File"));
            //tfs_log(" |     |     |     |--- Date: %s\n", Files[count].LastTime);
            tfs_log(" |     |     |     |--- CHMOD: 0x%x\n", __TCache__->Files[inx].CHMOD);
            tfs_log(" |     |     |--- Next!\n");
            inx++;
        }
    }

    return 0x7246;
}


void fs_tempfs_func_fixPackage(char* src, int count){
    for (int i = count; i < 9;i++){
		src[i] = 0;
	}
}

TEMPFS_PACKAGE* fs_tempfs_func_readPackage(const char Disk, size_t Address){
    TEMPFS_PACKAGE* pack = malloc(sizeof(TEMPFS_PACKAGE));
    if (pack == NULL){
        return NULL;
    }
    memset(pack, 0, sizeof(TEMPFS_PACKAGE));

    int read = dpm_read(Disk, Address, sizeof(TEMPFS_PACKAGE), pack);
    return pack;
}


int fs_tempfs_func_writePackage(const char Disk, size_t Address, TEMPFS_PACKAGE* pack){
    int write = dpm_write(Disk, Address, sizeof(TEMPFS_PACKAGE), pack);
    return (write == sizeof(TEMPFS_PACKAGE)?1:0);
}

TEMPFS_PACKAGE* fs_tempfs_func_getPackage(const char Disk, size_t Address){
    TEMPFS_PACKAGE* pack = malloc(sizeof(TEMPFS_PACKAGE));
    memset(pack, 0, sizeof(TEMPFS_PACKAGE));
    int read = dpm_read(Disk, Address, sizeof(TEMPFS_PACKAGE), pack);                         /// Загружаем данные пакет с диска
    return pack;
}

int fs_tempfs_func_getCountAllBlocks(size_t all_disk){
    size_t all_free_disk = (all_disk) - (sizeof(TEMPFS_BOOT)) - 1;                      /// Выполняем расчеты всего доступного простраства
    if (all_free_disk <= 0){                                                            /// Проверка на свободное пространство
        return 0;                                                                           /// Возвращаем 0, если нет места для блоков
    }
    size_t all_blocks = (all_free_disk / sizeof(TEMPFS_ENTITY)) - 1;                    /// Получаем количество доступных блоков информации
    return (all_blocks <= 0?0:all_blocks);                                              /// Возвращает кол-во блоков
}

TEMPFS_ENTITY* fs_tempfs_func_readEntity(const char Disk, char* Path){

}

int fs_tempfs_func_writeEntity(const char Disk, int Index, TEMPFS_ENTITY* entity){
    int write = dpm_write(Disk, 512 + (Index*sizeof(TEMPFS_ENTITY)), sizeof(TEMPFS_ENTITY), entity);
    return (write == sizeof(TEMPFS_ENTITY)?1:0);
}


int fs_tempfs_func_updateBoot(const char Disk, TEMPFS_BOOT* boot){
    int write = dpm_write(Disk, 0, sizeof(TEMPFS_BOOT), boot);
    return (write == sizeof(TEMPFS_BOOT)?1:0);
}

TEMPFS_BOOT* fs_tempfs_func_getBootInfo(const char Disk){
    TEMPFS_BOOT* boot = malloc(sizeof(TEMPFS_BOOT));                        /// Создаем переменную с загрузочным разделом
    memset(boot, 0, sizeof(TEMPFS_BOOT));                                   /// Зануляем данные
    int read = dpm_read(Disk, 0, sizeof(TEMPFS_BOOT), boot);                /// Загружаем данные с загрузочного раздела
    return boot;                                                            /// Может вернуться как и NULL, так и пустая структура, так и валидная
}


int fs_tempfs_func_getCountAllFreeEntity(const char Disk){
    TEMPFS_BOOT* boot = fs_tempfs_func_getBootInfo(Disk);                               /// Получаем данные о загрузочном блоке
    if (boot == NULL || fs_tempfs_func_checkSign(boot->Sign1, boot->Sign2) != 1) {      /// Проверка на NULL, а также проверка подписи
        return 0;                                                                         /// Проверка не удалась, возвращаем пустую папку
    }

    size_t all_free_disk = (boot->EndDisk) - (sizeof(TEMPFS_BOOT)) - 1;                 /// Выполняем расчеты всего доступного простраства
    if (all_free_disk <= 0){                                                            /// Проверка на свободное пространство
        return 0;                                                                           /// Возвращаем 0, если нет свободного пространства = нет свободных блоков
    }

    size_t all_blocks = (all_free_disk / sizeof(TEMPFS_ENTITY)) - 1;                    /// Получаем количество доступных блоков информации
    return (all_blocks <= 0?0:(all_blocks - boot->CountBlocks - boot->CountFiles <= 0?0:(all_blocks - boot->CountBlocks - boot->CountFiles)));
}

int fs_tempfs_func_findFreePackage(const char Disk, int Skip){
    printf("[>] Find free package...\n");

    TEMPFS_BOOT* boot = fs_tempfs_func_getBootInfo(Disk);                               /// Получаем данные о загрузочном блоке
    if (boot == NULL || fs_tempfs_func_checkSign(boot->Sign1, boot->Sign2) != 1) {      /// Проверка на NULL, а также проверка подписи
        printf(" |--- [ERR] Signature error\n");
        return -1;                                                                         /// Проверка не удалась, возвращаем -1
    }


    int allcount = fs_tempfs_func_getCountAllBlocks(boot->EndDisk);                     /// Получаем ОБЩИЕ кол-во блоков
    if (allcount <= 0){                                                                 /// Если кол-во блоков меньше= нуля
        printf(" |--- [ERR] No blocks\n");
        return -1;                                                                          /// Возвращаем -1
    }

    int adr = 0;
	for (int i = 0; i < allcount; i++){
		adr = (boot->EndDisk - sizeof(TEMPFS_PACKAGE) - (i * sizeof(TEMPFS_PACKAGE)));
		TEMPFS_PACKAGE* pack = fs_tempfs_func_getPackage(Disk, adr);

		printf("[%d] Test PackAge\n", i);

		printf(" |--- Addr           | %x\n",adr);
		printf(" |--- Data           | %s\n",pack->Data);
		printf(" |--- Status         | %d\n",pack->Status);
		printf(" |--- Length         | %d\n",pack->Length);
		printf(" |--- Next           | %x\n\n",pack->Next);

		if (pack->Status != TEMPFS_ENTITY_STATUS_READY && Skip !=0){
			Skip--;
			continue;
		}

		if (pack->Status != TEMPFS_ENTITY_STATUS_READY){
            free(pack);
            return adr;
		}
		free(pack);
	}

	return -1;
}

int fs_tempfs_func_findFreeInfoBlock(const char Disk){
    if (Cache->Ready == 0){                                                     /// Получаем данные с кэша
        return 0;                                                               /// Кэш не готов к работе, возвращаем 0.
    }
    size_t offset = 512;
    size_t cmx = 0;
    while(1){
        TEMPFS_ENTITY* tmp = malloc(sizeof(TEMPFS_ENTITY));                         /// Создаем временную сущность
        memset(tmp, 0, sizeof(TEMPFS_ENTITY));                                      /// Очищаем эту сущность
        int read = dpm_read(Disk, offset, sizeof(TEMPFS_ENTITY), tmp);              /// Загружаем данные с устройства
        if (read != sizeof(TEMPFS_ENTITY)) {                                        /// Проверяем количество загруженных байт
            free(tmp);
            return -1;                                                                  /// Возвращаем -1, так произошла ошибка.
        }
        if (tmp->Status == 0 || tmp->Type == 0){                                    /// Выполняем проверки
            free(tmp);                                                              /// Освобождение ОЗУ
            return cmx;                                                             /// Возвращаем индекс блока доступного для записи
        }
        cmx++;                                                                      /// Увеличиваем счетчик
        offset += sizeof(TEMPFS_ENTITY);                                            /// Увеличиваем смещение
        free(tmp);                                                                  /// Освобождение ОЗУ
    }
}

int fs_tempfs_func_findFILE(const char Disk, const char* Path){
    if (Cache->Ready == 0 || Cache->Count == 0){                            /// Получаем данные с кэша
        return 0;                                                               /// Данных в кэше нет, возвращаем 0, файл не найден
    }
    char* dir = pathinfo(Path, PATHINFO_DIRNAME);                           /// Создаем данные о родительской папке
    char* basename = pathinfo(Path, PATHINFO_BASENAME);                     /// Создаем данные о базовом названии
    for(size_t cid = 0; cid < Cache->Count; cid++){                         /// Поиск по циклу
        if (Cache->Files[cid].Ready != 1){                                      /// Проверяем сущность на готовность
            continue;                                                               /// Пропускаем, так как сущность не готова
        }
        if (Cache->Files[cid].Type != 0){                                       /// Проверяем сущность на тип файла
            continue;                                                               /// Пропускаем, так как это не файл
        }
        int is_in = strcmp(Cache->Files[cid].Path, dir);                       /// Провяем на совпадении пути
        int is_file = strcmp(Cache->Files[cid].Name, basename);                /// Провяем на совпадении имени
        if (is_in == 0 && is_file == 0){                                       /// Проверка выполненых условий
            free(dir);                                                              /// Освобождение ОЗУ
            free(basename);                                                         /// Освобождение ОЗУ
            return 1;                                                               /// Элемент найден, возвращаем 1
        }
    }
    free(dir);                                                              /// Освобождение ОЗУ
    free(basename);                                                         /// Освобождение ОЗУ
    return 0;                                                               /// Элемент не найден, возвращаем 0
}

int fs_tempfs_func_findDIR(const char Disk, const char* Path){
    if (Cache->Ready == 0 || Cache->Count == 0){                            /// Получаем данные с кэша
        return 0;                                                               /// Данных в кэше нет, возвращаем 0, папка не найдена
    }
    int ret = 0x00;
    char* dir = pathinfo(Path, PATHINFO_DIRNAME);                           /// Создаем данные о родительской папке
    for(size_t cid = 0; cid < Cache->Count; cid++){                         /// Поиск по циклу
        if (Cache->Files[cid].Ready != 1){                                      /// Проверяем сущность на готовность
            continue;                                                               /// Пропускаем, так как сущность не готова
        }
        if (Cache->Files[cid].Type != 5){                                       /// Проверяем сущность на тип папки
            continue;                                                               /// Пропускаем, так как это не папка
        }
        int is_in = strcmp(Cache->Files[cid].Path, dir);                        /// Провяем на наличие родительской папки
        int is_ph = strcmp(Cache->Files[cid].Path, Path);                       /// Провяем на наличие самой папки
        if (is_in == 0){
            ret |= TEMPFS_DIR_INFO_ROOT;                                            /// Ставим флаг, что корневая папка найдена
        }
        if (is_ph == 0){
            ret |= TEMPFS_DIR_INFO_EXITS;                                           /// Ставим флаг, что основная папка найдена
        }
        if ((ret & TEMPFS_DIR_INFO_ROOT) && (ret & TEMPFS_DIR_INFO_EXITS)){     /// Проверка выполненых условий
            free(dir);                                                              /// Освобождение ОЗУ
            return ret;                                                             /// Элемент найден, возвращаем 1
        }
    }
    free(dir);                                                              /// Освобождение ОЗУ
    return ret;                                                                 /// Элемент не найден, возвращаем 0
}

FSM_DIR* fs_tempfs_func_cacheFiles(const char Disk){
    FSM_DIR *Dir = malloc(sizeof(FSM_DIR));                                             /// Создаем данные о папке
    if (Dir == NULL) {                                                                  /// Проверка условий
        return NULL;                                                                        /// Возвращаем NULL, так как память выделить не удалось
    }
    TEMPFS_BOOT* boot = fs_tempfs_func_getBootInfo(Disk);                               /// Получаем данные о загрузочном блоке
    if (boot == NULL || fs_tempfs_func_checkSign(boot->Sign1, boot->Sign2) != 1) {      /// Проверка на NULL, а также проверка подписи
        return Dir;                                                                         /// Проверка не удалась, возвращаем пустую папку
    }
    FSM_FILE *Files = malloc(sizeof(FSM_FILE) * boot->CountFiles);                      /// Создаем данные о файлах (размер блока файла * кол-во сущностей)
    if (Files == NULL) {                                                                /// Проверяем удалось ли создать сущность
        return Dir;                                                                         /// Проверка не удалась, возвращаем пустую папку
    }
    size_t offset = 512;                                                                /// Указываем отступ, с которого начинаем поиск
    size_t count = 0;                                                                   /// Текущие количество найденых сущностей
    size_t CF = 0, CD = 0;                                                              /// Текущие количество найденых (CF-файлов|CD-папок)
    printf(" |--- [>] Enter while\n");
    while(1){                                                                           /// Входим в цикл для поиска сущностей
        if (count + 1 > boot->CountFiles){                                                  /// Проверяем на кол-во проверенных сущностей
            break;                                                                              /// Выходим с цикла, так как все данные уже найдены.
        }
        printf(" |     |--- [>] %d > %d\n", count + 1, boot->CountFiles);
        TEMPFS_ENTITY* entity = malloc(sizeof(TEMPFS_ENTITY));                              /// Создаем сущность
        if (entity == NULL) {                                                               /// Проверяем удалось ли выделить память, для данной сущности
            break;                                                                              /// Выходим с цикла, так как нет ОЗУ
        }
        int eread = dpm_read(Disk, offset, sizeof(TEMPFS_ENTITY), entity);                  /// Загружаем данные о сущности с диска
        printf(" |     |     |--- [>] Disk read\n");
        printf(" |     |     |     |--- Offset: %d\n", offset);
        printf(" |     |     |     |--- Size: %d\n", sizeof(TEMPFS_ENTITY));
        offset += sizeof(TEMPFS_ENTITY);                                                    /// Добавляем отступ для следующего поиска
        if (eread != sizeof(TEMPFS_ENTITY)){                                                /// Проверка на кол-во прочитаных байт
            printf(" |      |    |--- Failed to load enough bytes for data.!\n");
            free(entity);                                                                       /// Освобождаем ОЗУ от сущности
            break;                                                                              /// Выходим с цикла, так как не удалось загрузить достаточно байт для данных.
        }
        if (entity->Status == TEMPFS_ENTITY_STATUS_ERROR ||                                 /// Проверка на статус и тип сущности
            entity->Type   == TEMPFS_ENTITY_TYPE_UNKNOWN){
            printf(" |           |--- No data.!\n");
            free(entity);                                                                       /// Освобождаем ОЗУ от сущности
            continue;                                                                           /// Пропускаем блок информации, тк тут нет информации
        }
        Files[count].Ready = 1;                                                             /// Указываем что сущность есть
        Files[count].CHMOD = entity->CHMOD;                                                 /// Копируем настройки CHMOD (RWES)
        Files[count].Size = entity->Size;                                                   /// Копируем размер файла
        memcpy(Files[count].Name, entity->Name, strlen(entity->Name));                      /// Копируем имя файла
        memcpy(Files[count].Path, entity->Path, strlen(entity->Path));                      /// Копируем имя пути

        if (entity->Type == TEMPFS_ENTITY_TYPE_FOLDER){                                     /// Проверяем является ли сущность папкой?
            CD++;                                                                               /// Увеличиваем счетчик папок
            Files[count].Type = 5;                                                              /// Указываем что сущность папка
        } else {                                                                            /// Если это не папка, значит файл
            CF++;                                                                               /// Увеличиваем счетчик файлов
            Files[count].Type = 0;                                                              /// Указываем что это файл
        }
        printf(" |     |     |--- [>] File info\n");
        printf(" |     |     |     |--- Name: %s\n", Files[count].Name);
        printf(" |     |     |     |--- Path: %s\n", Files[count].Path);
        printf(" |     |     |     |--- Size: %d\n", Files[count].Size);
        printf(" |     |     |     |--- Type: %s\n", (Files[count].Type == 5?"Folder":"File"));
        //printf(" |     |     |     |--- Date: %s\n", Files[count].LastTime);
        printf(" |     |     |     |--- CHMOD: 0x%x\n", Files[count].CHMOD);

        count++;                                                                            /// Увеличеваем счетчик найденных сущностей
        free(entity);                                                                       /// Освобождаем ОЗУ от сущности
        printf(" |     |     |--- Next!\n");
    }

    Dir->Ready = 1;                                                                     /// Сообщаем что данные загружены
	Dir->Count = count;                                                                 /// Записываем кол-во найденых сущностей
	Dir->CountFiles = CF;                                                               /// Записываем кол-во найденых файлов
	Dir->CountDir = CD;                                                                 /// Записываем кол-во найденых папок
	Dir->CountOther = 0;                                                                /// Записываем кол-во найденых прочих элементов
	Dir->Files = Files;                                                                 /// Записываем ссылку на список с файлами
    free(boot);                                                                         /// Освобождаем ОЗУ
    return Dir;                                                                         /// Возвращаем папку
}

void fs_tempfs_func_cacheUpdate(const char Disk){
    if (Cache->Ready != 0){                                                   /// Проверка наличия данных в кэше
        //free(Cache->Files);                                                       /// Освобождение ОЗУ от файлов
        //free(Cache);                                                              /// Освобождение ОЗУ от кэша
    }
    Cache = fs_tempfs_func_cacheFiles(Disk);                                  /// Обновляем кэш устройства
}

size_t fs_tempfs_read(const char Disk, const char* Path, size_t Offset, size_t Size, void* Buffer){

	return 0;
}

size_t fs_tempfs_write(const char Disk, const char* Path, size_t Offset, size_t Size, void* Buffer){
    printf("File write...\n");
    int found = fs_tempfs_func_findFILE(Disk, Path);
	if (found == 0){
        printf("File no found.\n");
		return 0;
	}

    printf("File nrwxt...\n");
	size_t src_size = Size;
	size_t src_seek = 0;
	size_t currentAddr = 0;
	size_t read = 0;
	char src_buf[512] = {0};
    printf("(%d == 0?1:(%d/500)+1)\n", src_size, src_size);
	size_t countPack = (src_size == 0?1:(src_size/500)+1);

    printf("File countPack: %d...\n", countPack);
	printf("[WriteFile]\n * src_size: %d\n * countPack : %d\n * Buffer: %s\n", src_size, countPack, Buffer);

	TEMPFS_PACKAGE* pkg_free = malloc(sizeof(TEMPFS_PACKAGE));
	uint32_t *pkg_addr = malloc(sizeof(uint32_t)*(countPack+1));
	// Выделяем память для структур и записи
	if (pkg_free == NULL || pkg_addr == NULL){
		printf("KMALLOC ERROR\n");
		return 0;
	}

	for (int i = 0; i < countPack; i++) {

		pkg_addr[i] = fs_tempfs_func_findFreePackage(Disk,i);

		if (pkg_addr[i] == -1) {
			printf("NO FREE PACKAGE!!!\n");
			return 0;
		}

		printf("Found %x FREE PACKAGE!\n", pkg_addr[i]);
	}

	for (int a = 0; a <= src_size; a++){
		if (src_seek == 500){
			// Buffer
			fs_tempfs_func_fixPackage(src_buf,src_seek);

			printf("Buffer: %s\n",src_buf);

			memcpy((void*) pkg_free->Data, src_buf, 500);

			pkg_free->Length = 500;
			pkg_free->Next = pkg_addr[currentAddr+1];
			pkg_free->Status = TEMPFS_ENTITY_STATUS_PKG_READY;

            fs_tempfs_func_writePackage(Disk, pkg_addr[currentAddr], pkg_free);
            memset(src_buf, 0, 512);
			currentAddr++;
			src_seek = 0;
			a--;
			//src_buf = {0};
		} else if (a == (src_size)){
			fs_tempfs_func_fixPackage(src_buf, src_seek);

			printf("EOL | Buffer: %s\n",src_buf);
			memcpy((void*) pkg_free->Data, src_buf, 500);

			pkg_free->Length = src_seek;
			pkg_free->Next = -1;
			pkg_free->Status = TEMPFS_ENTITY_STATUS_PKG_READY;

            fs_tempfs_func_writePackage(Disk, pkg_addr[currentAddr], pkg_free);

			break;
		} else {
			src_buf[src_seek] = *((char *)Buffer + a);
			src_seek++;
		}
	}

	printf("Write complete!\n");
	printf("[+] Package to write:\n");

	for (int i = 0; i < countPack; i++){
		printf(" |--- Addr           | %x\n",pkg_addr[i]);
	}
	return src_size;
}

FSM_FILE fs_tempfs_info(const char Disk, const char* Path){

}

FSM_DIR* fs_tempfs_dir(const char Disk, const char* Path){
    if (Cache->Ready != 0){                                                   /// Проверка наличия данных в кэше
        return Cache;
    } else {
        return Cache;
    }
}

int fs_tempfs_create(const char Disk,const char* Path,int Mode){
    printf("[>] Creating a new entity\n");
    TEMPFS_ENTITY* entity = malloc(sizeof(TEMPFS_ENTITY));                  /// Создаем сущность
    memset(entity, 0, sizeof(TEMPFS_ENTITY));
    int find_dir = fs_tempfs_func_findDIR(Disk, Path);                      /// Ищем родительскую папку
    printf(" |--- Folder search result: 0x%x\n", find_dir);
    if (Mode == 0)  {                                                       /// Создаем файл
        printf(" |--- Creating a file\n");
        if ((find_dir & TEMPFS_DIR_INFO_EXITS) || !(find_dir & TEMPFS_DIR_INFO_ROOT)){
            free(entity);                                                           /// Освобождение ОЗУ
            return 0;                                                               /// Проверяется наличие родительской и основной папки если нет, то возвращаем 0
        }
        int find_file = fs_tempfs_func_findFILE(Disk, Path);                    /// Ищем такой файл в папке
        printf(" |--- File search result: %d\n", find_file);
        if (find_file == 1){                                                    /// Проверка условий
            free(entity);                                                           /// Освобождение ОЗУ
            return 0;                                                               /// Возвращаем 0, так как такой файл уже существует
        }
        printf(" |--- Filling in metadata\n");
        char* dir = pathinfo(Path, PATHINFO_DIRNAME);                           /// Создаем данные о родительской папке
        char* basename = pathinfo(Path, PATHINFO_BASENAME);                     /// Создаем данные о базовом названии
        memcpy(entity->Name, basename, strlen(basename));                       /// Копируем данные об имени файла
        memcpy(entity->Path, dir, strlen(dir));                                 /// Копируем данные об пути файла
        entity->CHMOD |= TEMPFS_CHMOD_READ | TEMPFS_CHMOD_WRITE;                /// Ставим биты, чтения и записи
        // entity->Date = 0;
        entity->Size = 0;                                                       /// Указываем размер файла
        entity->Status = TEMPFS_ENTITY_STATUS_READY;                            /// Указываем что файл готов к работе
        entity->Type = TEMPFS_ENTITY_TYPE_FILE;                                 /// Указываем что это файл
        printf(" |--- Next step\n");
    } else {                                                                /// Создаем папку
        printf(" |--- Creating a folder\n");
        if ((find_dir & TEMPFS_DIR_INFO_EXITS) || !(find_dir & TEMPFS_DIR_INFO_ROOT)){
            free(entity);                                                           /// Освобождение ОЗУ
            return 0;                                                               /// Если родительская папка не существует, или существует основная папка, возвращаеим 0
        }
        char* basename = pathinfo(Path, PATHINFO_BASENAME);                     /// Создаем данные о базовом названии
        memcpy(entity->Name, basename, strlen(basename));                       /// Копируем данные об имени файла
        memcpy(entity->Path, Path, strlen(Path));                               /// Копируем данные об пути файла
        entity->CHMOD |= TEMPFS_CHMOD_READ | TEMPFS_CHMOD_WRITE;                /// Ставим биты, чтения и записи
        // entity->Date = 0;
        entity->Size = 0;                                                       /// Указываем размер файла
        entity->Status = TEMPFS_ENTITY_STATUS_READY;                            /// Указываем что файл готов к работе
        entity->Type = TEMPFS_ENTITY_TYPE_FOLDER;                               /// Указываем что это папка
        printf(" |--- Next step\n");
    }
    printf(" |--- Searching for a free block to record an entity\n");
    int inx = fs_tempfs_func_findFreeInfoBlock(Disk);                       /// Поиск свободного блока
    if (inx == -1){                                                         /// Проверка условий
        printf(" |--- Couldn't find a free entity\n");
        free(entity);                                                           /// Освобождение ОЗУ
        return 0;                                                               /// Возвращаем 0, так как нет свободного блока
    }


    printf(" |--- Writing data to the device\n");
    int wr_entity = fs_tempfs_func_writeEntity(Disk, inx, entity);
    free(entity);
    if (wr_entity != 1){
        printf(" |-- [WARN] There was a problem when writed data on the 0x%x section\n", 512 + (inx * sizeof(TEMPFS_ENTITY)));
        return 0;
    }
    printf(" |--- Updating boot sector\n");
    TEMPFS_BOOT* boot = fs_tempfs_func_getBootInfo(Disk);
    if (boot == NULL || fs_tempfs_func_checkSign(boot->Sign1, boot->Sign2) != 1) {
        printf(" |--- [ERR] TempFS signature did not match OR error reading TempFS boot sector\n");
        return 0;
    }
    boot->CountFiles++;
    /// Пишем загрузочную
    int boot_write = fs_tempfs_func_updateBoot(Disk, boot);
    if (boot_write != 1){
        printf(" |-- [ERR] An error occurred while writing the TempFS boot partition\n");
        return 0;
    }
    printf(" |--- Updating the cache\n");
    fs_tempfs_func_cacheUpdate(Disk);
	return 1;
}

int fs_tempfs_delete(const char Disk,const char* Path,int Mode){
	return 0;
}

void fs_tempfs_label(const char Disk, char* Label){
    TEMPFS_BOOT* boot = fs_tempfs_func_getBootInfo(Disk);
    if (boot == NULL || fs_tempfs_func_checkSign(boot->Sign1, boot->Sign2) != 1) {
        printf(" |--- [ERR] TempFS signature did not match OR error reading TempFS boot sector\n");
        return;
    }
	memcpy(Label,boot->Label,strlen(boot->Label));
	free(boot);
}

int fs_tempfs_detect(const char Disk){
    printf("\n[>] Attempt to check the boot sector\n");
    int ret = fs_tempfs_tcache_update(Disk);
    printf(" |--- [>] ret: 0x%x\n", ret);

	return 0;
}

void fs_tempfs_format(const char Disk){
    printf("\n[>] Formatting for TempFS has started...\n");
    /// Создаем данные для записи
    TEMPFS_BOOT* boot = malloc(sizeof(TEMPFS_BOOT));
    TEMPFS_ENTITY* tmp = malloc(sizeof(TEMPFS_ENTITY));
    /// Заполняем данные нулями
    memset(boot, 0, sizeof(TEMPFS_BOOT));
    memset(tmp, 0, sizeof(TEMPFS_ENTITY));

    /// Заполняем базовую информацию
    boot->Sign1 = 0x7246;
    boot->Sign2 = 0x2519;
    memcpy(boot->Label,"New disk",strlen("New disk"));
    boot->EndDisk = dpm_get_size(Disk);
    boot->CountBlocks = 0;
    boot->CountFiles = 1;

    /// Пишем загрузочную
    int write = fs_tempfs_func_updateBoot(Disk, boot);
    if (write != 1){
        printf(" |-- [ERR] An error occurred while writing the TempFS boot partition\n");
        return;
    }

    /// Выполняем расчеты о свободном пространстве диске
    size_t all_free_disk = (boot->EndDisk) - (sizeof(TEMPFS_BOOT)) - 1;
    if (all_free_disk <= 0){
        printf(" |-- [ERR] The file system requires a minimum of 1024 bytes of memory\n");
        printf(" |-- %d = (%d - %d - 1)\n", all_free_disk, boot->EndDisk, (sizeof(TEMPFS_BOOT)));
        printf(" |-- INTERRUPTED!!");
        return;
    }
    /// Получаем количество доступных блоков информации
    size_t all_blocks = (all_free_disk / sizeof(TEMPFS_ENTITY)) - 1;
    if (all_blocks <= 0){
        printf(" |-- [WARN] There are no free blocks left for file system elements!\n");
        all_blocks = 0;
    }

    /// Затираем все данные с диска
    for (size_t abx = 0; abx < all_blocks; abx++){
        printf(" |-- [>] [%d | %d] Clearing the hard drive of old data\n",abx + 1,all_blocks);
        int wr_entity = fs_tempfs_func_writeEntity(Disk, abx, tmp);
        if (wr_entity != 1){
            printf(" |-- [WARN] There was a problem when erasing data on the 0x%x section\n", 512 + (abx * sizeof(TEMPFS_ENTITY)));
        }
    }

    /// Создаем корневую папку
    tmp->CHMOD |= TEMPFS_CHMOD_READ | TEMPFS_CHMOD_WRITE | TEMPFS_CHMOD_SYS; /// Ставим биты, чтения, записи и системы
    tmp->Status = TEMPFS_ENTITY_STATUS_READY;
    tmp->Type   = TEMPFS_ENTITY_TYPE_FOLDER;
    memcpy(tmp->Name, "/", strlen("/"));
    memcpy(tmp->Path, "/", strlen("/"));
    memcpy(tmp->Owner, "root", strlen("root"));

    int root_write = fs_tempfs_func_writeEntity(Disk, 0, tmp);
    if (root_write != 1){
        printf(" |-- [WARN] Failed to write the root directory\n");
    }
    printf(" |-- Disk formatting is complete!\n");
    printf(" |-- Label: %s\n", boot->Label);
    printf(" |-- Free space: %d\n", all_free_disk);
    printf(" |-- Free blocks: %d\n", all_blocks);

    /// Освобождаем оперативную память
    free(tmp);
    free(boot);
}
