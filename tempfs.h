#pragma once

#include <stdint.h>

#define TEMPFS_CHMOD_READ               0x01  /// Права чтения
#define TEMPFS_CHMOD_WRITE              0x02  /// Права записи
#define TEMPFS_CHMOD_EXEC               0x04  /// Права выполнения
#define TEMPFS_CHMOD_SYS                0x08  /// Права системы


#define TEMPFS_DIR_INFO_ROOT            0x01  /// Родительская папка найдена
#define TEMPFS_DIR_INFO_EXITS           0x02  /// Папка найдена

#define TEMPFS_ENTITY_STATUS_ERROR      0x00  /// Сущность НЕ готова к работе
#define TEMPFS_ENTITY_STATUS_READY      0x01  /// Сущность готова к работе (entity)
#define TEMPFS_ENTITY_STATUS_PKG_READY  0x02  /// Сущность готова к работе (package)

#define TEMPFS_ENTITY_TYPE_UNKNOWN      0x00  /// Неизвестно
#define TEMPFS_ENTITY_TYPE_FILE         0x01  /// Файл
#define TEMPFS_ENTITY_TYPE_FOLDER       0x02  /// Папка


typedef struct {
	uint8_t Status;			///< Статус
    char Name[64];		    ///< Имя файла
    char Path[412];		    ///< Путь файла
    uint32_t Size;			///< Размер файла в байтах
    uint32_t Point;			///< Точка входа в файл
    uint32_t Date;			///< Дата изменения
    char Owner[16];		    ///< Владелец файла
	uint8_t Type;			///< Тип файла
	uint8_t CHMOD;			///< Права доступа
} TEMPFS_ENTITY;  // Получится 512 байт

typedef struct {
	uint16_t Sign1;			///< Сигнатура 1
	uint16_t Sign2;			///< Сигнатура 2
    char Label[32];		    ///< Метка диска
    uint32_t CountFiles;	///< Количество файлов
    uint32_t EndDisk;		///< Точка конца диска
    uint32_t CountBlocks;	///< Количество используемых блоков
	char Rev[16];			///< Не используется
}  TEMPFS_BOOT;

typedef struct {
	uint8_t Status;					///< Статус пакета
	uint16_t Length;				///< Длина пакета
	uint32_t Next;					///< Следующий пакет данных
	char Data[500];					///< Пакет данных
	uint8_t Rev;					///< Зарезервировано, всегда ноль
} TEMPFS_PACKAGE;  // Получится 512 байт

typedef struct {
	uint8_t Status;			    ///< Статус
	TEMPFS_BOOT* Boot;			///< Ссылка на Boot
    uint32_t CountFiles;	    ///< Количество файлов
	TEMPFS_ENTITY* Files;		///< Ссылка на Файловые поинты
    uint32_t BlocksAll;	        ///< Максимальное количество блоков информации
    uint32_t FreeAll;           ///< Свободное количество блоков
    uint32_t EndPoint;          ///< Точка конца диска
}  TEMPFS_Cache;
