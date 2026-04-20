#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "encode.h"
#include "types.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

/* Function to calculate total image capacity in bytes */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    //Declare variables for width and height
    uint width, height;
    //Seek to 18th Bit
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);//Read 4 bytes from file into width
    printf("width = %u\n", width);//Print width

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);//Read 4 bytes from file into height
    printf("height = %u\n", height);//Print height

    fseek(fptr_image, 0, SEEK_SET);//Reset file pointer
    // Return image capacity
    return width * height * 3;//Calculate image data capacity and return
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

 /* Function to open source, secret, and stego files */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");//Open source image in read binary
    // Do Error handling
    //Check if file opened
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");//Print system error

    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);//Print custom error

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");//Open secret file in read binary
    // Do Error handling
    //Check if file opened
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");//Print system error

    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);//Print custom error

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");//Open stego file in write binary
    // Do Error handling
    //Check if file opened
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");//Print system error

    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);//Print custom error

    	return e_failure;
    }

    //If all files opened return e_success
    return e_success;
}

/* Function to parse command-line arguments */
Status read_and_validate_encode_args(int argc,char *argv[], EncodeInfo *encInfo)
{
    //Check the argc is less than 4 or argc is greater than 5
    if (argc < 4 || argc > 5)
    {
        printf("ERROR : Invalid number of arguments\n");
        return e_failure;
    }
    //check the source file argv[2] is having ".bmp" or not
    if (strstr(argv[2], ".bmp") == NULL)
    {
        printf("ERROR : Source image file must be a .bmp file\n");
        return e_failure;
    }
    else
    {
        //store the file name argv[2] into encInfo->src_image_fname
        encInfo->src_image_fname = argv[2];
        printf("INFO    : Source image file validated (%s)\n", encInfo->src_image_fname);
    }
    //check the secret file argv[3] is having "." or not
    if (strstr(argv[3], ".") == NULL)
    {
        printf("ERROR : Secret file must have a valid extension\n");
        return e_failure;
    }
    else
    {
        //store the secret file name argv[3] into encInfo->secret_fname
        encInfo->secret_fname = argv[3];
        printf("INFO    : Secret file validated (%s)\n", encInfo->secret_fname);
    }
    
    char *extn = strrchr(encInfo->secret_fname, '.');//Get last '.' to extract extension

    //Check extraction
    if (extn == NULL)
    {
        printf("ERROR : Unable to extract secret file extension\n");
        return e_failure;
    }
    strcpy(encInfo->extn_secret_file, extn);//Store extension in structure

    //check the stego file name is missing
    if (argc == 4)
    {
        //store the "Stego.bmp"(default stego filename)into the encInfo -> stego_image_fname
        encInfo->stego_image_fname = "Stego.bmp";
        printf("INFO    : Stego image not provided, using default name (%s)\n", encInfo->stego_image_fname);
    }
    else
    {
        //check the argv[4] stego file extension is ".bmp" or not
        if (strstr(argv[4], ".bmp") == NULL)
        {
            printf("ERROR : Stego image file must be a .bmp file\n");
            return e_failure;
        }
        //store the argv[4] into the encInfo -> stego_image_fname
        encInfo->stego_image_fname = argv[4];
        printf("INFO    : Stego image file validated (%s)\n", encInfo->stego_image_fname);
    }
    return e_success;
}

/* Function to calculate size of a file in bytes */
uint get_file_size(FILE *fptr)
{
    uint position;
    //use fseek to move file ptr to end of file
    fseek(fptr, 0, SEEK_END);
    //use ftell to get the current pos of the file ptr and return the pos
    position = ftell(fptr);
    //use fseek to move file pointer back to beginning
    fseek(fptr, 0, SEEK_SET);
    return position;
}

/* Check if image can hold secret data */
Status check_capacity(EncodeInfo *encInfo)
{
    //compare source file with destination file and return success or failure
    //Get image capacity (in bytes) 
    encInfo -> image_capacity = get_image_size_for_bmp(encInfo -> fptr_src_image);

    //Get secret file size 
    encInfo -> size_secret_file = get_file_size(encInfo -> fptr_secret);

    int required_size_capacity = (strlen(MAGIC_STRING) * 8) + (32) + (strlen(encInfo->extn_secret_file) * 8) + (32) + (encInfo->size_secret_file * 8);//Calculate required bits

    //Check whether image can hold the secret data 
    if (encInfo->image_capacity > required_size_capacity)
    {
        return e_success;
    }

    else
    {
        printf("ERROR : Image doesn't have sufficient capacity\n");
        return e_failure;
    }
}

