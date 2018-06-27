#pragma once
#include <math.h>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <queue>

// build_huffman_tree code from http://cs.umw.edu/~finlayson/class/spring17/cpsc340/samples/huffman/huffman.cpp

struct huff_node {
    huff_node* left = nullptr;
    huff_node* right = nullptr;
    unsigned int code = 0;
    int bit_count = 0;
    int freq = 0;
    int value = 0; // 0 - 255 for byte, -1 for sum node
};

struct dictionary {
    int code = 0;
    int bit_count = 0;
    int value = 0;
};

struct nodes_comp {
    bool operator()(huff_node*& a, huff_node*& b) const {
        return a->freq > b->freq;
    }
};

std::vector<huff_node> byte_frequency(uint8* str, int str_len)
{
    std::vector<huff_node> bf(256);
    for (int i=0; i < str_len; i++) {
        bf[(uint8)str[i]].value = str[i];
        bf[(uint8)str[i]].freq++;
    }
    // sort the frequency of bytes from low to high
    std::sort(bf.begin(), bf.end(), [](const huff_node & a, const huff_node & b) -> bool {return a.freq < b.freq;});
    return bf;
}

void assign_code(huff_node* node, int code, int bit_count) {

    if(node == nullptr) 
        return;

    // not sum node
    if(node->value != -1) {
        node->code = code;
        node->bit_count = bit_count;
    }

    if (bit_count > 31) {
        std::cout << "ERROR: data cannot be compressed since huffman tree has exceeded depth 16";
        exit(1);
    }

    int left_code = (code << 1);
    int right_code = (code << 1) | 1;

    // traverse the tree and assign left branches code + 0, and right branches code + 1
    assign_code(node->left, left_code, bit_count+1);
    assign_code(node->right, right_code, bit_count+1);
}

void build_huffman_tree(std::vector<huff_node>& hn)
{
    // ascending priority_queue
    std::priority_queue<huff_node*, std::vector<huff_node*>, nodes_comp> nodes;  

    for (int i=0; i < hn.size(); i++)
        nodes.push((huff_node*)&hn[i]);

    // create node connections
    while(nodes.size() > 1) {
        huff_node* a = nodes.top();
        nodes.pop();
        huff_node* b = nodes.top();
        nodes.pop();

        huff_node* sum_node = new huff_node;
        sum_node->value = -1; // sum now
        sum_node->freq = a->freq + b->freq;
        sum_node->left = a;
        sum_node->right = b;
        nodes.push(sum_node);
    }

    huff_node* root = nodes.top();

    assign_code(root, 0, 0);
}

// len=# of unique charcter
std::vector<uint8> huffman_cmpres(uint8* str, int str_len, const std::vector<huff_node>& hn)
{
    // main dictionary that holds the byte value, huffman code bit count and huffman code
    dictionary dict[256];
    for(int i=0; i < hn.size(); i++) {
        dict[hn[i].value].bit_count = hn[i].bit_count;
        dict[hn[i].value].code = hn[i].code;
    }

    std::vector<uint8> cstr; //compressed string
    cstr.reserve(str_len);

    //magic code
    cstr.push_back('H');
    cstr.push_back('U');
    cstr.push_back('F');
    cstr.push_back('F');
    
    //table size
    cstr.push_back(hn.size()); // if size == 256, then 0 is recorded

    // table
    for(int j=0; j < hn.size(); j++) {
        cstr.push_back(hn[j].code >> 16);
        cstr.push_back(hn[j].code >> 8);
        cstr.push_back(hn[j].code);

        cstr.push_back(hn[j].bit_count);
        cstr.push_back(hn[j].value);
    }

    //record file size, 4 bytes
    cstr.push_back(str_len >> 24);
    cstr.push_back(str_len >> 16);
    cstr.push_back(str_len >> 8);
    cstr.push_back(str_len);

    int bit_pos = 8;
    int b = 0;

    //packing codes into bytes and storing the bytes in the vector 
    for(int j=0; j < str_len; j++) {

        dictionary d = dict[*str++];
        int bit_count = d.bit_count;
        int code = d.code;

        // current byte has enough available bits for code bits to fit
        if (bit_pos > bit_count) {
            bit_pos -= bit_count;
            b |= code << bit_pos;
        }
        // current byte does not have enough room so keep writing bytes until bit_count is less than 1 byte
        else  {
            bit_count -= bit_pos;
            b |= code >> bit_count;
            cstr.push_back(b);
            while (bit_count >= 8) {
                bit_count -= 8;
                cstr.push_back(code >> bit_count);
            }
            b = code << (8 - bit_count);
            bit_pos = 8 -  bit_count;
        }

        // check if end of byte is reached, if yes, write then reset
        if (bit_pos == 0) {
            cstr.push_back(b);
            bit_pos = 8;
            b = 0;
        }
    }

    if (bit_pos != 8) {
        cstr.push_back(b);
    }

    return cstr;
}

