// Author: Pheonix Studios/AkshuDev

#pragma once

#include <stdint.h>
#include <stddef.h>

void decode_x86(uint8_t* data, size_t max_size, size_t* offset, size_t cvaddr, char* out, size_t outsz);
