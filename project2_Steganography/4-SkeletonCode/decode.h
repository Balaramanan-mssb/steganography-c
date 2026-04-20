#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

/* 
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4
#define MAGIC_STRING "STEGO"


typedef struct _DecodeInfo
{

    /* Secret File Info */
    char *output_secret_fname;
    FILE *output_fptr_secret;
    char *extn_secret_file;
    char secret_data[MAX_SECRET_BUF_SIZE];
    long size_secret_file;
    long extn_file_size;

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

} DecodeInfo;


/* Decoding function prototype */



/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(int argc,char *argv[], DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_decode_files(DecodeInfo *decInfo);


/* skip bmp image header */
Status skip_bmp_header(FILE *fptr_src_image);

/* Decode Magic String */
Status decode_magic_string(DecodeInfo *encInfo);

Status decode_secret_file_extn_size(DecodeInfo *decInfo);

/* Decode secret file extenstion */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decode function, which does the real decoding */
Status decode_image_to_data(char *data, int size, FILE *fptr_stego_image);

/* Decode a LSB into byte of image data array */
Status decode_lsb_to_byte(char* data, char *image_buffer);// Not returning

//char decode_lsb_to_byte(char *image_buffer);// returning

Status decode_lsb_to_size(long* data, char *image_buffer);//for updating address

//char decode_lsb_to_size(int* data, char *image_buffer);//for returning address


#endif
