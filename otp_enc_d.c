/* File: otp_enc_d
 * Name: Xavier Hollingsworth
 * Date: 04/04/2019
 * Class: CS 344 Operating Systems 1
 * Description: This program runs in the background as a daemon. It perform the actual encrypting. It receives from
 * otp_enc plaintext and a key. It then writes back the ciphertext to the otp_enc process that is connected
 * via the same communication socket.
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

#define SIZE 128000


/* Functions */

// Converts a char to an integer
int charToInt(char character)
{
    // Initialize variables
    int n;
    // All possible characters including space
    static const char *chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    
    // Loop through possible characters and return the matching number
    for (n = 0; n < 27; n++)
    {
        if (character == chars[n])
        {
            return n;
        }
    }
}


// Converts an integer to the equivalent character
char intToChar(int number)
{
    // All possible characters including space
    static const char *chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    
    // Return char equivalent of provided integer
    return chars[number];
}


// Function to encrypt the characters
void encryption(char message[], char key[], int length)
{
    int n, text, keyNumber, encryptionNumber;
    
    // Loop through plaintext
    for (n = 0; n < (length - 1); n++)
    {
        // Convert plain text to equivalent integer
        text = charToInt(message[n]);
        // Convert key text to equivalent integer
        keyNumber = charToInt(key[n]);
        // Retrieve encryption number based on formula
        encryptionNumber = (text + keyNumber) % 27;
        // Encrypt the plaintext to a different char
        message[n] = intToChar(encryptionNumber);
    }
    // Removes new line character
    message[length-1] = '\0';
    return;
}



/* Main Function */
int main(int argc, char *argv[])
{
    // Initialize variables
    int listenSocketFD, establishedConnectionFD, port, charsRead, i, n;
    int value = 1;
    pid_t pid;
    char *key;
    char buffer[SIZE];
    char keyBuffer[SIZE];
    char secondBuffer[SIZE];
    char message[SIZE];
    socklen_t sizeOfClientInfo;
    struct sockaddr_in serverAddress, clientAddress;
    
    // Check number of arguments and display error if incorrect number provided
    if (argc < 2)
    {
        fprintf(stderr, "ERROR: incorrect number of arguments. \n");
        exit(1);
    }
    
    // Set up the address struct for the server
    
    // Clear the address struct
    memset((char *)&serverAddress, '\0', sizeof(serverAddress));
    // Get port number and convert it from an integer to a string
    port = atoi(argv[1]);
    // Create a network capable socket
    serverAddress.sin_family = AF_INET;
    // Store the port number
    serverAddress.sin_port = htons(port);
    // Allow any address to connect
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    
    // Set up the socket
    listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    // Check if socket was created
    if (listenSocketFD < 0)
    {
        fprintf(stderr, "ERROR: opening socket");
        exit(2);
    }
    
    setsockopt(listenSocketFD, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int));
    
    // Enable the socket to begin listening and check for listening error
    if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        fprintf(stderr, "ERROR: port: %d cannot be bounded \n", port);
        exit(2);
    }
    
    // Error check to see that there are 5 possible connections
    if (listen(listenSocketFD, 5) == -1)
    {
        fprintf(stderr, "ERROR: port: %d cannot listen \n", port);
        exit(2);
    }
    
    // Get the size of the address for the client that will connect
    sizeOfClientInfo = sizeof(clientAddress);
    
    // Call all connections until there are no more
    while (1)
    {
        // Accept a connection, blocking if one is not available until one connects
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
        // Check to make sure the connection was accepted
        if (establishedConnectionFD < 0)
        {
            fprintf(stderr, "ERROR: on accept\n");
            exit(2);
        }
        
        
        
        // Start fork to connect files
        pid = fork();
        
        // Check to make sure fork was succesful
        if (pid < 0)
        {
            printf("ERROR: Fork failed \n");
            exit(1);
        }
        
        // Child process created
        if (pid == 0)
        {
            int keySize = 0;
            int textSize = 0;
            
            // Clear the buffer
            memset(buffer, '\0', SIZE);
            
            // Receive message from client and display error if message can't be read from socket
            int charsRead = recv(establishedConnectionFD, buffer, 1023, 0);
            if (charsRead < 0)
            {
                fprintf(stderr,  "ERROR: port: %d could not send text\n", port);
                exit(1);
            }

            
            // Check to make sure that the message received from client came from otp_enc
            if (strcmp(buffer, "enc") != 0)
            {
                
                exit(2);
            }
            // If correct message received from otp_enc send it back to client for authorization
            else
            {
                charsRead = send(establishedConnectionFD, buffer, 3, 0);
                // Error check to make sure message was sent to otp_enc
                if (charsRead < 0)
                {
                    fprintf(stderr,  "ERROR: port: %d could not send text\n", port);
                    exit(1);
                }
            }
            
            // Clear buffer
            memset(buffer, '\0', sizeof(buffer));
            
            // Receive plaintext from client
            textSize = recv(establishedConnectionFD, buffer, sizeof(buffer) - 1, 0);
            if (textSize < 0)
            {
                fprintf(stderr,  "ERROR: port: %d could not send text\n", port);
                exit(1);
            }
            // Check for bad characters
            for (i = 0; i < textSize - 1; i++)
            {
                if (isspace(buffer[i]) || isalpha(buffer[i]))
                {
                }
                else
                {
                    fprintf(stderr, "Plaintext contains bad characters\n");
                }
            }
            
            // Send message to client to control the flow of messages
            char message2[] = "okay to send";
            send(establishedConnectionFD, message2, strlen(message2), 0);
            
            
            // Clear key buffer
            memset(keyBuffer, '\0', sizeof(keyBuffer));
            // Receive key from client
            keySize = recv(establishedConnectionFD, keyBuffer, sizeof(keyBuffer) - 1, 0);
            if (keySize < 0)
            {
                fprintf(stderr,  "ERROR: port: %d could not send text\n", port);
                exit(1);
            }
            
            
            // Encrypt the plain text
            encryption(buffer, keyBuffer, strlen(buffer));
            
            // Send encrypted text back to client
            int sentInfo = send(establishedConnectionFD, buffer, strlen(buffer), 0);
            if (sentInfo < 0)
            {
                fprintf(stderr,  "ERROR: port: %d could not send text\n", port);
                exit(1);
            }
            
            // Close files and sockets
            close(establishedConnectionFD);
            close(listenSocketFD);
            return 0;
        }
        // Close newest connection
        else close(establishedConnectionFD);
    }
    
    return 0;
}

