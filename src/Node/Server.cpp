#include "Server.hpp"

// #include "TaskFlowMiner.cpp"

using namespace std;
namespace fs = std::filesystem;
using json = nlohmann::json;

#define SERVER_PORT 2181


/* Loads Hash table from json files stores in ./index/ */
HashTable loadHashTable(void){

    HashTable hashTable;
    string source;
    string dir = "index";
    string path;
    size_t pos = 0;
    size_t pos2 = 0;
    json j;
    HashBlock hashBlock;

    if(!(fs::is_empty(dir))){
        for (const auto & entry : fs::directory_iterator(dir)){
            string path = entry.path();
            //find source in file path
            pos = path.find("/");
            pos2 = path.find(".");
            source = path.substr(pos+1, pos2-(pos+1));

            //Load rotation from json file
            
            j = loadJson(path);
            for(auto& el : j.items()){
                hashTable[source].push_back(el.key());
            }
            
        }
    }
        
    return hashTable;
}

DataTable loadDataTable(void){
    DataTable dataTable;
    string dir = "data";
    ifstream inFile;
    string data;
    size_t pos = 0;
    size_t pos2 = 0;
    string source;


    if(!(fs::is_empty(dir))){
        for (const auto & entry : fs::directory_iterator(dir)){
            string path = entry.path();

            //find source in file path
            pos = path.find("/");
            pos2 = path.find(".");
            source = path.substr(pos+1, pos2-(pos+1));
            
            //open file
            inFile.open(path);
            //check if file opened successfully
            if (!inFile){
                cerr << "Unable to open file " + path << endl;
                continue;
            }

            inFile >> data;
            inFile.close();
            dataTable[source] = data;

        }
    }
    return dataTable;

}

void StartServer(int port){
    httplib::Server svr;

    //Hash table 
    HashTable hashTable = loadHashTable();
    DataTable dataTable = loadDataTable();

    // ----- Mine new block onto keyword 
    svr.Post("/api/v2/mine", [&hashTable, &dataTable](const httplib::Request &req, httplib::Response &res){
        auto source = req.get_param_value("source");
        auto data = req.get_param_value("data");
        
        auto target = req.get_param_value("target");
        auto user = hashFunction("abc");//hashFunction(req.get_param_value("user"));
        int nonce = 0;//to_int(req.get_param_value("nonce"));
        int iterations = 1;
        if(req.has_param("iterations")){
            iterations = to_int(req.get_param_value("iterations"));
        }
        auto mode = req.get_param_value("mode");
        int wrttxt = 1;
        ofstream o;
        nlohmann::ordered_json j;
        string dataHash =hashFunction(data);

        //check for source in hashtable
        if(hashTable.count(source)>0){
            //Means fork
            j = loadJson("index/"+source+".json");
            


        }
        else{
            //means rotation
            for(auto const& [key, value] : hashTable){
                vector<string> rotations = value;
                string tmp = key;
                int check = 0;
                
                while(!rotations.empty()){
                    string rotation = rotations.back();
                    rotations.pop_back();
                    //check if rotation == source
                    if(rotation == source){
                        //load json
                        json tempJson = loadJson("index/"+tmp+".json");
                        tempJson = tempJson[rotation];
                        dataHash = tempJson["datahash"];
                        check++;
                        break;
                    }
                }

                if(check == 1){
                    wrttxt = 0;
                }
            }
            //Means new index insertion

        }
        //open json
        o.open("index/"+source+".json");
        
        HashBlock hashBlock = match(source, dataHash, target, user, nonce);

        //save hashBlock to index
        hashTable[hashBlock.source].push_back(hashBlock.rotation);

        //persist hashtable        
        j.emplace(hashBlock.rotation,to_json(hashBlock)[hashBlock.rotation]);
        o << setw(4) << j << endl;
        o.close();
        if(wrttxt){
            dataTable[hashBlock.dataHash] = data;
            writeToTxt(hashBlock.dataHash, data, "data/");
        }
        iterations--;
        source = hashBlock.rotation;

        while(iterations>0){
            
            json newj;
            //open json
            o.open("index/"+source+".json");
            
            hashBlock = match(source, dataHash, target, user, nonce);

            //save hashBlock to index
            hashTable[hashBlock.source].push_back(hashBlock.rotation);

            //persist hashtable
            newj.emplace(hashBlock.rotation,to_json(hashBlock)[hashBlock.rotation]);
            o << setw(4) << newj << endl;
            o.close();
            source = hashBlock.rotation;
            iterations--;
        }

        res.set_header("Access-Control-Allow-Origin", "*");
        res.status = 202;
        res.set_content("Successful insert on keyword \nsource:"+ source + 
        ", \nrotation:"+hashBlock.rotation+", \nnonce:"+
        to_string(hashBlock.nonce), "text/plain");
        return;
    });
    
    // ----- Index look up
    svr.Get(R"(/api/v2/index/([a-zA-Z_0-9]+))", [&hashTable, &dataTable](const httplib::Request &req, httplib::Response &res){
        auto source = req.matches[1];
        string s = source;
        json output;
        ifstream in;
        string out;

        res.status = 200;
        in.open("index/"+s+".json");
        if(in){
            in.close();
            json j = loadJson("index/"+s+".json");
            vector<string> data = hashTable[s];
            for(auto it=data.cbegin(); it!=data.cend(); it++){
                output.emplace(*it, j[*it]);
                s = *it;
                while(hashTable.count(s)!= 0){
                    json j = loadJson("index/" + s +".json");
                    vector<string> rotData = hashTable[s];

                    auto it = rotData.begin();
                    output.emplace(*it, j[*it]);
                    
                    s = *it;
                }
            }
            out = output.dump();
            res.set_content(out, "application/json");
        }

        res.set_header("Access-Control-Allow-Origin", "*");

    });

    // ----- Data look up
    svr.Get(R"(/api/v2/data/([a-zA-Z_0-9]+))" , [&dataTable](const httplib::Request &req, httplib::Response &res){
        auto source = req.matches[1];
        string out;

        out = dataTable[source];
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content(out, "application/octet-stream");

    });


    cout << "\nNode on port "+ to_string(port)+ "\n" << endl;

    svr.listen("0.0.0.0", port); 
}


/*  C++ server/miner communicates with and maintains hashtable
    for python client for inter process communication
    Python server runs as HTTP server for different nodes to communicate with
    and make requests to c++ server/miner  */
// int main()  {
    
//     StartServer();
    

//     return 0;
// }