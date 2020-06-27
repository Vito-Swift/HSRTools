/**
 * @author: vitowu
 * @email: vitow@nvidia.com
 * @date: 6/17/20
 */

#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/time.h>
#include <cstdlib>
#include <random>
#include <stdint.h>
#include <chrono>
#include <cstring>

#include <immintrin.h>
#include <emmintrin.h>

#include "utils.h"

#define FILE_SIZE (128UL*1024UL*1024UL)
#define CHUNK_SIZE (FILE_SIZE / 8)
#define SLICE 32
#define TOTAL_SLICE_NUM (CHUNK_SIZE / SLICE)
#define PARITY_PADDING (SLICE * 20)
#define PARITY_SIZE (CHUNK_SIZE + PARITY_PADDING)

#define SAMPLE_NUM 0x01UL
#define ENCODE_FILE "dummy.txt"
//#define STORE_FILE(CHUNK_ID, TYPE)                                                                  \
//    do {                                                                                            \
//        std::ofstream FileOut;                                                                      \
//        if (TYPE == 0)                                                                              \
//            FileOut.open(std::string(ENCODE_FILE)+std::string(".chunk")+std::to_string(CHUNK_ID));  \
//        else                                                                                        \
//            FileOut.open(std::string(ENCODE_FILE)+std::string(".chunk")+std::to_string(CHUNK_ID+8));\
//        if (!FileOut)                                                                               \
//        {                                                                                           \
//            PRINTF_ERR_STAMP("cannot open file to write chunk on! file saving is not success.\n");  \
//            return ;                                                                                \
//        }                                                                                           \
//                                                                                                    \
//    #if (TYPE==0)                                                                           \
//            FileOut.write((char*) (input_chunk_##CHUNK_ID), sizeof(input_chunk_##CHUNK_ID));        \
//        else                                                                                        \
//        {                                                                                           \
//            FileOut.write((char*) (parity_packet_##CHUNK_ID), sizeof(parity_packet_##CHUNK_ID));    \
//#endif                                                                                     \
//        FileOut.close();                                                                            \
//    } while (0);

uint8_t input_chunk_1[CHUNK_SIZE] = {0};
uint8_t input_chunk_2[CHUNK_SIZE] = {0};
uint8_t input_chunk_3[CHUNK_SIZE] = {0};
uint8_t input_chunk_4[CHUNK_SIZE] = {0};
uint8_t input_chunk_5[CHUNK_SIZE] = {0};
uint8_t input_chunk_6[CHUNK_SIZE] = {0};
uint8_t input_chunk_7[CHUNK_SIZE] = {0};
uint8_t input_chunk_8[CHUNK_SIZE] = {0};
uint8_t output_chunk_6[CHUNK_SIZE] = {0};
uint8_t output_chunk_7[CHUNK_SIZE] = {0};
uint8_t output_chunk_8[CHUNK_SIZE] = {0};
uint8_t parity_packet_1[PARITY_SIZE] = {0};
uint8_t parity_packet_2[PARITY_SIZE] = {0};
uint8_t parity_packet_3[PARITY_SIZE] = {0};

