#include "router.hpp"
#include <inttypes.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <sstream>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}




int main(int argc, char const * argv[]){

    //Start router
    int port = 8080;    
    int port_con = 8080;
    std::string address = "0.0.0.0";
    boost::asio::io_service io_service;

    // cout << "Start Router! choose port: " << endl;
    // std::cin >> port;

    //Start router
    Router router(io_service, port, address, port_con);  

    std::thread serv([&](){io_service.run();});
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
    bool show_data = false;
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

        if(show_my_window)
        {
            bool AutoScroll = true;
            // if(ImGui::BeginMenuBar()){
            //     if(ImGui::BeginMenu("Menu")){

            //     }

            // }
            ImGui::Begin("My Window", &show_my_window);
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

            //Table showing traffic
            std::vector<net::Packet::packet> cache = router.ShowCache();
            const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
            int index = 0;
            static ImVec2 outer_size_value = ImVec2(0.0f, TEXT_BASE_HEIGHT * 12);
            static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
            static ImVector<int> selection;
            static ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;

            if (ImGui::BeginTable("Packets", 5, flags, outer_size_value))
            {

                //Set up columns
                ImGui::TableSetupColumn("No.", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Length", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Destination", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Protocol", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();
                

                //Loop through every item in the cache and print it on imgui
                ImGui::PushButtonRepeat(true);
                std::for_each(cache.rbegin(), cache.rend(), [&](net::Packet::packet p){

                    const bool item_is_selected = selection.contains(index);
                    ImGui::PushID(index);
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


                    // convert uint64_t destination to string


                    ImGui::TableNextColumn();
                    ImGui::Text("%d", p.header.length);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", p.header.saddr);
                    ImGui::TableNextColumn();
                    stringstream os;
                    os << std::hex << p.header.daddr;
                    string dest = os.str();
                    ImGui::Text(dest.c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text("TCP");
                    index++;

                    ImGui::PopID();
                });

                if(AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()){
                    ImGui::SetScrollHereY(1.0f);
                }
            
                ImGui::PopButtonRepeat();
                ImGui::EndTable();
                
            }

            //Parse uint64_t payload as string
            char payload_out[1024];
            sprintf(payload_out, "%" PRIx64, payload);

            // sprintf(payload_out, "%" PRIx64, payload);

            // ImGui::TextWrapped(payload_out);

            ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 100));
            ImGui::BeginChild("Red", outer_size_value, true, ImGuiWindowFlags_None);
            ImGui::TextWrapped(payload_out);
            ImGui::EndChild();
            bool child_is_hovered = ImGui::IsItemHovered();
            ImVec2 child_rect_min = ImGui::GetItemRectMin();
            ImVec2 child_rect_max = ImGui::GetItemRectMax();
            ImGui::PopStyleColor();
            

            //scrolling window showing connected clients
            router.ShowClients();
            // ImVec2 ourter_size = imVec2(0.0f, ImGUi::GetTextLineHeightWithSpacing() * 8);
            // if(ImGui::BeginTable("packet_feed")){
            //     ImGui::TableSetupScrollFreeze(0, 1);//Makes top row always visible
            //     ImGui::TableSetupColumn("No.", 0);
            //     ImGui::TableSetupColumn("Time", 0);
            //     ImGui::TableSetupColumn("Source", 0);
            //     ImGui::TableSetupColumn("Destination", 0);
            //     ImGui::TableSetupColumn("Protocol", 0);
            //     ImGui::TableHeadersRow();

            //     ImGuiListClipper clipper;
            //     clipper.Begin(1000);
            //     while(clipper.Step()){
            //         for(int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++){
            //             ImGui::TableNextRow();
            //             for(int column = 0; column < 5; column)
            //         }
            //     }

            // }


            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
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