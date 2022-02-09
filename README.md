# Router project for 21e8

- running "make" builds the router and gui for router
- then running "make prosumer" builds the prosumer that interacts with the router
- prosumer requires 2 folders in directory called "index" and "data" for the node (However node isn't fully realized in this yet")

# Dependencies
- nlohmann/json https://github.com/nlohmann/json
- imgui https://github.com/ocornut/imgui + GLFW
- crypto++ https://cryptopp.com/ ( sudo apt-get install libcrypto++-dev libcrypto++-doc libcrypto++-utils )
- TaskFlow https://github.com/taskflow/taskflow ( already included in repo )