/* Copy BMP header from source to stego */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{ 
    char buffer[54];//Initialize buffer to store BMP header

    //Read a 54 bytes from fptr_src_image and store the data into buffer
    if (fread(buffer, 1, 54, fptr_src_image) != 54)
    {
        printf("ERROR : Failed to read BMP header\n");
        return e_failure;
    }
    //Write a 54 bytes of buffer to fptr_dest_image
    if (fwrite(buffer, 1, 54, fptr_dest_image) != 54)
    {
        printf("ERROR : Failed to write BMP header\n");
        return e_failure;
    }

    //if there is read or write performed then return e_success
    return e_success;
}

/* Encode one byte of data into 8 bytes of image LSB */
Status encode_byte_to_lsb(char data, char *image_buffer)
{ 
    char bit;//Temporary variable to store each bit

    //Loop for 8 bits (MSB to LSB)
    for (int i = 0; i < 8; i++)
    {
       //Get the bit from data MSB to LSB
       bit = (data >> (7 - i)) & 0x01;

       //Clear the lsb bit of image_buffer[i] 
       image_buffer[i] = image_buffer[i] & 0xFE;

       //Set the extracted bit into the LSB of image_buffer[i]
       image_buffer[i] = image_buffer[i] | bit;
    }
    return e_success;
}
 
/* Encode multiple bytes into image */
Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char buffer[8];//Temporary buffer of 8 bytes for LSB encoding

    //loop for 8 times
    for (int i = 0; i < size; i++)
    {
        //Read 8 bytes from fptr_src_image and store the data into buffer
        if (fread(buffer, sizeof(char), 8, fptr_src_image) != 8)
        {
            printf("ERROR : Failed to read image data during encoding process\n");
            return e_failure;
        }

        //Encode one byte to lsb of (data[i], buffer);
        if (encode_byte_to_lsb(data[i], buffer) != e_success)
        {
            return e_failure;
        }

        //Write 8 bytes of buffer into the fptr_stego_image
        if (fwrite(buffer, sizeof(char), 8, fptr_stego_image) != 8)
        {
            printf("ERROR : Failed to write encoded image data\n");
            return e_failure;
        }
    }
    return e_success;
}

/* Encode predefined magic string */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    //Encode the magic string into the LSBs of the image 
    if (encode_data_to_image(magic_string, strlen(magic_string), encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
    {
        return e_failure;
    }
    return e_success;
}

/*  Encode 32-bit integer into 32 bytes of image LSB */
Status encode_size_to_lsb(int data, char *image_buffer)
{
    int bits;
    //loop for 32 bits (MSB to LSB)
    for (int i = 0; i < 32; i++)
    {
       //Get the bit from data (MSB - LSB)
       bits = (data >> (31 - i)) & 0x01;

       //Clear the lsb bit of image_buffer[i]
       image_buffer[i] = image_buffer[i] & 0xFE;

       //Set the extracted bit into the LSB of image_buffer[i]
       image_buffer[i] = image_buffer[i] | bits;
    }
    return e_success;
}

/* Encode size of file extension */
Status encode_secret_file_extn_size(long extn_file_size, EncodeInfo *encInfo)
{
    char buffer[32];//Initialize Buffer for 32 bytes
    //Read 32 bytes from fptr_src_image and store the data into buffer
    if (fread(buffer, sizeof(char), 32, encInfo->fptr_src_image) != 32)
    {
        printf("ERROR : Failed to read image data for extension size\n");
        return e_failure;
    }

    //Encode extension file size to lsb(data[i], buffer)
    encode_size_to_lsb((int)extn_file_size, buffer);

    //Write 32 bytes of buffer into the fptr_stego_image
    if (fwrite(buffer, sizeof(char), 32, encInfo->fptr_stego_image) != 32)
    {
        printf("ERROR : Failed to write extension size\n");
        return e_failure;
    }
    return e_success;
}

/* Encode file extension */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    //Encode the secret file extension into the stego image
    if(encode_data_to_image(file_extn, strlen(file_extn), encInfo -> fptr_src_image, encInfo -> fptr_stego_image)!= e_success)
    {
        return e_failure;
    }
    return e_success;
}

