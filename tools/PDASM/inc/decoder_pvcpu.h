// Author: Pheonix Studios/AkshuDev

#pragma once
#include <stddef.h>
#include <stdint.h>

void decode_pvcpu(uint8_t* data, size_t* offset, size_t cvaddr, char* out, size_t outsz);
