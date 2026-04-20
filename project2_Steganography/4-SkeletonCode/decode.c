#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "decode.h"
#include "types.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(int argc,char *argv[], DecodeInfo *decInfo)
{
    //Check argument count must be less than 3 and greater than 4
    if (argc < 3 || argc > 4)
    {
        return e_failure;
    }

    //Check stego image extension file is .bmp file or not
    if (strstr(argv[2], ".bmp") == NULL)
    {
        return e_failure;
    }

    //Store the argv[2] into stego image file name
    decInfo->stego_image_fname = argv[2];

    // Check output file name
    if (argc == 4)
    {
        //store the file name argv[3] into decInfo->output_secret_fname
        decInfo->output_secret_fname = argv[3];
    }
    else
    {
        //Default output file name
        decInfo->output_secret_fname = malloc(200);//Allocate memory for default output filename
        //Check allocation
        if (!decInfo->output_secret_fname)
        {
            return e_failure;
        }
        strcpy(decInfo->output_secret_fname, "decoded_secret_file");//Set default output filename
    }
    return e_success;
}

/* Get File pointers for i/p and o/p files */
/*  Open stego image and output file */
Status open_decode_files(DecodeInfo *decInfo)
{
    //Stego Image file
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");//Open stego image in read-binary mode
    //Check if file opened
    if (!decInfo->fptr_stego_image)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
    	return e_failure;
    }

    //Output secret file
    decInfo->output_fptr_secret = fopen(decInfo->output_secret_fname, "wb");//Open output file in write-binary mode
    //Check if file opened
    if (!decInfo->output_fptr_secret)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->output_secret_fname);
        fclose(decInfo->fptr_stego_image);//Close previously opened stego image 
    	return e_failure;
    }
    //Return success if both files opened
    return e_success;
}

/* skip bmp image header */
Status skip_bmp_header(FILE *fptr_src_image)
{
    //Check if file pointer is valid
    if (!fptr_src_image)
    {
        return e_failure;
    }
    //Move file pointer 54 bytes ahead and skip header
    if (fseek(fptr_src_image, 54, SEEK_SET) != 0)
    {
        return e_failure;
    }
    return e_success;
}

/* Decode Magic String */
Status decode_magic_string(DecodeInfo *decInfo)
{
    char magic[strlen(MAGIC_STRING) + 1];//Allocate buffer for magic string
    //Decode bytes into magic buffer
    if (decode_image_to_data(magic, strlen(MAGIC_STRING), decInfo->fptr_stego_image) != e_success)
    {
        return e_failure;
    }

    magic[strlen(MAGIC_STRING)] = '\0';//Null-terminate string

    //Compare decoded string with expected magic string
    if (strcmp(magic, MAGIC_STRING) != 0)
    {
        printf("ERROR : Magic string mismatch\n");
        return e_failure;
    }

    printf("SUCCESS : Magic string matched\n");
    return e_success;
}


/* Decode secret file extension size */
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];//Initialize temporary buffer for 32 bytes

    //Read 32 bytes from stego image
    if(fread(buffer, 1, 32, decInfo->fptr_stego_image) != 32)
    {
        printf("ERROR : Failed to read extension size from image\n");
        return e_failure;
    }

    decode_lsb_to_size(&decInfo->extn_file_size, buffer);//Convert LSBs to integer size
    return e_success;
}

/* Decode secret file extenstion */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    // Allocate memory for extension
    decInfo->extn_secret_file = malloc(decInfo->extn_file_size + 1);
    //Check allocation
    if (!decInfo->extn_secret_file)
    {
        printf("ERROR : Memory allocation failed\n");
        return e_failure;
    }
    // Decode the extension from image
    if (decode_image_to_data(decInfo->extn_secret_file, decInfo->extn_file_size, decInfo->fptr_stego_image) != e_success)
    {
        free(decInfo->extn_secret_file);//Free memory on failure
        return e_failure;
    }
    decInfo->extn_secret_file[decInfo->extn_file_size] = '\0';//Null-terminate extension

    //Set output file name with decoded extension
    char *default_name = "decoded_secret_file";//Default filename prefix

    char temp_fname[256];//Temporary buffer for full name
    sprintf(temp_fname, "%s%s", default_name, decInfo->extn_secret_file);// Concatenate default name with extension

// Free previous malloc if needed
if (decInfo->output_secret_fname)
{
    free(decInfo->output_secret_fname);
}

