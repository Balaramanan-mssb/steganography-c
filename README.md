# steganography-c
Steganography project in C used to hide and extract secret data from files
Steganography in C:

Problem Statement:
In many situations, secure communication is required without attracting attention. Encryption hides the content, but not the existence of data. This project implements steganography to conceal secret information inside a file, making the communication less detectable.

Method of Approach:
This project is developed using C programming and file handling concepts. The core idea is to embed secret data into a carrier file (such as an image or text file) by modifying its data at the byte/bit level.

Process Involved:
- Encoding: Secret message is hidden inside the file
- Decoding: Hidden message is extracted from the file
- Bit manipulation techniques are used to store and retrieve data

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
