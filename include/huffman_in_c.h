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
        huff_node hn = {1, cf[i].freq};
        vector_push(&tree, &hn);
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

    vector_free(&tree);
    return (huffman){total_chars, codes};
}

// len=# of unique charcter
uint8* huffman_cmpres(huffman h, char_freq* cf, int cflen, uint8* str, int str_len, int* cmp_len)
{
    // main dictionary used to transorm char in str to code in h.codes
    huff_code dict[256];
    for(int i=0; i < cflen; i++)
        dict[cf[i].value] = h.codes[i];

    uint8* fstr = malloc((cflen * 4) + (h.total_chars * h.codes[0].bit_count) + 10);

    int i = 0;
    //table size
    fstr[i++] = cflen & 255;

    // table
    for(int j=0; j < cflen; j++) {

        for(int k=0; k < 2; k++)
            fstr[i++] = (h.codes[j].code >> (8*k)) & 0x0FF;

        fstr[i++] = h.codes[j].bit_count;
        fstr[i++] = cf[j].value;
    }

    // bit count, little endian, 8 bytes
    for(int j=0; j < 8; j++)
        fstr[i++] = (h.total_chars >> (8*j)) & 0xFF;

    int bit_pos = 8;
    int b = 0;

    for(int j=0; j < str_len; j++) {

        huff_code hc = dict[*str++];
        int bit_count = hc.bit_count;
        int code = hc.code;

        // current byte has enough available bits for code bits to fit
        if (bit_pos > bit_count) {
            bit_pos -= bit_count;
            b |= code << bit_pos;
        }
        // current byte does not have enough room so keep writing bytes until bit_count is less than 1 byte
        else  {
            bit_count -= bit_pos;
            b |= code >> bit_count;
            fstr[i++] = b;
            while (bit_count >= 8) {
                bit_count -= 8;
                fstr[i++] = code >> bit_count;
            }
            b = code << (8 - bit_count);
            bit_pos = 8 -  bit_count;
        }

        // check if end of byte is reached, if yes, write then reset
        if (bit_pos == 0) {
            fstr[i++] = b;
            bit_pos = 8;
            b = 0;
        }
    }

    if (bit_pos != 8) {
        fstr[i++] = b;
    }

    fstr[i] = '\0';
    *cmp_len = i;
    return fstr;
}

uint8* huffman_decompress(uint8* str, int* len)
{
    // getting tablee
    int i = 0;

    int table_size = str[i++];
    if (table_size == 0)
        table_size = 256;
    printf("table size: %d\n", table_size);

    #define MAX 65536
    #define REP 5

    uint8 bit_count[MAX][REP] = {0};
    uint8 values[MAX][REP] = {0};
    uint8 length[MAX] = {0};

    // memset(bit_count, 0, max * rep);
    // memset(values, 0, max * rep);
    // memset(length, 0, max);

    int code = 0;
    for(int j=0; j < table_size; j++) {

        code = str[i++];
        code |= (str[i++] << 8); // code
        int len = length[code];

        bit_count[code][len] = str[i++];
        values[code][len] = str[i++];
        length[code]++;
    }

    // get fewest_bits
    int fewest_bits = bit_count[code][length[code]-1]; //smallest code by bit count

    // get 8 bytes total-bits
    size_t total_chars=0;
    for(int j=0; j < 8; j++)
        total_chars |= str[i++] << (j*8);

    uint8* result = malloc(total_chars + 1);

    int bit_pos = 8;
    int b = str[i++];

    for(int j=0; j < total_chars; j++) {

        int c = 0, m;
        int cbc = fewest_bits; // CurrentBitCount

        do {
            if (bit_pos >= cbc)
                c = b >> (bit_pos - cbc);
            else {
                b = (b << 8) | str[i++];
                bit_pos += 8;
                c = b >> (bit_pos - cbc);
            }

            for(m=0; m < length[c] && bit_count[c][m] != cbc; m++)
                ;
            ++cbc;
        }
        while (m == length[c]);

        bit_pos -= (cbc-1);
        b &= (1 << bit_pos) - 1; // clear out bits left of bit_pos
        result[j] = values[c][m];
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
