EXE = router
IMGUI_DIR = ./Imgui
SOURCES = main.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SOURCES += Node/helper.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
UNAME_S := $(shell uname -s)
LINUX_GL_LIBS = -lGL

CXXFLAGS = -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS += -std=c++17 -g -Wall -Wformat 
CXXFLAGS += -pthread -DBOOST_ERROR_CODE_HEADER_ONLY -lcryptopp
LIBS =

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS += $(LINUX_GL_LIBS) `pkg-config --static --libs glfw3`

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
	LIBS += -L/usr/local/lib -L/opt/local/lib -L/opt/homebrew/lib
	#LIBS += -lglfw3
	LIBS += -lglfw

	CXXFLAGS += -I/usr/local/include -I/opt/local/include -I/opt/homebrew/include
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
	LIBS += -lglfw3 -lgdi32 -lopengl32 -limm32

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:Node/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)


# all: 
# 		g++ -Wall -g -std=c++17 -c Node/helper.cpp Node/Server.cpp Prosumer.cpp router.cpp -pthread -DBOOST_ERROR_CODE_HEADER_ONLY -lcryptopp
# 		g++ -Wall -g -std=c++17 -o prosumer Prosumer.o helper.o Server.o -pthread -DBOOST_ERROR_CODE_HEADER_ONLY -lcryptopp
# 		g++ -Wall -g -std=c++17 -o router router.o helper.o -pthread -DBOOST_ERROR_CODE_HEADER_ONLY -lcryptopp
build:
	cd boost_1_66_0
	./bootstrap.sh --prefix=/usr/local/
	./b2 install 
docker:
	g++ -Wall -g -std=c++17 -o router router.cpp Node/helper.cpp -pthread -DBOOST_ERROR_CODE_HEADER_ONLY -lcryptopp

	
prosumer:
	g++ -Wall -g -std=c++17 -o prosumer Prosumer.cpp Node/helper.cpp Node/Server.cpp -pthread -DBOOST_ERROR_CODE_HEADER_ONLY -lcryptopp
# router:
# 		g++ -Wall -g -std=c++17 -o router router.cpp -pthread -DBOOST_ERROR_CODE_HEADER_ONLY
# clean:
# 		rm *.o prosumer router


