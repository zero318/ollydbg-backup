#include <stdbool.h>

#define REV L"318"

typedef struct rva_t {
    unsigned int address;
    union {
        int raw_type;
        char type[256];
        char label[256];
    };
    union {
        char name[256];
        char comment[256];
    };
    struct rva_t *next;
} rva_t;

rva_t *backup_load(const char *filename, char *message);
bool backup_save(const char *filename, rva_t *rvas, char *message);
