all: 
		g++ -Wall -g -std=c++17 -c Prosumer.cpp Node/helper.cpp Node/Server.cpp router.cpp -pthread -DBOOST_ERROR_CODE_HEADER_ONLY -lcryptopp
		g++ -Wall -g -std=c++17 -o prosumer Prosumer.o helper.o Server.o -pthread -DBOOST_ERROR_CODE_HEADER_ONLY -lcryptopp
		g++ -Wall -g -std=c++17 -o router router.o helper.o -pthread -DBOOST_ERROR_CODE_HEADER_ONLY -lcryptopp
	
prosumer:
		g++ -Wall -g -std=c++17 -o prosumer Prosumer.cpp ../helper.cpp -pthread -DBOOST_ERROR_CODE_HEADER_ONLY -lcryptopp
router:
		g++ -Wall -g -std=c++17 -o router router.cpp -pthread -DBOOST_ERROR_CODE_HEADER_ONLY
clean:
		rm *.o prosumer router


