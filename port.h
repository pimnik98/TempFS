#pragma once
/// Системные дефайны
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    false = 0,
    true = 1
} bool;

/// Подключаем доп.либы
#include "ext/pathinfo.h"
#include "ext/my_string.h"

/// Дефайны драйвера
/// Эмуляция дискового обращения
#include "emu/disk.h"

/// Подключение типов файлов и папок в SayoriOS
#include "emu/fsm.h"

/// Подключение драйвера файловой системы TempFS
#include "tempfs.h"

#define TEST_STRING "DbG6gWBisxWW5MTUKNu0yB96p94w9o3R0jC950RAfjrp48xBhLwGKgg45128DE2mUsCUX0fX7RBCH41miYY5KPFWt2oH1TFVNZkfxKhA1XYbPq1qyg1Q6zmtZeQVkEjtK4xrhtYmKVcK5TJ1VcXRffZYgUrs1xAMvzTiDrTXVTd6EApcrwzPNvcRCMxTeuwtRcdp4A31gC0VAZ00yHfPjEcVLFDbuny7Pp6EMwek7fXaHYLEGJni6R8euQaoDVKZhpU2o3VKyENJA0JH0rN1Q4dGtZ9KHt0on5Uh5WYgRNHDsvmd1Z8QeyMbAHMYRtBfx83oEkcoCA3Z8yMvRB3ndpo5oPD2qJiRs2D6pnLMTfV54nkWTTTogfguPUcZQ7r5dCrHu9fngz23HGgRZqorWt83uQxc5oJkGxjibiBjDTNKAGGVueG50eFZa0fsUaGEms2TpL6x1Bpxy4gvtd7sZ5RyRmjPnETgzUKnPJXuygp56Ppew0nnpV8FDiDXAPCzhD2ZHFsdHedxgThtZCxkYK6mgEixrwPi66Bkrq2NXzcdE0nv2vRKmQ7WMYEZfom0ZcfvcYKXsUxKmwP02JDq70Lr"
