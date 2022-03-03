#include "imgui_assets.hpp"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}


int main(int argc, char const * argv[]){

    //Start router
    int port = 8080;    
    int port_con = 8080;
    std::string address = "10.147.20.7";
    if(argc > 1){
        port = stoi(argv[1]);
        address = argv[2];
        port_con = stoi(argv[3]);
    }
    boost::asio::io_service io_service;
    boost::asio::io_service router_ioservice;


    //Start router
    printf("Starting Router connecting on Port %d, connecting on address %s and port %d \n", port, address.c_str(), port_con);
    Router router(io_service, port, address, port_con, router_ioservice);  

    std::thread serv([&]()
        {
            io_service.run();
        });
    std::thread serv2([&]()
        {
            router_ioservice.run();
        });

    /* USER VARIABLES */
    std::vector<std::string> output_bufs(6*12); // output storage for cell functions

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    bool show_my_window = true;
    bool show_spreadsheet = false;
    bool show_analyser = true;
    bool show_data = false;
    bool show_routingTable = false;


    int packet_count[] = {0, 0, 0}; //must be changed to an array of int for scalability
    std::vector<net::Packet::packet> cache;
    net::Packet::packet packet_temp;
    uint64_t payload = 0;


    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WzantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }


         /*
            Window for displaying the routing table and client table
        */  
        if(show_routingTable){

            show_table(router.getRoutingTable(), show_routingTable);

        }


        if(show_my_window)
        {

            static bool no_titlebar = false;
            static bool no_scrollbar = false;
            static bool no_menu = false;
            static bool no_move = false;
            static bool no_resize = false;
            static bool no_collapse = false;
            static bool no_nav = false;
            static bool no_background = false;
            static bool no_bring_to_front = false;
            static bool no_docking = true;
            static bool unsaved_document = false;

            ImGuiWindowFlags window_flags = 0;
            if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
            if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
            if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
            if (no_move)            window_flags |= ImGuiWindowFlags_NoMove;
            if (no_resize)          window_flags |= ImGuiWindowFlags_NoResize;
            if (no_collapse)        window_flags |= ImGuiWindowFlags_NoCollapse;
            if (no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
            if (no_background)      window_flags |= ImGuiWindowFlags_NoBackground;
            if (no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
            if (no_docking)         window_flags |= ImGuiWindowFlags_NoDocking;
            if (unsaved_document)   window_flags |= ImGuiWindowFlags_UnsavedDocument;

            bool AutoScroll = true;

            ImGui::Begin("My Window", &show_my_window, window_flags);

            // e.g. Leave a fixed amount of width for labels (by passing a negative value), the rest goes to widgets.
            ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

            auto window_size = ImGui::GetWindowSize();

            /*
                Menu Bar for accessing different tools
            */
            if(ImGui::BeginMenuBar())
            {
                if(ImGui::BeginMenu("Menu")){
                    if(ImGui::MenuItem("Spread sheet")){
                        show_spreadsheet = true;
                        show_analyser = false;
                    }
                    if(ImGui::MenuItem("Packet Analyser")){
                        show_spreadsheet = false;
                        show_analyser = true;
                    }
                    if(ImGui::MenuItem("Routing table")){
                        show_routingTable = true;

                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            /*
                Spreadsheet tool for network programming and packet filtering
            */
            if(show_spreadsheet)
            {        
                // Window Setup
                const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
                static ImGuiTableFlags flags2 = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
                static ImVec2 cell_padding(0.0f, 0.0f);
                static ImVec2 outer_size_value = ImVec2(0.0f, TEXT_BASE_HEIGHT * 30);

                // Get packet from router
                static net::Packet::packet pkt;
                bool update = router.ShowPacket(pkt);

                // Start table
                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cell_padding);  
                if (ImGui::BeginTable("table_padding_2", 12, flags2, outer_size_value))
                {
                    static char text_bufs[6 * 12][128]; // text storage for cells
                    
                    // Traverse spreadsheet
                    for (int cell = 0; cell < 6 * 12; cell++)
                    {
                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::PushID(cell);
                        
                        //use regex to find function in text buffer
                        std::regex e ("Count\\([a-fA-F0-9]*\\)");
                        if(regex_match(text_bufs[cell], e)){

                            string match (text_bufs[cell]);

                            char temp[INET6_ADDRSTRLEN];
                            inet_ntop(AF_INET6, pkt.header.daddr, temp, INET6_ADDRSTRLEN);
                            string dstaddress(temp);

                            CountPacket(match, dstaddress, update, output_bufs[cell]); //function used to count the specific packets (result stored in output_bufs)

                            ImGui::Text(output_bufs[cell].c_str());

                        }
                        else{
                            ImGui::InputText("##cell", text_bufs[cell], IM_ARRAYSIZE(text_bufs[cell]));
                        }

                        
                        ImGui::PopID();
                        
                    }
                    
                    ImGui::EndTable();
                }
                ImGui::PopStyleVar();
            }

            /*
                Analyiser for viewing the stream of packets 
                coming through the router
            */
            if(show_analyser)
            {


                ImGui::Text("Console for viewing all connected clients");
                static int port = 8081;
                static int node_port = 2180;
                static int newport = 8080;
                static std::string address = "10.147.20.40";
                if(ImGui::Button("Start Prosumer")){
                    char command[1024];
                    sprintf(command, "gnome-terminal -e 'sh -c \"./prosumer %d %d %s %d\"'", node_port, port, address.c_str(), newport);
                    system(command);
                    port++;
                    node_port++;
                }
                int index = 0;
                bool update = router.ShowCache(cache);
                const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
                static ImVec2 outer_size_value = ImVec2(0.0f, TEXT_BASE_HEIGHT * 12);
                static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
                static ImVector<int> selection;
                static ImVector<net::Packet::packet> packets;
                static ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;

                /*
                Table of packets as they stream through the router
                */
                if (ImGui::BeginTable("Packets", 6, flags, outer_size_value))
                {
                   

                    //Set up columns
                    ImGui::TableSetupColumn("No.", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Time Stamp", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Length", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Destination", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Protocol", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();
                    

                    //Loop through every item in the cache and print it on imgui
                    std::for_each(cache.rbegin(), cache.rend(), [&](net::Packet::packet p){

                        Packet packet(p);
                        string dstaddress = packet.get_dstAddress();
                        srand(stoi(dstaddress, nullptr, 16));
                        float hsv_col = static_cast<float>(rand())/static_cast<float>(RAND_MAX);

                        auto txt_color = ImVec4(1,1,1,1);
                        
                        const bool item_is_selected = selection.contains(index);
                        ImGui::PushID(index);
                        ImGui::PushStyleColor(ImGuiCol_TableRowBg, (ImVec4)ImColor::HSV(hsv_col, 0.6f, 0.6f));
                        ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, (ImVec4)ImColor::HSV(hsv_col, 0.6f, 0.6f));
                        ImGui::TableNextRow(ImGuiTableRowFlags_None, 0.0f);
                        
                        ImGui::TableNextColumn();
                        //Setup selectable rows
                        char label[32];
                        sprintf(label, "%04d", index);
                        if(ImGui::Selectable(label, item_is_selected, selectable_flags, ImVec2(0, 0.0f))){
                            if (ImGui::GetIO().KeyCtrl)
                            {
                                if(item_is_selected){

                                    payload = p.payload.payload;
                                    show_data = true;
                                    selection.find_erase_unsorted(index);
                                }
                                else
                                    selection.push_back(index);
                            }
                            else
                            {
                                payload = p.payload.payload;
                                show_data = true;
                                selection.clear();
                                selection.push_back(index);
                            }
                        }

                        ImGui::TableNextColumn();
                        ImGui::TextColored(txt_color, "%lld", (long long)p.header.timestamp);
                        ImGui::TableNextColumn();
                        ImGui::TextColored(txt_color, "%d", p.header.length);
                        ImGui::TableNextColumn();
                        ImGui::TextColored(txt_color,"10.147.20.40:%d", p.header.saddr);
                        ImGui::TableNextColumn();
                        ImGui::TextColored(txt_color, "%s", packet.get_dstAddress().c_str());
                        ImGui::TableNextColumn();
                        ImGui::TextColored(txt_color, "TCP");
                        index++;

                        ImGui::PopStyleColor(2);
                        ImGui::PopID();
                    });

                    if(AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()){
                        ImGui::SetScrollHereY(1.0f);
                    }
                
                    ImGui::EndTable();                    
                    
                }

                //Parse uint64_t payload as string
                string payload_out = to_hash<uint64_t>(payload);
                
                /*
                Window for displaying payload                
                */
                ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 100));
                ImGui::BeginChild("payload", ImVec2(0.0f, ImGui::GetContentRegionAvail().y * 0.5f), true, ImGuiWindowFlags_None);
                ImGui::TextWrapped("%s", payload_out.c_str());
                ImGui::EndChild();
                ImGui::PopStyleColor();                  

                //scrolling window showing connected clients
                std::map<string, boost::shared_ptr<cli_handler>> clients = router.ShowClients();
        
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
                ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, TEXT_BASE_HEIGHT * 5), false, window_flags);
                
                for(auto const& [key, val] : clients){
                    ImGui::Text("%s: Online", key.c_str());
                }
                ImGui::EndChild();

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS), Window Size (%f, %f)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate, window_size.x, window_size.y);
                
            }

            ImGui::PopItemWidth();
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glfwSetWindowSize(window, 1920, 1080);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    	
        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();


    return 0;
}