import matplotlib
import matplotlib.pyplot as plt
import sys

matplotlib.use('TkAgg')

if __name__ == '__main__':
    file_name = sys.argv
    print(sys.argv[0])
    # 读取数据文件, 默认数据文件名data.csv
    data = []
    with open("./data.csv") as file_obj:
        line = file_obj.readline()
        tempData = line.split(",")
        for num in tempData:
            if len(num) != 0 and num != '\n':
                data.append(float(num))
            
    
    plt.plot(range(0,len(data)),data)
    plt.show()
        





