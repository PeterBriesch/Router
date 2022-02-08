#include "sha2.hpp"
#include <cryptopp/base64.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

using std::cout;
using std::endl;
using namespace CryptoPP;

int main(int argc, char const *argv[]){
    
    
    unsigned long long inlen;

    inlen = 64* 8;

    int i, j;
    unsigned char message[] = "hello";

    unsigned char *in = new unsigned char[inlen];
    unsigned char *out = new unsigned char[inlen];    
    memcpy(in, message, sizeof(message));



    sha256(out, in);

    cout << in << endl;
    cout << out << endl;


    free(in);
    free(out);
     
    return 0;

}