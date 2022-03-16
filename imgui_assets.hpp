#include "21e8Router.hpp"
#include <inttypes.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <regex>
#include <stdio.h>
#include <sstream>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

/*Variables for Spread sheet*/
#define S_ROW 12
#define S_COL 6
std::vector<std::string> output_bufs = {"0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0","0", "0",}; // output storage for cell functions



std::string CountPacket(string match, string compare, bool update, std::string &cell)
{
    int counter = stoi(cell);
    auto start = match.find("(");
    auto end = match.find(")");
    string comparitor = match.substr(start,end);

    if((compare.substr(0,4)).compare(comparitor)&& update)
    {
        counter++;
    }

    return to_string(counter);
}


void ShowSheet(Router &router, bool &show_spreadsheet){
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
    bool update = true;

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

    ImGui::Begin("spreadsheet", &show_spreadsheet, window_flags);


    /*
        Spreadsheet tool for network programming and packet filtering
    */
    if(show_spreadsheet)
    {        
        // Window Setup
        const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
        static ImGuiTableFlags flags2 = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;
        static ImVec2 cell_padding(0.0f, 0.0f);
        static ImVec2 outer_size_value = ImVec2(0.0f, TEXT_BASE_HEIGHT * 30);

        // Get packet from router
        static net::Packet::packet pkt;
        update = router.ShowPacket(pkt);
        // Radio button to show when packet is updated
        ImGui::RadioButton("update", &update);

        //Start Table
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cell_padding);  
        if (ImGui::BeginTable("table_padding_2", S_ROW, flags2))
        {

            //Set up columns
            ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, IM_COL32(0,0,0,255));
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(151,255,34,255));
            ImGui::TableSetupScrollFreeze(1, 1);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoHide);
            
            //Setup headers
            for (int i = 0; i < 11; i++)
            {
                char header = i+0x41;
                ImGui::TableSetupColumn(&header);
            }

            ImGui::TableHeadersRow();
            ImGui::PopStyleColor(2);

            static char text_bufs[S_COL * S_ROW][128]; // text storage for cells
            
            // Traverse spreadsheet
            for (int cell = 0; cell < S_COL * S_ROW; cell++)
            {
                ImGui::PushStyleColor(ImGuiCol_TableRowBg, IM_COL32(0,0,0,255));
                ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, IM_COL32(0,0,0,255));

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::PushID(cell);
                
                //use regex to find function in text buffer
                std::regex e ("Count\\([a-fA-F0-9]*\\)");
                if(cell%S_ROW == 0){

                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(151,255,34,255));
                    ImGui::Text("%d", cell/S_ROW);
                    ImGui::PopStyleColor(1);
                }
                else if(regex_match(text_bufs[cell], e)){

                    string match (text_bufs[cell]);

                    char temp[INET6_ADDRSTRLEN];
                    inet_ntop(AF_INET6, pkt.header.daddr, temp, INET6_ADDRSTRLEN);
                    string dstaddress(temp);

                    output_bufs[cell] = CountPacket(match, dstaddress, update, output_bufs[cell]); //function used to count the specific packets (result stored in output_bufs)

                    ImGui::Text(output_bufs[cell].c_str());

                }
                else{
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(69,69,69,150));
                    ImGui::InputText("##cell", text_bufs[cell], IM_ARRAYSIZE(text_bufs[cell]));
                    ImGui::PopStyleColor(1);
                }

                ImGui::PopStyleColor(2);
                ImGui::PopID();
                
            }
            
            ImGui::EndTable();
        }
        ImGui::PopStyleVar();
    }
    ImGui::End();
}

void ShowTable(Router &router, bool &show_routingTable){
    

    routingTable rTable = router.getRoutingTable();
    ImGui::Begin("Table", &show_routingTable);

    // ImGui::PushItemWidth(ImGui::GetFontSize() * -2);

    auto window_size = ImGui::GetWindowSize();

    // Get routing table data from router
    auto rtable = rTable.get_rTable();
    auto rclients = rTable.get_clients();

    auto table_flags = ImGuiTableFlags_ScrollY|ImGuiTableFlags_ScrollX;
    
    
    if(ImGui::BeginTable("routing table", 1, table_flags, ImVec2(window_size.x/2, 0.0f)))
    {
        ImGui::TableSetupColumn("Routing Table");
        ImGui::TableHeadersRow();


        for(auto const& x : rtable)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Key: %s, Value: %s", x.first.c_str(), x.second.c_str());
        }
        ImGui::EndTable();
    }

    ImGui::SameLine();
    
    if(ImGui::BeginTable("client table", 1, table_flags, ImVec2(window_size.x/2, 0.0f)))
    {
        ImGui::TableSetupColumn("Client Table");
        ImGui::TableHeadersRow();

        
        for(auto const& x : rclients)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Client: %s", x.first.c_str());
        }
        ImGui::EndTable();
    }

    // ImGui::PopItemWidth();
    ImGui::End();
}
