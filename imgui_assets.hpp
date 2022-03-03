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


void show_table(routingTable rTable, bool &show_routingTable){
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

void CountPacket(string match, string compare, bool update, std::string &cell)
{
    int counter = stoi(cell);
    auto start = match.find("(");
    auto end = match.find(")");
    string comparitor = match.substr(start,end);

    if(compare.substr(0,4) == comparitor && update)
    {
        counter++;
    }

    cell = to_string(counter);
}