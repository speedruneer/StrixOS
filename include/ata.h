#pragma once
#include <stdint.h>
#include <stddef.h>
int ATA_WRITE_28(const void *source, uint32_t lba, uint8_t count);
int ATA_READ_28(void *buffer, uint32_t lba, uint8_t count);