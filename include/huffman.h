#pragma once
#include <math.h>

/* needs
#include "vector.h"
*/

// used before huffman to measure frequencies
typedef struct char_freq {
    int freq;
    uint8 value;
} char_freq;

typedef struct huff_code {
    uint16 code;
    uint8 bit_count;
} huff_code;

typedef struct huffman {
    size_t total_chars;
    huff_code* codes;
} huffman;

int __char_frequency_sort_comp(const void* a, const void* b)
{
    return ((char_freq*)a)->freq - ((char_freq*)b)->freq;
}

char_freq* char_frequency(uint8* str, int str_len, char_freq* cf, int* cf_len)
{
    for (int i=0; i < str_len; i++) {
        cf[(uint8)str[i]].value = str[i];
        cf[(uint8)str[i]].freq++;
    }

    // sort the frequency of bytes from low to high
    qsort(cf, 256, sizeof(char_freq), __char_frequency_sort_comp);

    // return the first non zero frequency byte index
    int i;
    for(i=0; i < 256 && cf[i].freq == 0; i++)
        ;
    *cf_len = 256 - i;
    return i + cf;
}

huffman build_huffman_tree(char_freq* cf, int cf_len)
{
    // only used in tree construction, internal
    typedef struct {
        char flag; // flag = 0 for sum, 1 for actual char
        int freq;
        int parent;
    } huff_node;

    vector tree; vector_init(&tree, sizeof(huff_node), cf_len*2);
    huff_code* codes = malloc(cf_len*sizeof(huff_code)+1);

    // empty tree in a vector
    for(int i=0; i < cf_len; i++) {
        huff_node hv = {1, cf[i].freq};
        vector_push(&tree, &hv);
    }

    // build tree and insert sums in correct positions, -1 flag for sums
    for(int i=0; i < tree.length-1; i++) {

        huff_node* hn1 = vector_index(&tree, i++);
        huff_node* hn2 = vector_index(&tree, i);
        int sum = hn1->freq + hn2->freq;

        int pos = i+1;
        huff_node* hv = tree.begin;
        while(pos < tree.length && sum >= hv[pos].freq)
            pos++;

        hn1->parent = pos;
        hn2->parent = pos;

        huff_node hn = {0, sum};
        vector_insert(&tree, &hn, pos);
    }

    // get huffman code
    huff_node* hn = tree.begin;
    size_t total_chars = 0;
    int j = 0;
    int ROOT = tree.length-1;
    for(int i=0; i < ROOT; i++) {
        if(hn[i].flag) {
            int index = i;
            int shift = 0;
            int code = 0;
            total_chars += hn[index].freq;
            do
                code |= ((index % 2) << shift++);
            while((index = hn[index].parent) != ROOT);
            codes[j].code = code;
            codes[j].bit_count = shift;
            ++j;
        }
    }

    vector_dest(&tree);
    return (huffman){total_chars, codes};
}

// len=# of unique charcter
uint8* huffman_cmpres(huffman h, char_freq* cf, int cflen, uint8* str, int str_len, int* cmp_len)
{
    // main dictionary used to transorm char in str to code in h.codes
    huff_code dict[256];
    for(int i=0; i < cflen; i++)
        dict[cf[i].value] = h.codes[i];

    // printf("cflen: %d, h.total_chars %d, h.codes[0].bit_count %d\n", cflen,h.total_chars,h.codes[0].bit_count);
    size_t malloc_size = (cflen * 4) + (h.total_chars * h.codes[0].bit_count) + 10;
    uint8* fstr = malloc(malloc_size);

    int i = 0;

    //table size
    for(int k=0; k < 2; k++)
        fstr[i++] = (cflen >> (8*k)) & 0x0FF;

    // table
    for(int j=0; j < cflen; j++) {

        for(int k=0; k < 2; k++)
            fstr[i++] = (h.codes[j].code >> (8*k)) & 0x0FF;

        fstr[i++] = h.codes[j].bit_count;
        fstr[i++] = cf[j].value;
    }

    // bit count, little endian, 8 bytes
    for(int j=0; j < 8; j++)
        fstr[i++] = (h.total_chars >> (8*j)) & 0x0FF;

    // convertion
    int byte_count = 0;
    int bit_pos = 8;
    int b = 0;

    for(int j=0; j < str_len; j++) {

        int c = *str++;
        huff_code hc = dict[c];
        int bit_count = hc.bit_count;
        int code = hc.code;

        int avail = bit_pos - bit_count;

        // current byte has enough available bits for code bits to fit
        if (avail >= 0) {
            b |= code << avail;
            // debug("c: %d, b:  %d in avail >= 0\n", code, b);
            bit_pos -= bit_count;
            if (bit_pos == 0) {

                fstr[i++] = b;
                byte_count++;
                // debug("wrote when avail >= 0: %d\n", fstr[i-1]);
                bit_pos = 8;
                b = 0;
            }
        }

        // byte does not have enough room so fit in as many as bits from code as possible
        else  {
            do {
                b |= code >> (bit_count - bit_pos);
                // debug("shifted: %d, in bit pos > 0\n", bit_count - bit_pos);
                fstr[i++] = b;
                bit_count -= bit_pos;
                bit_pos = 8;
                // debug("c: %c, b: %d in bit_pos > 0\n", c, b);
                // debug("wrote when bit_pos > 0: %d\n", fstr[i-1]);
                b = 0;
                byte_count++;
            }
            while (bit_count >= bit_pos);
            b |= code << (bit_pos - bit_count);
            bit_pos -= bit_count;
        }
    }

    if (bit_pos != 8) {
        fstr[i++] = b;
        byte_count++;
        // debug("wrote when bit_pos != 8: %d\n", fstr[i-1]);
    }

    fstr[i] = '\0';
    *cmp_len = i;
    return fstr;
}

