//
// Created by YitaiWTQ on 2020/12/6.
//

#ifndef __OPENCV_BARCODE_VERIFYER_HPP__
#define __OPENCV_BARCODE_VERIFYER_HPP__
#include "barcode.hpp"
#include <windows.h>
#include <vector>
#include <map>
#include <regex>
typedef std::vector<std::string> stringvec;
class Verifyer
{
public:
    std::string data_dir;
    std::string result_file_path;
    std::map<std::string,std::string> dataset;
    float total_case_num;
    float correct_case_num;
private:
    cv::barcode::BarcodeDetector barcodeDetector;
    stringvec postfixes;

public:
    Verifyer(std::string data_dir, std::string result_file_path, stringvec postfixes);
    void verify();
    float getCorrectness();
    void reset();
private:
    void buildDataSet();

};
const stringvec explode(const std::string& s, const char& c)
{
    std::string buff{""};
    stringvec v;

    for(auto n:s)
    {
        if(n != c) buff+=n; else
        if(n == c && buff != "") { v.push_back(buff); buff = ""; }
    }
    if(buff != "") v.push_back(buff);

    return v;
}

void read_directory(const std::string& name, stringvec & v, const stringvec postfixes)
{
    std::string pattern(name);
    pattern.append("\\*");
    WIN32_FIND_DATA data;
    HANDLE hFind;
    if ((hFind = FindFirstFile(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE) {
        do {
            std::string filename(data.cFileName);
            std::string reg_args = "";
            for(auto str : postfixes)
            {
                if(reg_args.length() != 0)
                    reg_args.append("|");
                reg_args.append("(.*\\.");
                reg_args.append(str);
                reg_args.append(")");
            }
            std::regex rx(reg_args);
            if(std::regex_match(filename, rx))
            {
                v.push_back(filename);
            }

        } while (FindNextFile(hFind, &data) != 0);
        FindClose(hFind);
    }
}

Verifyer::Verifyer(std::string data_dir, std::string result_file_path, stringvec postfixes)
{
    this->data_dir = data_dir;
    this->result_file_path = result_file_path;
    this->postfixes = postfixes;
    this->total_case_num = 0;
    this->correct_case_num = 0;
    buildDataSet();
}

void Verifyer::reset()
{
    this->total_case_num = 0;
    this->correct_case_num = 0;
    dataset.clear();
}

float Verifyer::getCorrectness()
{
    return correct_case_num / total_case_num;
}

void Verifyer::verify()
{
    stringvec imgs_name;
    read_directory(data_dir,imgs_name, postfixes);
    for(auto img_name : imgs_name)
    {
        total_case_num++;
        cv::Mat img = cv::imread(data_dir+img_name);
        std::vector<std::vector<cv::Point2f>> points;
        stringvec infos;
        barcodeDetector.detectAndDecode(img, infos, points);
        if(infos.size() == 1)// 暂时先这么干
        {
            std::string result = infos[0];
            if(dataset.find(img_name) != dataset.end())
            {
                if(result == dataset[img_name])
                    correct_case_num ++;
                else
                    printf("wrong case:%s, wrong result:%s, right result:%s\n", img_name.c_str(),
                           infos[0].c_str(), dataset[img_name].c_str());
            }

        }
        else
        {
            std::cout << "wrong case: " << img_name << "no result" << std::endl;
        }
    }

}

void Verifyer::buildDataSet()
{
    std::ifstream result_file;
    result_file.open(result_file_path);
    std::string line;
    if(result_file.is_open())
    {
        while(std::getline(result_file,line))
        {
            stringvec result = explode(line,',');
            std::string filename = result[0];
            if(dataset.find(filename) == dataset.end())
            {
                dataset[filename] = result[1];//todo 改成多个
            }
        }
    }

    result_file.close();
}
#endif //__OPENCV_BARCODE_VERIFYER_HPP__
