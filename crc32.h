#pragma once

// Computes the CRC32 of a buffer
uint32_t crc32(void* buf, size_t len, uint32_t partial_crc = 0);