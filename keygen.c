/*
 * Name: Xavier Hollingsworth
 * File Name: keygen.c
 * Description: Generates a random key
 * Date: 03/20/19
 * */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


int main(int argc, char *argv[])
{
  // Initialize variables
  int character;
  int n, length;

  // Generate random seed
  srand(time(NULL));

  if (argc < 2)
  {
    fprintf(stderr, "Error, need keylength to run program.");
    exit(1);
  }

  // Set length equal to second argument provided
  length = atoi(argv[1]);

  // Pick random capital letters until length is reached
  for (n = 0; n < length; n++)
  {
    character = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[rand()%27];

    if (character == '@')
    {
      character = ' ';
    }

    printf("%c", character);
  }

  printf("\n");
  return 0;
}
