# steganography-c
Steganography project in C used to hide and extract secret data from files
Steganography in C:

Problem Statement:

In many situations, secure communication is required without attracting attention. Encryption hides the content, but not the existence of data. This project implements steganography to conceal secret information inside a file, making the communication less detectable.

Method of Approach:

This project is developed using C programming and file handling concepts. The core idea is to embed secret data into a carrier file (such as an image or text file) by modifying its data at the byte/bit level.

Process Involved:

Encode:

   The secret data is first validated and then hidden inside the
least significant bits (LSBs) of the cover image pixels. The BMP
image format is used because it allows precise manipulation of
pixel data. During encoding, the program validates input files,
opens them in appropriate modes, checks image capacity, copies
the BMP header unchanged, and encodes a magic string followed by
the secret file extension size, extension, file size, and file
contents into the image pixels.

Decode:

   During decoding, the program reads the stego image, verifies the
magic string, extracts the hidden file information, and
reconstructs the original secret file.
       
- Bit manipulation techniques are used to store and retrieve data.
  
- This project ensures secure and accurate data hiding and retrieval using simple bit manipulation techniques.
  

Features:

- Encode secret message into a carrier file  
- Decode hidden message from the carrier file  
- Supports basic file formats  
- Efficient use of file handling  
- Command-line based interface  

Technologies Used
- C Programming  
- File Handling concepts 
- Bit Manipulation  

How to Run:

1. Compile the program
   
gcc main.c encode.c decode.c

2. Run Encoding
   
./a.out -e beautiful.bmp output.bmp "secret.txt"

3. Run Decoding
   
./a.out -d output.bmp

Project Structure
main.c → Main control logic
encode.c → Handles encoding process
decode.c → Handles decoding process
header.h → Function declarations and structures
