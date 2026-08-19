/*读取用户输入的文件路径，判断路径对应文件类型，为普通文件或文件夹，则递归遍历文件，遍历到的文件若为视频，路径拼接进ffmpeg指令*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <iostream>
#include <format>
#include <algorithm>
#include <filesystem>
using namespace std::filesystem;
//string.h_extension
void strncpy(char* c_str1, const size_t n, const char* c_str2) {        //使 strncpy 支持 (c_str1, size_t, c_str2) 传参
    strncpy(c_str1, c_str2, n);
}
//stdlib.h_extension
void system(const std::string& s) {                                     //使 system 支持 string 传参
    system(s.c_str());
}
void system(const char c) {                                             //使 system 支持字符传参
    char c_str[2] = {c, '\0'};
    system(c_str);
}
//filesystem_extension
void clear_path(std::string& s) {                                       //清理路径：从外向内剔除双引号、问号，最极端的情况是："?\\?NAS\XXX"
    while (s.size() > 0 && s[0] == '\"')                                //清理路径开头的双引号："?\\?NAS\XXX"
        s.erase(0, 1);
    while (s.size() > 0 && s[s.size()-1] == '\"')                       //清理路径末尾的双引号：?\\?NAS\XXX"
        s.erase(s.size()-1, 1);
    if (s.size() > 0 && s[0] == '?')                                    //清理根路径前的问号：?\\?NAS\XXX
        s.erase(0, 1);
    if (s.size() > 2 && s[2] == '?')                                    //清理网络路径 \\ 后的问号：\\?NAS\XXX
        s.erase(2, 1);
}
void clear_path(char* c_str) {
    std::string s = c_str;
    clear_path(s);
    strcpy(c_str, s.c_str());
}
void clear_path(std::filesystem::path& p) {
    std::string s = p.string();
    clear_path(s);
    p = std::filesystem::path(s);
}
void clear_path(std::filesystem::directory_entry& de) {
    std::string s = de.path().string();
    clear_path(s);
    de = std::filesystem::directory_entry(s);
}
bool is_video(const std::string& s) {                                   //判断文件是否为视频
    std::string str = s;
    clear_path(str);
    std::string ext = std::filesystem::path(str).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);     //转小写
    if (std::filesystem::is_regular_file(str) && (ext == ".mp4" || ext == ".mkv" || ext == ".webm" || ext == ".mov" || ext == ".m3u8" || ext == ".avi" || ext == ".flav"))
        return true;
    else
        return false;
}
bool is_video(const char* c_str) {
    std::string s = c_str;
    if (is_video(s))
        return true;
    else
        return false;
}
bool is_video(const std::filesystem::path& p) {
    if (is_video(p.string()))
        return true;
    else
        return false;
}
bool is_video(const std::filesystem::directory_entry& de) {
    if (is_video(de.path().string()))
        return true;
    else
        return false;
}
int main (void) {
    std::string input;                                                                  //存储用户输入的文件的完整路径
    input.reserve(4096);
//读取用户输入路径
    std::cout << "The complete path of the file:\n";
label:
    std::getline(std::cin >> std::ws, input);
    clear_path(input);
//拼接路径至 ffmpeg 指令，并输入终端
    if (is_regular_file(input) && is_video(input))
        system(std::format(R"(ffmpeg -i "{}" -map 0 -map_metadata 0 -c copy -c:v libx265 -crf 23 -preset medium -cues_to_front yes "{}_NEW.mkv")", input, (path(input).parent_path()/path(input).stem()).string()));
    else if (is_directory(input))
        for (const directory_entry& each : recursive_directory_iterator(input)) {       //递归遍历文件/文件夹
            if (is_video(each))                                                         //判断是否为视频
                system(std::format(R"(ffmpeg -i "{}" -map 0 -map_metadata 0 -c copy -c:v libx265 -crf 23 -preset medium -cues_to_front yes "{}_NEW.mkv")", each.path().string(), (each.path().parent_path()/each.path().stem()).string()));
        }
    else std::cout << "error";
//跳转，重新读取用户输入
    goto label;
}