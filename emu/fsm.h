#pragma once

#include <stdint.h>

typedef struct {
	uint16_t year;	///< Год
	uint8_t month;	///< Месяц
	uint8_t day;	///< День
	uint8_t hour;	///< Час
	uint8_t minute;	///< Минуты
	uint8_t second;	///< Секунды
} __attribute__((packed)) FSM_TIME;

typedef struct {
	int Ready;				///< Существует ли файл?
    char Name[1024];		///< Имя файла
    char Path[1024];		///< Путь файла
    int Mode;				///< Режим файла
    size_t Size;			///< Размер файла в байтах (oсt2bin)
    FSM_TIME LastTime;		///< Время последнего изменения файла
    int Type;				///< Тип элемента
    uint8_t CHMOD;			///< CHMOD файла
} __attribute__((packed)) FSM_FILE;

typedef struct {
	int Ready;				///< Существует ли папка?
	size_t Count;			///< Количество всего
	size_t CountFiles;		///< Количество файлов
	size_t CountDir;		///< Количество папок
	size_t CountOther;		///< Количество неизвестного типа файлов
    FSM_FILE* Files;		///< Файлы и папки
} __attribute__((packed)) FSM_DIR;
