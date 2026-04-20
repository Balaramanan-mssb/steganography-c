/*
    NAME               : BALARAMANAN M
    BATCH              : 25031C
    BATCH ID           : 25031_157
    DATE OF SUBMISSION : 15/01/2026

    DESCRIPTION        :
    This project implements image steganography, a technique for hiding
    secret information within digital images in a way that is invisible
    to the human eye. The project securely embeds a secret text file into
    a cover image using the BMP file format and later retrieves the data
    without altering the visible appearance of the image.

    Two processes are involved in this project:

    --> Encoding :
       The secret data is first validated and then hidden inside the
       least significant bits (LSBs) of the cover image pixels. The BMP
       image format is used because it allows precise manipulation of
       pixel data. During encoding, the program validates input files,
       opens them in appropriate modes, checks image capacity, copies
       the BMP header unchanged, and encodes a magic string followed by
       the secret file extension size, extension, file size, and file
       contents into the image pixels.

    --> Decoding :
       During decoding, the program reads the stego image, verifies the
       magic string, extracts the hidden file information, and
       reconstructs the original secret file.

    This project ensures secure and accurate data hiding and retrieval
    using simple bit manipulation techniques.
*/
#include <stdio.h>
#include <string.h>
#include "encode.h"
#include"decode.h"
#include "types.h"

OperationType check_operation_type(char* argv[])
{
    //Check the argv[1] is "-e" or not
    if(strcmp(argv[1], "-e") == 0)
    {
        return e_encode;
    }
    //Check the argv[1] is "-d" or not
    else if(strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }
    //check for unsupported operation
    else
    {
        return e_unsupported;
    }
}

int main(int argc,char* argv[])
{
    EncodeInfo encInfo = {0};
    DecodeInfo decInfo = {0};
    OperationType res;

    res = check_operation_type(argv);

    //Check operation type
    /* ENCODE */
    if(res == e_encode)
    {

        printf("Selected Operation : Encoding\n");

        //Validate encoding arguments
        if(read_and_validate_encode_args(argc,argv, &encInfo) != e_success)
        {
            printf("ERROR : Invalid encoding arguments\n");
            return 1;
        }

        // To perform encoding
        if (do_encoding(&encInfo) != e_success)
        {
            printf("ERROR : Encoding failed\n");
            return 1;
        }

        printf("SUCCESS : Encoding completed successfully\n");
    }
    /* DECODE */
    else if (res == e_decode)
    {
        printf("Selected Operation : Decoding\n");

        if (read_and_validate_decode_args(argc, argv, &decInfo) != e_success)
        {
            printf("ERROR : Invalid decoding arguments\n");
            return 1;
        }

        if (do_decoding(&decInfo) != e_success)
        {
            printf("ERROR : Decoding failed\n");
            return 1;
        }

        printf("SUCCESS : Decoding completed successfully\n");
    }

    /* UNSUPPORTED */
    else
    {
        printf("ERROR : Unsupported operation\n");
        return 1;
    }
    return 0;
}