void encode() {

    // HID matrix:
    // |  1    &  z^1  &  z^2  &  z^3  &  z^4  &  z^5  &  z^6  &  z^7  |
    // |  1    &  1    &  1    &  1    &  1    &  1    &  1    &  1    |
    // |  z^7  &  z^6  &  z^5  &  z^4  &  z^3  &  z^2  &  z^1  &  1    |

    __m256i mxx1, mxx2;
    const uint64_t chunk_size = 512;
    const uint64_t chunk_num = TOTAL_SLICE_NUM / chunk_size;
    uint64_t chunk_slice_id;
    uint64_t slice_id;
    uint64_t slice_offset_1;
    uint64_t slice_offset_2;
    uint64_t slice_offset_3;

    for (chunk_slice_id = 0; chunk_slice_id < chunk_num; chunk_slice_id++)
    {
        for (slice_id = chunk_slice_id * chunk_size; slice_id < chunk_slice_id * chunk_size + chunk_size; slice_id++)
        {
            slice_offset_2 = slice_id * SLICE; // point to the start point of memory block (packet $slice_id)
            mxx1 = _mm256_loadu_si256((__m256i *) &input_chunk_1[slice_offset_2]);
            _mm256_storeu_si256((__m256i *) &parity_packet_1[slice_offset_2], mxx1);
            _mm256_storeu_si256((__m256i *) &parity_packet_2[slice_offset_2], mxx1);
            _mm256_storeu_si256((__m256i *) &parity_packet_3[slice_offset_2 + 7 * SLICE], mxx1);
        }

        for (slice_id = chunk_slice_id * chunk_size; slice_id < chunk_slice_id * chunk_size + chunk_size; slice_id++)
        {
            slice_offset_2 = slice_id * SLICE; // point to the start point of memory block (packet $slice_id)
            slice_offset_1 = ((slice_id + 1) * SLICE); // point to the start point of memory block in packet 1
            slice_offset_3 = ((slice_id + 6) * SLICE); // point to the start point of memory block in packet 2

            // load memory block of file into register
            mxx1 = _mm256_loadu_si256((__m256i *) &input_chunk_2[slice_offset_2]);

            // xor and store in packet 1
            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_1[slice_offset_1]));
            _mm256_storeu_si256((__m256i *) &parity_packet_1[slice_offset_1], mxx2);

            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_2[slice_offset_2]));
            _mm256_storeu_si256((__m256i *) &parity_packet_2[slice_offset_2], mxx2);

            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_3[slice_offset_3]));
            _mm256_storeu_si256((__m256i *) &parity_packet_3[slice_offset_3], mxx2);
        }

        for (slice_id = chunk_slice_id * chunk_size; slice_id < chunk_slice_id * chunk_size + chunk_size; slice_id++)
        {
            slice_offset_2 = slice_id * SLICE; // point to the start point of memory block (packet $slice_id)
            slice_offset_1 = ((slice_id + 2) * SLICE); // point to the start point of memory block in packet 1
            slice_offset_3 = ((slice_id + 5) * SLICE); // point to the start point of memory block in packet 2

            // load memory block of file into register
            mxx1 = _mm256_loadu_si256((__m256i *) &input_chunk_3[slice_offset_2]);

            // xor and store in packet 1
            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_1[slice_offset_1]));
            _mm256_storeu_si256((__m256i *) &parity_packet_1[slice_offset_1], mxx2);

            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_2[slice_offset_2]));
            _mm256_storeu_si256((__m256i *) &parity_packet_2[slice_offset_2], mxx2);

            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_3[slice_offset_3]));
            _mm256_storeu_si256((__m256i *) &parity_packet_3[slice_offset_3], mxx2);
        }

        for (slice_id = chunk_slice_id * chunk_size; slice_id < chunk_slice_id * chunk_size + chunk_size; slice_id++)
        {
            slice_offset_2 = slice_id * SLICE; // point to the start point of memory block (packet $slice_id)
            slice_offset_1 = ((slice_id + 3) * SLICE); // point to the start point of memory block in packet 1
            slice_offset_3 = ((slice_id + 4) * SLICE); // point to the start point of memory block in packet 2

            // load memory block of file into register
            mxx1 = _mm256_loadu_si256((__m256i *) &input_chunk_4[slice_offset_2]);

            // xor and store in packet 1
            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_1[slice_offset_1]));
            _mm256_storeu_si256((__m256i *) &parity_packet_1[slice_offset_1], mxx2);

            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_2[slice_offset_2]));
            _mm256_storeu_si256((__m256i *) &parity_packet_2[slice_offset_2], mxx2);

            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_3[slice_offset_3]));
            _mm256_storeu_si256((__m256i *) &parity_packet_3[slice_offset_3], mxx2);
        }

        for (slice_id = chunk_slice_id * chunk_size; slice_id < chunk_slice_id * chunk_size + chunk_size; slice_id++)
        {
            slice_offset_2 = slice_id * SLICE; // point to the start point of memory block (packet $slice_id)
            slice_offset_1 = ((slice_id + 4) * SLICE); // point to the start point of memory block in packet 1
            slice_offset_3 = ((slice_id + 3) * SLICE); // point to the start point of memory block in packet 2

            // load memory block of file into register
            mxx1 = _mm256_loadu_si256((__m256i *) &input_chunk_5[slice_offset_2]);

            // xor and store in packet 1
            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_1[slice_offset_1]));
            _mm256_storeu_si256((__m256i *) &parity_packet_1[slice_offset_1], mxx2);

            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_2[slice_offset_2]));
            _mm256_storeu_si256((__m256i *) &parity_packet_2[slice_offset_2], mxx2);

            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_3[slice_offset_3]));
            _mm256_storeu_si256((__m256i *) &parity_packet_3[slice_offset_3], mxx2);
        }

        for (slice_id = chunk_slice_id * chunk_size; slice_id < chunk_slice_id * chunk_size + chunk_size; slice_id++)
        {
            slice_offset_2 = slice_id * SLICE; // point to the start point of memory block (packet $slice_id)
            slice_offset_1 = ((slice_id + 5) * SLICE); // point to the start point of memory block in packet 1
            slice_offset_3 = ((slice_id + 2) * SLICE); // point to the start point of memory block in packet 2

            // load memory block of file into register
            mxx1 = _mm256_loadu_si256((__m256i *) &input_chunk_6[slice_offset_2]);

            // xor and store in packet 1
            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_1[slice_offset_1]));
            _mm256_storeu_si256((__m256i *) &parity_packet_1[slice_offset_1], mxx2);

            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_2[slice_offset_2]));
            _mm256_storeu_si256((__m256i *) &parity_packet_2[slice_offset_2], mxx2);

            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_3[slice_offset_3]));
            _mm256_storeu_si256((__m256i *) &parity_packet_3[slice_offset_3], mxx2);
        }

        for (slice_id = chunk_slice_id * chunk_size; slice_id < chunk_slice_id * chunk_size + chunk_size; slice_id++)
        {
            slice_offset_2 = slice_id * SLICE; // point to the start point of memory block (packet $slice_id)
            slice_offset_1 = ((slice_id + 6) * SLICE); // point to the start point of memory block in packet 1
            slice_offset_3 = ((slice_id + 1) * SLICE); // point to the start point of memory block in packet 2

            // load memory block of file into register
            mxx1 = _mm256_loadu_si256((__m256i *) &input_chunk_7[slice_offset_2]);

            // xor and store in packet 1
            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_1[slice_offset_1]));
            _mm256_storeu_si256((__m256i *) &parity_packet_1[slice_offset_1], mxx2);

            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_2[slice_offset_2]));
            _mm256_storeu_si256((__m256i *) &parity_packet_2[slice_offset_2], mxx2);

            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_3[slice_offset_3]));
            _mm256_storeu_si256((__m256i *) &parity_packet_3[slice_offset_3], mxx2);
        }

        for (slice_id = chunk_slice_id * chunk_size; slice_id < chunk_slice_id * chunk_size + chunk_size; slice_id++)
        {
            slice_offset_2 = slice_id * SLICE; // point to the start point of memory block (packet $slice_id)
            slice_offset_1 = ((slice_id + 7) * SLICE); // point to the start point of memory block in packet 1
            slice_offset_3 = slice_id * SLICE; // point to the start point of memory block in packet 2

            // load memory block of file into register
            mxx1 = _mm256_loadu_si256((__m256i *) &input_chunk_8[slice_offset_2]);

            // xor and store in packet 1
            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_1[slice_offset_1]));
            _mm256_storeu_si256((__m256i *) &parity_packet_1[slice_offset_1], mxx2);

            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_2[slice_offset_2]));
            _mm256_storeu_si256((__m256i *) &parity_packet_2[slice_offset_2], mxx2);

            mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_3[slice_offset_3]));
            _mm256_storeu_si256((__m256i *) &parity_packet_3[slice_offset_3], mxx2);
        }
    }
}

