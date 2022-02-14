#pragma once

#include <iostream>
#include <fstream>
#include <cstring>
#include "include/json.hpp"
#include "include/taskflow/taskflow.hpp"
#include <algorithm>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#include <cryptopp/files.h>

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

std::string hashFunction(string i);
vector<string> splitString(string s);
HashBlock split(nlohmann::json j);
int to_int(string s);
json to_json(HashBlock &h);
void writeToJson(HashBlock h, string dir);
void writeToTxt(string source, string data, string dir);
json loadJson(string source);
bool isMatch(string rotation, string target);
HashBlock match(string source, string data, string target, string user, int nonce);
HashBlock rotation(string source);

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

