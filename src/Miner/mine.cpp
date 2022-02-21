#include "mine.hpp"

using namespace std;
using json = nlohmann::json;

/* HELPER FUNCTIONS */


//Helper function for hashing any string and returning the resultant
//hash as a Hex encoded string 
string hashFunction(string i){
    using namespace CryptoPP;
    SHA256 hash;
    string out;

    StringSource foo(i, true, 
        new HashFilter(hash,new HexEncoder(new StringSink(out),false))
    );

    return out;
}


vector<string> splitString(string s){
    //string dump for spliting
    string delim = "\"";
    size_t pos = 0;
    size_t pos2 = 0;
    vector<string> tokens;
    pos = s.find(delim);

    while((pos2 = s.find(delim, pos+1)) != string::npos){
        tokens.push_back(s.substr(pos+1, pos2-(pos+delim.length())));
        s.erase(pos-1, pos2+delim.length());
        pos = s.find(delim);
    }

    return tokens;
}

//Extracts data in json encapsulated by quotations
HashBlock split(nlohmann::json j){
    //string dump for splitting
    string s = j.dump();
    string delim = "\"";
    size_t pos = 0;
    size_t pos2 = 0;
    vector<string> tokens;
    pos = s.find(delim);

    while((pos2 = s.find(delim, pos+1)) != string::npos){
        tokens.push_back(s.substr(pos+1, pos2-(pos+delim.length())));
        s.erase(pos-1, pos2+delim.length());
        pos = s.find(delim);
    }


    HashBlock data = {tokens[2], to_int(tokens[4]), tokens[6], tokens[8], tokens[12], tokens[10]};

    return data;
}

int to_int(string s){
    stringstream sstream(s);
    int i;

    sstream >> i;

    return i;
    
}

json to_json(HashBlock &h){

    json j = {
                {h.rotation, {
                    {"datahash", h.dataHash},
                    {"n", to_string(h.nonce)},
                    {"rotation", h.rotation},
                    {"source", h.source},
                    {"user", h.usr},
                    {"target", h.target}
                }}   
            };


    return j;

}


void writeToJson(HashBlock h, string dir){
    json j;
    string srcHash;

   
    srcHash = h.source;
    

    j = to_json(h);

    ofstream o(dir+srcHash+".json");
    o << setw(4) << j << endl;
    o.close();

    cout << "Writen to json" << endl;
}

void writeToTxt(string dataHash , string data, string dir){
    ofstream o(dir+dataHash+".txt");
    o << data;
    o.close();
    cout << "Writen to txt" << endl;
}

json loadJson(string source){
    
    ifstream i(source);
    json j;
    i >> j;
    i.close();
    return j;

}


bool isMatch(string rotation, string target){
    if(rotation.substr(0,target.length()) == target){
        return true;
    }
    else{
        return false;
    }
}


//Basic mine function mines data into hashtable
//returns final rotation
//Rotation: pass previouse source as source
//Fork: pass previouse nonce to perform fork
//note source = sourceHash + dataHash
// tf::Task make_cudaflow(tf::Taskflow& taskflow);

HashBlock match(string source, string data, string target, string user, int nonce){

    tf::Executor executor;
    tf::Taskflow taskflow;

    int iterations = 16^7;

    int first, last;
    int batch = 1;

    //Set params for batch mining
    auto setparam = taskflow.emplace([&](){
        cout << "batch: " << batch << endl;
        first = (iterations)*(batch-1);
        last = (iterations*batch);
        batch++;
    });

    string hash = source + data + target + user;
    string rotation;
    
    tf::Task task = taskflow.for_each_index(std::ref(first), std::ref(last), 1, [hash, target, &rotation, &nonce](int m){
        string newhash = hashFunction(hash + to_string(m));
        if(isMatch(newhash, target)){
            printf("Match found with nonce [%d], hash [%s]\n", m, newhash.c_str());
            rotation = newhash;
            nonce = m;
            
        }
    });

    setparam.precede(task);
    while(1){
        executor.run(taskflow).wait();
        if(isMatch(rotation, target)){
            HashBlock hashBlock = {data, nonce, rotation, source, user, target};

            return hashBlock;
        }
        
    }

}

HashBlock rotation(string source){

    //Read json file
    string dir = "index/";
    ifstream i(dir+source+".json");
    cout << "found file" << endl;
    json j;
    i >> j;
    //extract data from json
    HashBlock data = split(j);
    

    HashBlock hashBlock = match(data.rotation, data.dataHash, data.target, data.usr, 1);

    return hashBlock;

}