uint8* huffman_decompress(uint8* str, int* len)
{
    // getting tablee
    int i = 0;

    int table_size = 0;
    for(int j=0; j < 2; j++)
        table_size |= str[i++] << (j*8);
    // printf("table size: %d\n", table_size);


    int code = str[i++];
    code |= (str[i++] << 8);

    int max_bit_count = str[i++];
    i-=3;

    // int max_code = 1 << (max_bit_count+1);

    int rep = 5;
    int max = 50000;

    uint8 bit_count[max][rep];
    uint8 values[max][rep];
    uint8 length[max];

    memset(bit_count, 0, max * rep);
    memset(values, 0, max * rep);
    memset(length, 0, max);

    for(int j=0; j < table_size; j++) {

        code = str[i++];
        code |= (str[i++] << 8); // code
        int len = length[code];

        bit_count[code][len] = str[i++]; // bit_count
        values[code][len] = str[i++]; //value
        length[code]++;

        // printf("code: %d ", code);
        // printf("bit_count: %d ", bit_count[code][len]);
        // printf("Val: %c\n", values[code][len]);
    }

    // get fewest_bits
    int fewest_bits = bit_count[code][length[code]-1];

    // get 8 bytes total-bits
    size_t total_chars=0;
    for(int j=0; j < 8; j++)
        total_chars |= str[i++] << (j*8);
    // printf("total_chars %llu\n\n", total_chars);

    uint8* result = malloc(total_chars + 1);

    int bit_pos = 8;
    int c = str[i++];

    for(int j=0; j < total_chars; j++) {

        int b = 0, m = 0;
        int cbc = fewest_bits - 1; // current bit count, offset from bit_pos

        do {
            ++cbc;
            if (bit_pos >= cbc) {
                b = bit_range(c, bit_pos, cbc);
                // debug("in bit_pos >= cbc! b: %d, cbc: %d, bit_pos: %d\n", b, cbc, bit_pos);
            }
            else if (bit_pos > 0) {
                b = bit_range(c, bit_pos, bit_pos);
                c = (b << 8) | str[i++];
                cbc = fewest_bits - 1;
                bit_pos += 8;
                // debug("in bit_pos > 0, b: %d, c: %d, cbc: %d, bit_pos: %d\n", b, c, cbc, bit_pos);
            }
            else {
                // debug("blah........\n");
                bit_pos = 8;
                c = str[i++];
                cbc = fewest_bits - 1;
                b = 0;
            }

            if (cbc >= fewest_bits)
                for(m=0; m < length[b] && bit_count[b][m] != cbc; m++)
                    ;
            else
                m = length[b];
        }
        while (m == length[b]);

        bit_pos -= cbc;
        // debug("output %c\n", values[b][m], m, b);
        result[j] = values[b][m];
    }
    result[total_chars] = '\0';
    *len = total_chars;
    return result;
}

uint8* huffman_compress(uint8* str, int str_len, int* compress_len)
{
    char_freq cf[256] = {0};
    int cf_len;
    char_freq* cf_main = char_frequency(str, str_len, cf, &cf_len);
    huffman h = build_huffman_tree(cf_main, cf_len);
    uint8* ret =  huffman_cmpres(h, cf_main, cf_len, str, str_len, compress_len);
    free(h.codes);
    return ret;
}

// for(int j=0; j < total_chars; j++) {
//
//     int b = 0, m = 0;
//     int cbc = max_bit_count + 1; // current bit count, offset from bit_pos
//
//     do {
//         --cbc;
//         if (bit_pos >= cbc) {
//             b = bit_range(c, bit_pos, cbc);
//             debug("in bit_pos >= cbc! b: %d, cbc: %d, bit_pos: %d\n", b, cbc, bit_pos);
//         }
//         else if (bit_pos > 0) {
//             b = bit_range(c, bit_pos, bit_pos);
//             c = (b << 8) | str[i++];
//             cbc = max_bit_count + 1;
//             bit_pos += 8;
//             debug("in bit_pos > 0, b: %d, c: %d, cbc: %d, bit_pos: %d\n", b, c, cbc, bit_pos);
//         }
//         else {
//             debug("blah........\n");
//             bit_pos = 8;
//             c = str[i++];
//             cbc = max_bit_count + 1;
//             b = 0;
//         }
//
//         if (b < max)
//             for(m=0; m < length[b] && bit_count[b][m] != cbc; m++)
//                 ;
//         else {
//             b = 0;
//             m =  length[b] + 1;
//         }
//     }
//     while (m == length[b]);
