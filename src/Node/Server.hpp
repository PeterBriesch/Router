#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <functional> 
#include <cctype>
#include <locale>
#include <filesystem>
#include "../Miner/mine.hpp"
#include "include/httplib.h"
#include <stack>

void StartServer(int a);
HashTable loadHashTable(void);
DataTable loadDataTable(void);