// Assign the new filename
decInfo->output_secret_fname = malloc(strlen(temp_fname) + 1);//Allocate memory for new filename

strcpy(decInfo->output_secret_fname, temp_fname);//Copy concatenated filename

free(decInfo->extn_secret_file);//Free extension memory

}



/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];//INitailize temporary buffer
    // Read 32 bytes from stego image
    if(fread(buffer, 1, 32, decInfo->fptr_stego_image) != 32)
    {
        printf("ERROR : Failed to read secret file size\n");
        return e_failure;
    }
    decode_lsb_to_size(&decInfo->size_secret_file, buffer);//Convert LSBs to integer size
    return e_success;
}

/* Decode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];//Initailize temporary buffer for 8 bytes

    printf("INFO    : Writing decoded data to output file...\n");
    //Loop for each byte of secret file
    for (long i = 0; i < decInfo->size_secret_file; i++)
    {
        //Read 8 bytes from stego image
        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            printf("ERROR : Failed to read encoded data\n");
            return e_failure;
        }

        char decoded;//Initialize temporary variable for decoded byte
        decode_lsb_to_byte(&decoded, buffer);//Decode 8 LSBs into 1 byte

        //Write decoded byte to output file
        if (fwrite(&decoded, 1, 1, decInfo->output_fptr_secret) != 1)
        {
            printf("ERROR : Failed to write decoded data\n");
            return e_failure;
        }
    }
    return e_success;
}

/* Decode function, which does the real decoding */
Status decode_image_to_data(char *data, int size, FILE *fptr_stego_image)
{
   char buffer[8];//Declare 8-byte buffer to read image data
   //Loop for decode each character
   for (int i = 0; i < size; i++)
    {
        //Read 8 bytes from the stego image file
        if (fread(buffer, 1, 8, fptr_stego_image) != 8)
        {
            return e_failure;
        }
        //Decode LSB bytes
        if(decode_lsb_to_byte(&data[i], buffer) != e_success)
        {
            return e_failure;
        }    
    }
    return e_success;
}

/* Decode a LSB into byte of image data array */
Status decode_lsb_to_byte(char *data, char *image_buffer)
{
    *data = 0;//Initialize data
    //Loop for 8 bits
    for (int i = 0; i < 8; i++)
    {
        *data = (*data << 1) | (image_buffer[i] & 0x01);//Extract LSB bit
    }
    return e_success;

}
/* Decode a LSB into size of image data array */
Status decode_lsb_to_size(long *data, char *image_buffer)
{
    *data = 0;//Initialize size
    //Loop for 32 bits
    for (int i = 0; i < 32; i++)
    {
        *data = (*data << 1) | (image_buffer[i] & 0x01);//Extract LSB bit
    }
    return e_success;

}

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo)
{
    //Open required files
    if (open_decode_files(decInfo) != e_success)
    {
        printf("ERROR : Files not opened successfully");
        return e_failure;
    }
    //Skip BMP header
    if (skip_bmp_header(decInfo->fptr_stego_image) != e_success)
    {
        printf("ERROR : Failed to skip BMP header\n");
        return e_failure;
    }
    //Decode magic string
    if (decode_magic_string(decInfo) != e_success)
    {
        printf("ERROR : Magic string not found\n");
        return e_failure;
    }
    //Decode extension size
    if (decode_secret_file_extn_size(decInfo) != e_success)
    {
        printf("ERROR : Failed to decode secret file extension size\n");
        return e_failure;
    }
    
    printf("SUCCESS : Secret file extension size decoded\n");
    //Decode extension
    if (decode_secret_file_extn(decInfo) != e_success)
    {
        printf("ERROR : Failed to decode secret file extension\n");
        return e_failure;
    }
    
    printf("SUCCESS : Secret file extension decoded\n");
    //Decode file size
    if (decode_secret_file_size(decInfo) != e_success)
    {
        printf("ERROR : Failed to decode secret file size\n");
        return e_failure;
    }
    
    printf("SUCCESS : Secret file size decoded\n");
    //Decode secret file data
    if (decode_secret_file_data(decInfo) != e_success)
    {
        printf("ERROR : Failed to decode secret file data\n");
        return e_failure;
    }
    
    printf("SUCCESS : Secret file data decoded\n");

    printf("SUCCESS : File Decoded Successfully\n");

    fclose(decInfo->fptr_stego_image);//Close stego image
    fclose(decInfo->output_fptr_secret);// Close output file

    return e_success;
}



