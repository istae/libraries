#pragma once

#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

#include <file.h>
#include <vector>
#include <string>
#include <image.h>
#include <imgui/imgui.h>
#include <imgui/imgui_glfw.h>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

void texture_image(Image* img)
{
    glGenTextures(1, &img->texture);
    glBindTexture(GL_TEXTURE_2D, img->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->width, img->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    // glGenerateTextureMipmap(img->texture);
    img->texture_loaded = 1;
}

void free_image(Image* img)
{
    if (img->pixels) {
        free(img->pixels);
        img->pixels = NULL;
    }
}

void reset_texture(Image* img)
{
    if (img->texture_loaded)
        glDeleteTextures(1, &img->texture);
    img->texture_loaded = 0;
}

void reset_image(Image* img)
{
    free_image(img);
    reset_texture(img);
}


//returns new size based on max side size
ImVec2 resize_ui(int x, int y, int max_side_size=300)
{
    if (x > max_side_size && y > max_side_size) {
        float d = (float)(x > y ? x : y) / max_side_size;
        return ImVec2(x/d, y/d);
    }
    return ImVec2(x, y);
}

std::vector<std::string> dirlist(const char* dir, int flg)
{
    // MAX_PATH  = 255
    DIR* dfd = opendir(dir);
    if (dfd == NULL) {
        fprintf(stderr, "cannot access %s\n", dir);
        exit(1);
    }
    struct dirent* dp;
    std::vector<std::string> dlist;
    while ((dp = readdir(dfd)) != NULL ) {
        if (strcmp(dp->d_name, ".") == 0)
            continue;
        if (flg == FILES_AND_DIRS)
            dlist.push_back(std::string(dp->d_name));
        else {
            char path[1000];
            sprintf(path, "%s/%s", dir, dp->d_name);
            int is = isdir(path);
            if (flg == FILES_ONLY && (!is))
                dlist.push_back(std::string(dp->d_name));
            else if (flg == DIRS_ONLY && is)
                dlist.push_back(std::string(dp->d_name));
        }
    }
    closedir(dfd);
    return dlist;
}

int fs(std::string& s, std::string& cd, const char* popup)
{
    static std::string current_dir = ".";
    if (!ImGui::BeginPopup(popup))
        return -1;

    int sindex = -1;
    std::vector<std::string> dlist = dirlist(current_dir.c_str(), FILES_AND_DIRS);
    ImGui::Text("Files");
    static ImGuiTextFilter filter;
    filter.Draw("");

    for (int i = 0; i < dlist.size(); i++) {
        if (filter.PassFilter(dlist[i].c_str())) {
            if (ImGui::Selectable(dlist[i].c_str())) {
                if (isdir((current_dir + "/" + dlist[i]).c_str())) {
                    current_dir += "/" + dlist[i];
                    filter.Clear();
                }
                else {
                    sindex = i;
                    s = dlist[i];
                    filter.Clear();
                }
                cd = current_dir;
                break;
            }
        }
    }
    ImGui::EndPopup();
    if (sindex == -1)
        ImGui::OpenPopup(popup);

    return sindex;
}

//pops up the file list
std::string image_select(int s)
{
    static std::vector<std::string> selected = {"",""};
    ImGui::SameLine();
    ImGui::Text(selected[s] == "" ? "<None>" : selected[s].c_str());

    std::string current_dir;
    int sindex;
    if  (s == 0)
        sindex = fs(selected[s], current_dir, "select 0");
    else if (s == 1)
        sindex = fs(selected[s], current_dir, "select 1");

    if (sindex == -1 )
        return "";
    else
        return current_dir + "/" + selected[s];
}