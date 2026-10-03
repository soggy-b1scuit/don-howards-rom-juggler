#include <cstdio>
#include <juggler.hpp>
#define XXH_INLINE_ALL
#include "xxhash.h"
#include <zlib-ng.h>
#include <cstring>

struct patch_header
{
    char identifier[8] = {'D','O','N','P','X','T','C','H'};
    size_t original_size;
    size_t modified_size;
    size_t compressed_size;
    XXH64_hash_t original_hash;
    XXH64_hash_t modified_hash;
};

#define CSTR(string_view) (const char*)(string_view).data()
int create_patch(std::string_view file_original, std::string_view file_modified, std::string_view file_patch)
{
    patch_header header;

    //read unmodified rom to store hash and size
    FILE *f = fopen(CSTR(file_original), "rb");
    if (!f)
    {
        fclose(f);
        return ERROR_FILE1_ACCESS;
    }

    fseek(f, 0, SEEK_END);
    header.original_size = ftell(f);

    fseek(f, 0, SEEK_SET);

    char *original_data = new char[header.original_size];
    fread(original_data, sizeof(char), header.original_size, f);

    fclose(f);

    header.original_hash = XXH3_64bits(original_data, header.original_size);

    //read modified rom to store hash and size
    f = fopen(CSTR(file_modified), "rb");
    if (!f)
    {
        fclose(f);
        delete[] original_data;
        return ERROR_FILE2_ACCESS;        
    }
    
    fseek(f, 0, SEEK_END);
    header.modified_size = ftell(f);

    fseek(f, 0, SEEK_SET);

    char *modified_data = new char[header.modified_size];
    fread(modified_data, sizeof(char), header.modified_size, f);

    fclose(f);

    header.modified_hash = XXH3_64bits(modified_data, header.modified_size);

    if (header.original_hash == header.modified_hash)
    {
        delete[] original_data;
        delete[] modified_data;
        return ERROR_SAME_HASH;        
    }

    char *patch_data = new char[header.modified_size];

    for (size_t i = 0; i < header.modified_size; i++)
    {
        size_t pos = i % header.original_size;
        patch_data[i] = (modified_data[i] ^ original_data[header.original_size - pos - 1] ^ original_data[pos]);
    }

    header.compressed_size = zng_compressBound(header.modified_size);
    char *compressed_patch_data = new char[header.compressed_size];
    
    int zresult = zng_compress((uint8_t *)compressed_patch_data, &header.compressed_size, (uint8_t *)patch_data, header.modified_size);
    if (zresult)
        exit(zresult);
    f = fopen(CSTR(file_patch), "wb");
    if (!f)
    {
        fclose(f);
        delete[] original_data;
        delete[] modified_data;
        delete[] compressed_patch_data;
        delete[] patch_data;
        return ERROR_FILE3_ACCESS;        
    }

    fwrite(&header, sizeof(patch_header), 1, f);

    fwrite(compressed_patch_data, sizeof(char), header.compressed_size, f);

    fclose(f);

    delete[] original_data;
    delete[] modified_data;
    delete[] compressed_patch_data;
    delete[] patch_data;

    return ERROR_OK;
}
int apply_patch(std::string_view file_original, std::string_view file_patch, std::string_view file_modified)
{
    //read original rom 
    FILE *f = fopen(CSTR(file_original), "rb");
    if (!f)
    {
        fclose(f);
        return ERROR_FILE1_ACCESS;        
    }

    fseek(f, 0, SEEK_END);
    size_t original_size = ftell(f);

    fseek(f, 0, SEEK_SET);
    char *original_data = new char[original_size];
    fread(original_data, sizeof(char), original_size, f);

    fclose(f);

    XXH64_hash_t original_hash = XXH3_64bits(original_data, original_size);

    //read patch-
    f = fopen(CSTR(file_patch), "rb");
    if (!f)
    {
        fclose(f);
        delete[] original_data;
        return ERROR_FILE2_ACCESS;
    }
    patch_header header;

    fread(&header, sizeof(patch_header), 1, f);
    
    //compare header identifier
    if (!strcmp(header.identifier,"DONPXTCH"))
    {
        fclose(f);
        delete[] original_data;
        return ERROR_INVALID_FILE;
    }
    //compare header unmodified rom hash to current 
    if (header.original_hash != original_hash)
    {
        fclose(f);
        delete[] original_data;
        return ERROR_WRONG_HASH;
    }

    char *patch_data = new char[header.modified_size * 2]; //idk why it needs extra space to work

    char *compressed_patch_data = new char[header.compressed_size];

    fread(compressed_patch_data, sizeof(char), header.compressed_size, f);

    fclose(f);

    size_t uncompressed_size;

    int zresult = zng_uncompress((uint8_t *)patch_data, &uncompressed_size, (uint8_t *)compressed_patch_data, header.compressed_size);
    if (zresult)
        exit(zresult);
    if (uncompressed_size != header.modified_size)
    {
        return ERROR_INVALID_FILE;
    }
    char *modified_data = new char[header.modified_size];

    for (size_t i = 0; i < header.modified_size; i++)
    {
        size_t pos = i % header.original_size;
        modified_data[i] = (patch_data[i] ^ original_data[header.original_size - pos - 1] ^ original_data[pos]);
    }

    XXH64_hash_t modified_hash = XXH3_64bits(modified_data, header.modified_size);

    if (header.modified_hash != modified_hash)
    {
        fclose(f);
        delete[] original_data;
        delete[] patch_data;
        delete[] compressed_patch_data;
        delete[] modified_data;
        return ERROR_WRONG_OUTPUT_HASH;
    }

    f = fopen(CSTR(file_modified), "wb");

    if (!f)
    {
        fclose(f);
        delete[] original_data;
        delete[] patch_data;
        delete[] compressed_patch_data;
        delete[] modified_data;
        return ERROR_FILE3_ACCESS;
    }

    fwrite(modified_data, sizeof(char), header.modified_size, f);

    fclose(f);

    delete[] original_data;
    delete[] patch_data;
    delete[] compressed_patch_data;
    delete[] modified_data;
    return ERROR_OK;
}