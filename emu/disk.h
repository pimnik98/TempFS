#pragma once



typedef struct {
	bool Ready;				///< Устройство подключено? (1 - да | 0 - нет)
	void* Reserved;			///< Можно в ОЗУ дописать доп.данные если требуется.
} __attribute__((packed)) DPM_Disk;

void dpm_init();
size_t dpm_write(char Letter, size_t Offset, size_t Size, void *Buffer);
size_t dpm_read(char Letter, size_t Offset, size_t Size, void *Buffer);
size_t dpm_get_size(char Letter);
int fsm_isPathToFile(const char* Path,const char* Name);
