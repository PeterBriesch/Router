#pragma once
#include "include/taskflow/taskflow.hpp"
#include <algorithm>
#include "include/json.hpp"
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#include <cryptopp/files.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include "include/json.hpp"

using namespace std;
using json = nlohmann::json;

typedef map<std::string, vector<string>> HashTable;
typedef map<std::string, std::string> DataTable;

struct HashBlock{

    std::string dataHash;
    int nonce;
    std::string rotation;
    std::string source;
    std::string usr;
    std::string target;
    HashBlock() = default;
};


vector<string> splitString(string s);
HashBlock split(nlohmann::json j);
int to_int(string s);
json to_json(HashBlock &h);
void writeToJson(HashBlock h, string dir);
void writeToTxt(string source, string data, string dir);
json loadJson(string source);
bool isMatch(string rotation, string target);

template <class T>
T to_int(string& hash){
    
    T i = std::stoul(hash, nullptr, 16);
    
    return i;
}

template <class T>
string to_hash(T &i){

    std::stringstream stream;
    stream << std::hex << i;
    string hash(stream.str());

    return hash;

}



std::string hashFunction(string i);
HashBlock match(string source, string data, string target, string user, int nonce);
HashBlock rotation(string source);