void decode() {
    // assume that only input file 1-5 and parity file 1-3 are available

    __m256i mxx1, mxx2;
    uint64_t slice_id;
    uint64_t slice_offset_1;
    uint64_t slice_offset_2;
    uint64_t slice_offset_3;

    // back substitution for packet 1-5
    // IDENTICAL with encoding packet 1 to 5
    for (slice_id = 0; slice_id < TOTAL_SLICE_NUM; slice_id++)
    {
        slice_offset_2 = slice_id * SLICE; // point to the start point of memory block (packet $slice_id)
        slice_offset_1 = slice_id * SLICE; // point to the start point of memory block in packet 1
        slice_offset_3 = ((slice_id + 7) * SLICE); // point to the start point of memory block in packet 2

        // load memory block of file into register
        mxx1 = _mm256_loadu_si256((__m256i *) &input_chunk_1[slice_offset_2]);

        // xor and store in packet 1
        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_1[slice_offset_1]));
        _mm256_storeu_si256((__m256i *) &parity_packet_1[slice_offset_1], mxx2);

        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_2[slice_offset_2]));
        _mm256_storeu_si256((__m256i *) &parity_packet_2[slice_offset_2], mxx2);

        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_3[slice_offset_3]));
        _mm256_storeu_si256((__m256i *) &parity_packet_3[slice_offset_3], mxx2);
    }

    for (slice_id = 0; slice_id < TOTAL_SLICE_NUM; slice_id++)
    {
        slice_offset_2 = slice_id * SLICE; // point to the start point of memory block (packet $slice_id)
        slice_offset_1 = ((slice_id + 1) * SLICE); // point to the start point of memory block in packet 1
        slice_offset_3 = ((slice_id + 6) * SLICE); // point to the start point of memory block in packet 2

        // load memory block of file into register
        mxx1 = _mm256_loadu_si256((__m256i *) &input_chunk_2[slice_offset_2]);

        // xor and store in packet 1
        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_1[slice_offset_1]));
        _mm256_storeu_si256((__m256i *) &parity_packet_1[slice_offset_1], mxx2);

        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_2[slice_offset_2]));
        _mm256_storeu_si256((__m256i *) &parity_packet_2[slice_offset_2], mxx2);

        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_3[slice_offset_3]));
        _mm256_storeu_si256((__m256i *) &parity_packet_3[slice_offset_3], mxx2);
    }

    for (slice_id = 0; slice_id < TOTAL_SLICE_NUM; slice_id++)
    {
        slice_offset_2 = slice_id * SLICE; // point to the start point of memory block (packet $slice_id)
        slice_offset_1 = ((slice_id + 2) * SLICE); // point to the start point of memory block in packet 1
        slice_offset_3 = ((slice_id + 5) * SLICE); // point to the start point of memory block in packet 2

        // load memory block of file into register
        mxx1 = _mm256_loadu_si256((__m256i *) &input_chunk_3[slice_offset_2]);

        // xor and store in packet 1
        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_1[slice_offset_1]));
        _mm256_storeu_si256((__m256i *) &parity_packet_1[slice_offset_1], mxx2);

        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_2[slice_offset_2]));
        _mm256_storeu_si256((__m256i *) &parity_packet_2[slice_offset_2], mxx2);

        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_3[slice_offset_3]));
        _mm256_storeu_si256((__m256i *) &parity_packet_3[slice_offset_3], mxx2);
    }

    for (slice_id = 0; slice_id < TOTAL_SLICE_NUM; slice_id++)
    {
        slice_offset_2 = slice_id * SLICE; // point to the start point of memory block (packet $slice_id)
        slice_offset_1 = ((slice_id + 3) * SLICE); // point to the start point of memory block in packet 1
        slice_offset_3 = ((slice_id + 4) * SLICE); // point to the start point of memory block in packet 2

        // load memory block of file into register
        mxx1 = _mm256_loadu_si256((__m256i *) &input_chunk_4[slice_offset_2]);

        // xor and store in packet 1
        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_1[slice_offset_1]));
        _mm256_storeu_si256((__m256i *) &parity_packet_1[slice_offset_1], mxx2);

        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_2[slice_offset_2]));
        _mm256_storeu_si256((__m256i *) &parity_packet_2[slice_offset_2], mxx2);

        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_3[slice_offset_3]));
        _mm256_storeu_si256((__m256i *) &parity_packet_3[slice_offset_3], mxx2);
    }

    for (slice_id = 0; slice_id < TOTAL_SLICE_NUM; slice_id++)
    {
        slice_offset_2 = slice_id * SLICE; // point to the start point of memory block (packet $slice_id)
        slice_offset_1 = ((slice_id + 4) * SLICE); // point to the start point of memory block in packet 1
        slice_offset_3 = ((slice_id + 3) * SLICE); // point to the start point of memory block in packet 2

        // load memory block of file into register
        mxx1 = _mm256_loadu_si256((__m256i *) &input_chunk_5[slice_offset_2]);

        // xor and store in packet 1
        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_1[slice_offset_1]));
        _mm256_storeu_si256((__m256i *) &parity_packet_1[slice_offset_1], mxx2);

        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_2[slice_offset_2]));
        _mm256_storeu_si256((__m256i *) &parity_packet_2[slice_offset_2], mxx2);

        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_3[slice_offset_3]));
        _mm256_storeu_si256((__m256i *) &parity_packet_3[slice_offset_3], mxx2);
    }

    // solve chunk 6, 7, 8 in 256-bitwise
    for (slice_id = 0; slice_id < TOTAL_SLICE_NUM; slice_id++)
    {
        /* Step 1: solve 256 bits in packet 6 */
        slice_offset_1 = ((slice_id + 5) * SLICE);
        slice_offset_2 = slice_id * SLICE;
        slice_offset_3 = ((slice_id + 2) * SLICE);

        // directly use leftmost block of parity packet 1 as message of packet 6
        mxx1 = _mm256_loadu_si256((__m256i *) &parity_packet_1[slice_offset_1]);
        _mm256_storeu_si256((__m256i *) &output_chunk_6[slice_offset_2], mxx1);

        // back substitute packet 6 to parity packet 2 and 3
        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_2[slice_offset_2]));
        _mm256_storeu_si256((__m256i *) &parity_packet_2[slice_offset_2], mxx2);
        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_3[slice_offset_3]));
        _mm256_storeu_si256((__m256i *) &parity_packet_3[slice_offset_3], mxx2);


        // Step 2: solve 256 bits in packet 8
        slice_offset_1 = ((slice_id + 7) * SLICE);
        slice_offset_2 = slice_id * SLICE;
        slice_offset_3 = slice_id * SLICE;

        // directly use leftmost block of parity packet 3 as message of packet 8
        mxx1 = _mm256_loadu_si256((__m256i *) &parity_packet_3[slice_offset_3]);
        _mm256_storeu_si256((__m256i *) &output_chunk_8[slice_offset_2], mxx1);

        // back substitute packet 6 to parity packet 1 and 2
        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_1[slice_offset_1]));
        _mm256_storeu_si256((__m256i *) &parity_packet_1[slice_offset_1], mxx2);
        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_2[slice_offset_2]));
        _mm256_storeu_si256((__m256i *) &parity_packet_2[slice_offset_2], mxx2);


        // Step 3: solve 256 bits in packet 7
        slice_offset_1 = ((slice_id + 6) * SLICE);
        slice_offset_2 = slice_id * SLICE;
        slice_offset_3 = ((slice_id + 1) * SLICE);

        // directly use leftmost block of parity packet 1 as message of packet 7
        mxx1 = _mm256_loadu_si256((__m256i *) &parity_packet_2[slice_offset_2]);
        _mm256_storeu_si256((__m256i *) &output_chunk_7[slice_offset_2], mxx1);

        // back substitute packet 7 to parity packet 1 and 3
        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_1[slice_offset_1]));
        _mm256_storeu_si256((__m256i *) &parity_packet_1[slice_offset_1], mxx2);
        mxx2 = _mm256_xor_si256(mxx1, *((__m256i *) &parity_packet_3[slice_offset_3]));
        _mm256_storeu_si256((__m256i *) &parity_packet_3[slice_offset_3], mxx2);
    }
}