/* Encode secret file size */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{ 
    char buffer[32];//Initailize Buffer for 32 bytes

    //Read 32 bytes from fptr_src_image and store the data into buffer
    if (fread(buffer, sizeof(char), 32, encInfo->fptr_src_image) != 32)
    {
        printf("ERROR : Failed to read image data for secret file size\n");
        return e_failure;
    }

    //Encode secret_file_extension size to lsb(file_size, buffer)
    encode_size_to_lsb((int)file_size, buffer);

    //Write 32 bytes of buffer into the fptr_stego_image
    if (fwrite(buffer, sizeof(char), 32, encInfo->fptr_stego_image) != 32)
    {
        printf("ERROR : Failed to write secret file size\n");
        return e_failure;
    }
    return e_success;
}

/* Encode actual secret file content */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char *buffer = malloc(encInfo->size_secret_file);//Allocate memory for secret data
    //Check for allocation success
    if(buffer == NULL)
    {
        printf("ERROR : Memory allocation failed\n");
        return e_failure;
    }

    //Read a encInfo -> size_secret_file bytes from encInfo -> fptr_secret and store the data into char buffer
    if (fread(buffer, 1, encInfo->size_secret_file, encInfo->fptr_secret) != encInfo->size_secret_file)
    {
        printf("ERROR : Failed to read secret file\n");
        free(buffer);//Free memory
        return e_failure;
    }
    printf("INFO    : Encoding secret file data...\n");

    //Encode encInfo -> size_secret_file data into image
    if(encode_data_to_image(buffer, encInfo -> size_secret_file, encInfo -> fptr_src_image, encInfo -> fptr_stego_image) != e_success)
    {
        free(buffer);//Free memory on failure
        return e_failure;
    }
    free(buffer);//Free allocated memory
    return e_success;
}

/*  Copy rest of image after encoding */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char buffer;//Initailize temporary variable for single byte
    {
        //loop until End of File
        //Read 1 byte from fptr_src and store to buffer
        while (fread(&buffer, sizeof(char), 1, fptr_src) == 1)
        {
            //Write 1 byte of buffer info fptr_dest
            if (fwrite(&buffer, sizeof(char), 1, fptr_dest) != 1)
            {
                printf("ERROR : Failed to copy remaining image data\n");
                return e_failure;
            }
        }
        return e_success;
    }
}

/* Perform full encoding process */
Status do_encoding(EncodeInfo *encInfo)
{
    //open required files
    if(open_files(encInfo) != e_success) 
    {
        //print failure message
        printf("Error : Files not opened successfully");
        return e_failure;
    }
    //printf success message
    printf("SUCCESS : Files Opened Successfully\n");

    //Check image capacity
    if(check_capacity(encInfo) !=  e_success)
    {
        printf("ERROR : Insufficient image capacity\n");
        return e_failure;
    }
    //printf success message
    printf("SUCCESS : Image has sufficient capacity\n");

    //Copy BMP header
    if(copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image) != e_success)
    {
        printf("ERROR : Failed to copy BMP header\n");
        return e_failure;
    }
    printf("SUCCESS : BMP header copied\n");

    // Move file pointers to start encoding AFTER header
    fseek(encInfo->fptr_src_image, 54, SEEK_SET);//Move source pointer after header
    fseek(encInfo->fptr_stego_image, 54, SEEK_SET);// Move stego pointer after header

    //Encode magic string
    if (encode_magic_string(MAGIC_STRING, encInfo) != e_success)
    {
        printf("ERROR : Failed to encode magic string\n");
        return e_failure;
    }
    printf("SUCCESS : Magic string encoded\n");

    //Encode secret file extension size 
    long extn_file_size = strlen(encInfo->extn_secret_file);//Calculate extension length
    
    if (encode_secret_file_extn_size(extn_file_size, encInfo) != e_success)
    {
        printf("ERROR : Failed to encode secret file extension size\n");
        return e_failure;
    }
    printf("SUCCESS : Secret file extension size encoded\n");

    //Encode secret file extension 
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) != e_success)
    {
        printf("ERROR : Failed to encode secret file extension\n");
        return e_failure;
    }
    printf("SUCCESS : Secret file extension encoded\n");

    //Encode secret file size 
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) != e_success)
    {
        printf("ERROR : Failed to encode secret file size\n");
        return e_failure;
    }
    printf("SUCCESS : Secret file size encoded\n");

    //Encode secret file data
    if (encode_secret_file_data(encInfo) != e_success)
    {
        printf("ERROR : Failed to encode secret file data\n");
        return e_failure;
    }
    printf("SUCCESS : Secret file data encoded\n");

    //Copy remaining image data 
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
    {
        printf("ERROR : Failed to copy remaining image data\n");
        return e_failure;
    }
    printf("SUCCESS : Remaining image data copied\n");

    printf("SUCCESS : File Encoded successfully\n");
    return e_success;

}
