#include "test_precomp.hpp"
#include "verifyer.hpp"
TEST(integration_testing, correctness)
{
    std::ifstream correctness_file;
    float last_correctness = 0;
    std::string path("./../../test/data/integration_test_data/correctness.txt");
    correctness_file.open(path);
    if(correctness_file.is_open())
    {
        correctness_file >> last_correctness;
        correctness_file.close();
    }

    //图->答案, 图list,读取文件中所有的图ignore .csv
    std::vector<std::string> img_types = {"jpg", "png"};
    std::string data_path("./../../test/data/integration_test_data/");
    std::string result_file("./../../test/data/integration_test_data/result.csv");
    Verifyer verifyer(data_path,result_file, img_types);
    verifyer.verify();
    float correctness = verifyer.getCorrectness();
    if(correctness >= last_correctness)
    {
        std::cout << "pass rate: " << correctness*100 << "%" << std::endl;
        std::ofstream correctness_file;
        correctness_file.open(path);
        if(correctness_file.is_open())
        {
            correctness_file << correctness;
            correctness_file.close();
        }
    }
    else
    {
        std::cout << "pass rate is lesser than last time!" << std::endl;
        std::cout << "last: " << last_correctness << ", now: " << correctness << std::endl;
    }

    //save correctness if bigger

}