void validate_recovered_part() {
    if (strcmp((char *) output_chunk_6, (char *) input_chunk_6) == 0
        && strcmp((char *) output_chunk_7, (char *) input_chunk_7) == 0
        && strcmp((char *) output_chunk_8, (char *) input_chunk_8) == 0)
    {
        PRINTF_STAMP("output files are equal!\n");
    }
}

void store_parity_file() {
//    STORE_FILE(1, 0);
//    STORE_FILE(2, 0);
//    STORE_FILE(3, 0);
//    STORE_FILE(4, 0);
//    STORE_FILE(5, 0);
//    STORE_FILE(6, 0);
//    STORE_FILE(7, 0);
//    STORE_FILE(8, 0);

    PRINTF_STAMP("file saved!\n");
}

int main() {
    PRINTF_STAMP("encoding invoked!\n");
    PRINTF_STAMP("input file size: %d byte\n", FILE_SIZE);
    PRINTF_STAMP("chunk size: %d byte\n", CHUNK_SIZE);

    double tic, toc;
    double exec_time_in_sec, throughput_in_MBps;
    std::fstream FileIn(ENCODE_FILE, std::ios::in | std::ios::out | std::ios::binary);
    if (FileIn.is_open())
    {
        FileIn.read((char *) input_chunk_1, CHUNK_SIZE);
        FileIn.read((char *) input_chunk_2, CHUNK_SIZE);
        FileIn.read((char *) input_chunk_3, CHUNK_SIZE);
        FileIn.read((char *) input_chunk_4, CHUNK_SIZE);
        FileIn.read((char *) input_chunk_5, CHUNK_SIZE);
        FileIn.read((char *) input_chunk_6, CHUNK_SIZE);
        FileIn.read((char *) input_chunk_7, CHUNK_SIZE);
        FileIn.read((char *) input_chunk_8, CHUNK_SIZE);
        FileIn.close();
        PRINTF_STAMP("file %s have been load into memory\n", ENCODE_FILE);
        PRINTF_STAMP("start encoding......\n");
        tic = clock();
        for (uint32_t i = 0; i < SAMPLE_NUM; i++)
            encode();
        toc = clock();

        exec_time_in_sec = (toc - tic) / (CLOCKS_PER_SEC / 1000);
        throughput_in_MBps = (SAMPLE_NUM * (FILE_SIZE + 3 * CHUNK_SIZE)) / (exec_time_in_sec * 1024);
        PRINTF_STAMP("encoding finished!\n");
        PRINTF_STAMP("throughput: %.4f MBps\n", throughput_in_MBps);

        PRINTF_STAMP("start decoding......\n");
        tic = clock();
        decode();
        toc = clock();
        exec_time_in_sec = (toc - tic) / (CLOCKS_PER_SEC / 1000);
        throughput_in_MBps = (FILE_SIZE) / (exec_time_in_sec * 1024);
        PRINTF_STAMP("decoding finished!\n");
        PRINTF_STAMP("throughput: %.4f MBps\n", throughput_in_MBps);

        validate_recovered_part();

        PRINTF_STAMP("storing encoded result to files......\n");
//        store_parity_file();
    } else
    {
        PRINTF_ERR_STAMP("cannot open file dummy.txt. exit.\n");
        return 255;
    }
    PRINTF_STAMP("all subroutine of encoding have finished. exit.\n");
    return 0;
}
