/* File: otp_enc.c
 * Author: Xavier Hollingsworth
 * Date: 04/01/2019
 * Description: This is the client encryption file. It connects to otp_enc_d and asks it to perform 
 * a one-time pad encryption. otp_enc then receives the ciphertext back from otp_enc_d and outputs it to stdout.
 * */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

#define SIZE 128000


/* Main Function */

int main(int argc, char* argv[]){
    // Initialize variables
    int port, status, value, socketFD, i, infoReceived;
    char buffer[SIZE];
    char keyBuffer[SIZE];
    char secondBuffer[SIZE];
    char authentication[] = "enc";
    struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;
    
    // Clear buffer strings
    memset(buffer, '\0', sizeof(buffer));
    memset(keyBuffer, '\0', sizeof(keyBuffer));
    memset(secondBuffer, '\0', sizeof(secondBuffer));
    
    // Clear the address struct
    memset((char*)&serverAddress, '\0', sizeof(serverAddress));
    
    // Make sure that there are four arguments otherwise an error will display
    if(argc < 4)
    {
        fprintf(stderr, "ERROR: Incorrect number of arguments.\n");
        exit(1);
    }
    
    // Set up socket file descriptor
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(socketFD < 0)
    {
        fprintf(stderr, "ERROR: Socket could not be opened");
        exit(2);
    }
    
    // Get hostname
    serverHostInfo = gethostbyname("localhost");
    // Check to make sure host retrieved
    if(serverHostInfo == NULL)
    {
        fprintf(stderr, "ERROR: Could not resolve host name \n");
        exit(2);
    }
    
    // Retrieve port number from last argument
    port = atoi(argv[3]);
    
    // Create network capable socket and store port number
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    
    // Preserve special arrangement of the bytes
    memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);
    
    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
    {
        fprintf(stderr, "ERROR: could not contact otp_enc_d on port %d \n", port);
        exit(2);
    }
    // Authenticate that we are connected otp_enc_d
    int charsWritten =  send(socketFD, authentication, strlen(authentication), 0);
    // Error check to make sure message authentication message was sent to server
    if (charsWritten < strlen(authentication))
    {
        fprintf(stderr,  "ERROR: port: %d could not send text\n", port);
        exit(1);
    }
    
    
    // Clear authentication buffer
    memset(authentication, '\0', sizeof(authentication));
    
    // Read message fron server
    int charsRead = recv(socketFD, authentication, sizeof(buffer) - 1, 0);
    // Error check to make sure message was received from server
    if (charsRead < 0)
    {
        fprintf(stderr, "ERROR: port: %d could not receive text\n", port);
        exit(1);
    }
    
    // Compare message received to make sure we are not connected otp_dec_d
    if(strcmp(authentication, "enc") != 0)
    {
        fprintf(stderr, "ERROR: otp_enc can not use otp_dec_d\n");
        exit(2);
    }
    
    // Retrieve the key and its size
    int key = open(argv[2], O_RDONLY);
    int keySize = read(key, keyBuffer, sizeof(keyBuffer) - 1);
    
    
    // Get the text and the size of the text
    int text = open(argv[1], O_RDONLY);
    int textSize = read(text, buffer, sizeof(buffer) - 1);
    
    
    // Check the text for invalid characters by looping through the array
    for (i = 0; i < textSize - 1; i++)
    {
        if (isspace(buffer[i]) || isalpha(buffer[i]))
        {
        }
        else
        {
            fprintf(stderr, "ERROR: Plaintext contains bad characters\n");
            exit(1);
        }
    }
    
    // Check to see if the text is larger than the key
    if(textSize > keySize)
    {
        fprintf(stderr, "ERROR: key '%s' is too short\n", argv[2]);
        exit(1);
    }
    
    
    // Send the plaintext to server
    textSize = send(socketFD, buffer, strlen(buffer), 0);
    if (textSize < 0)
    {
        fprintf(stderr,  "ERROR: port: %d could not send text\n", port);
        exit(1);
    }
    
    
    //Receive blank message from server
    memset(secondBuffer, '\0', sizeof(secondBuffer));
    infoReceived = recv(socketFD, secondBuffer, sizeof(secondBuffer), 0);
    if (infoReceived < 0)
    {
        fprintf(stderr,  "ERROR: port: %d could not receive text\n", port);
        exit(1);
    }


    
    // Send key to server
    keySize = send(socketFD, keyBuffer, strlen(keyBuffer), 0);
    if (keySize < 0)
    {
        fprintf(stderr,  "ERROR: port: %d could not send text\n", port);
        exit(1);
    }
    
    
    // Clear buffer
    memset(buffer, '\0', sizeof(buffer));
    
    // Receive ciphered text
    status = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
    if (status < 0)
    {
        fprintf(stderr, "ERROR: port: %d error reading from socket.\n", port);
        exit(1);
    }
    
    // Print ciphered text
    printf("%s\n", buffer);
    
    // Close socket file descriptor
    close(socketFD);
    
    return 0;
}
