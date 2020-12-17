//
// Created by YitaiWTQ on 2020/12/6.
//

#ifndef __OPENCV_BARCODE_VERIFIER_HPP__
#define __OPENCV_BARCODE_VERIFIER_HPP__

#include "barcode.hpp"
#include <fstream>
#include <iostream>
#include <windows.h>
#include <utility>
#include <vector>
#include <unordered_map>
#include <regex>

using stringvec = std::vector<std::string>;
using datasetType = std::unordered_map<std::string, std::string>;

class Verifier
{
public:
    std::string data_dir;
    std::string result_file_path;
    datasetType dataset;
    float total_case_num;
    float correct_case_num;
private:
    cv::barcode::BarcodeDetector barcodeDetector;
    stringvec postfixes;

public:
    Verifier(std::string data_dir, std::string result_file_path, stringvec postfixes);

    void verify();

    float getCorrectness() const;

    void reset();

private:
    void buildDataSet();

};

stringvec explode(const std::string &s, const char &c)
{
    std::string buff;
    stringvec v;

    for (auto n:s)
    {
        if (n != c)
        { buff += n; }
        else if (n == c && !buff.empty())
        {
            v.push_back(buff);
            buff = "";
        }
    }
    if (!buff.empty())
    { v.push_back(buff); }

    return v;
}

void read_directory(const std::string &name, stringvec &v, const stringvec& postfixes)
{
    std::string pattern{name};
    pattern.append(R"(\*)");
    WIN32_FIND_DATA data;
    HANDLE hFind;
    if ((hFind = FindFirstFile(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            std::string filename{data.cFileName};
            std::string reg_args;
            for (const auto &str : postfixes)
            {
                if (reg_args.length() != 0)
                {
                    reg_args.append("|");
                }
                reg_args.append(R"((.*\.)");
                reg_args.append(str);
                reg_args.append(")");
            }
            std::regex rx(reg_args);
            if (std::regex_match(filename, rx))
            {
                v.push_back(filename);
            }

        } while (FindNextFile(hFind, &data) != 0);
        FindClose(hFind);
    }
}

Verifier::Verifier(std::string data_dir, std::string result_file_path, stringvec postfixes) : data_dir(
        std::move(data_dir)), result_file_path(std::move(result_file_path)), postfixes(std::move(postfixes)),
                                                                                              total_case_num(0.0f),
                                                                                              correct_case_num(0.0f)
{
    buildDataSet();
}

void Verifier::reset()
{
    this->total_case_num = 0;
    this->correct_case_num = 0;
    dataset.clear();
}

float Verifier::getCorrectness() const
{
    return correct_case_num / total_case_num;
}

void Verifier::verify()
{
    stringvec imgs_name;
    read_directory(data_dir, imgs_name, postfixes);
    for (const auto &img_name : imgs_name)
    {
        total_case_num++;
        cv::Mat img = cv::imread(data_dir+img_name);
        std::vector<cv::Point2f> points;
        stringvec infos;
        barcodeDetector.detectAndDecode(img, infos, points);
        if(infos.size() == 1)// 暂时先这么干
        {
            std::string result = infos[0];
            if (dataset.find(img_name) != dataset.end())
            {
                if (result == dataset[img_name])
                {
                    correct_case_num++;
                }
                else
                {
                    printf("wrong case:%s, wrong result:%s, right result:%s\n", img_name.c_str(), infos[0].c_str(),
                           dataset[img_name].c_str());
                }
            }

        }
        else
        {
            std::cout << "wrong case: " << img_name << "no result" << std::endl;
        }
    }

}

void Verifier::buildDataSet()
{
    std::ifstream result_file;
    result_file.open(result_file_path);
    std::string line;
    if (result_file.is_open())
    {
        while (std::getline(result_file, line))
        {
            stringvec result = explode(line, ',');
            std::string filename = result[0];
            if (dataset.find(filename) == dataset.end())
            {
                dataset[filename] = result[1];//todo 改成多个
            }
        }
    }

    result_file.close();
}

#endif //__OPENCV_BARCODE_VERIFIER_HPP__