std::vector<uint8> huffman_decompress(uint8* str, int len)
{
    int i = 0;

    std::string magic((char*)str, 4); // first 4 characters must be HUFF
    if (magic != "HUFF") {
        std::cout << "ERROR: file is not a HUFF file";
        exit(1);
    }
    i+=4;

    // getting tablee
    int table_size = str[i++];
    if (table_size == 0)
        table_size = 256;
    // printf("table size: %d\n", table_size);

    dictionary dict[table_size];
    for(int j=0; j < table_size; j++) {
        int code = (str[i] << 16) | (str[i+1] << 8) | str[i+2];
        i += 3;
        dict[j].code = code;
        dict[j].bit_count = str[i++];
        dict[j].value = str[i++];
    }

    int decompressed_len = (str[i] << 24) | (str[i+1] << 16) | (str[i+2] << 8) | str[i+3];
    i+=4; 

    // get fewest_bits
    int fewest_bits = dict[table_size-1].bit_count;

    // get 8 bytes total-bits
    std::vector<uint8> dstr; //decompressed file
    dstr.reserve(len * 2); // reserve some memory, not exact

    int bit_pos = 8;
    int b = str[i++];
    int val;

    for(int j=0; j < decompressed_len; j++) {

        int c = 0, next = 0;
        int cbc = fewest_bits; // CurrentBitCount

        do {
            if (bit_pos >= cbc)
                c = b >> (bit_pos - cbc);
            else {
                b = (b << 8) | str[i++];
                bit_pos += 8;
                c = b >> (bit_pos - cbc);
            }
            
            // search dict for code and bit_count match
            for (int j=0; j < table_size; j++) {
                if (dict[j].bit_count == cbc && dict[j].code == c) {
                    val = dict[j].value;
                    next = 1;
                }
            }
            ++cbc;
        }
        while (next == 0);

        bit_pos -= (cbc-1);
        b &= (1 << bit_pos) - 1; // clear out bits left of bit_pos
        dstr.push_back(val);
    }

    return dstr;
}

// wrapper function for compression with huffman coding
// input: data pointer, data length in bytes
// returns vector containing compressed data
std::vector<uint8> huffman_compress(uint8* str, int str_len)
{
    std::vector<huff_node> hn = byte_frequency(str, str_len); // count frequency of each byte in the data
    auto nonzero_iterator = std::find_if(hn.begin(), hn.end(), [](const huff_node& a) -> bool {return a.freq != 0;}); 
    hn = std::vector<huff_node> (nonzero_iterator, hn.end()); // only keep bytes whose frequency is greater than zero
    build_huffman_tree(hn);
    std::vector<uint8> cfile = huffman_cmpres(str, str_len, hn);
    return cfile;
}

// sort(hn.begin(), hn.end(), [](const huff_node& a, const huff_node& b) -> bool {return a.code < b.code;});
// for (int i=0; i < hn.size(); i++)
//     std::cout << hn[i].value << " "  << hn[i].freq << " " << hn[i].code << " " << hn[i].bit_count << std::endl;
    
