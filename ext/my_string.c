#include "../port.h"

uint32_t str_cdsp2(const char a_str[], char del){
    int x = 0;
    for(size_t i = 0, len = strlen(a_str); i < len; i++){
        if (a_str[i] == del) {
            x++;
        }
    }
    return x;
}


/**
 * @brief Сравнение строк
 *
 * @param s1 - Строка 1
 * @param s2 - Строка 2
 *
 * @return Возращает 0 если строки идентичны или разницу между ними
 */
int my_strcmp(const char *s1, const char *s2) {
    while (*s1 && *s1 == *s2) {
        ++s1;
        ++s2;
    }

    return (*s1 - *s2);
}

/**
 * @brief Сравнение строк
 *
 * @param str1 - Строка 1
 * @param str2 - Строка 2
 *
 * @return bool - Возращает true если строки идентичны
 */
bool strcmpn(const char *str1, const char *str2){
    return my_strcmp(str1, str2) == 0;
}
