

// int main(int argc, char const *argv[]){
//     try
//     {
//         // variables for connecting to other router
//         std::string address = argv[1];
//         int port_con = stoi(argv[2]);    

//         int port;    
//         boost::asio::io_service io_service;

//         cout << "Start Router! choose port: " << endl;
//         std::cin >> port;

//         //Start router
//         Router router(io_service, port, address, port_con);    
//         cout << "------------ Router Started ------------" << endl;
//         io_service.run();
        
//     }
//     catch(const std::exception& e)
//     {
//         std::cerr << e.what() << endl;
//     }

//     return 0;

// }