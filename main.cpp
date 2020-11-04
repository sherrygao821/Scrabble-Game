#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
using namespace std;
/*
 * 计算检测设备
 */
int get_parallel_rate(vector<vector<double> > concentration_pm){
    int i;
    int j;
    int m = concentration_pm[0].size(); // 共有m台设备
    int k = concentration_pm.size(); // 共有k组样本

    // 求出Cj：每一组仪器对于一份样品的测量平均值
    vector<double> concentrationAvg;
    for(j = 0; j < k; j++){
        double concentrationSum = 0;
        for(i = 0; i < m; i++){
            concentrationSum += concentration_pm[j][i];
        }
        concentrationAvg.push_back(concentrationSum / m);
    }

    // 求出Pj
    vector<double> parallelRate;
    for(j = 0; j < k; j++){
        double differenceSum = 0;
        for(i = 0; i < m; i++){
            differenceSum += pow(concentration_pm[j][i] - concentrationAvg[j], 2);
        }
        parallelRate.push_back(sqrt(differenceSum / (m - 1)) / concentrationAvg[j]);
    }
    cout << "Pj" << endl;
    for(int l = 0; l < k; l++){
        cout << parallelRate[l] << endl;
    }

    // 求出P
    double parallelRate_sum = 0;
    for(j = 0; j < k; j++){
        parallelRate_sum += pow(parallelRate[j], 2);
    }
    cout << parallelRate_sum << endl;
    double P = sqrt(parallelRate_sum / k);
    cout << "p : " << P << endl;
    if(P <= 0.15){
        return 1;
    }
    else{
        return 0;
    }
}

int get_relevance_rate(vector<vector<double> > concentration_pm){
    int i;
    int j;
    int n = concentration_pm.size(); // 共有n组样本
    int net_count = concentration_pm[0].size() - 1;
    int standard_count = 1;
    double C, R;

    // 找到标准检测仪器每组测量的平均值，并求出所有样本的总平均值
    vector<double> standard_concentrationAvg; // Rj
    for(j = 0; j < n; j++){
        standard_concentrationAvg.push_back(concentration_pm[j][0]);
    }
    double sum_standard_concentrationAvg = 0; // R
    for(j = 0; j < n; j++){
        sum_standard_concentrationAvg += standard_concentrationAvg[j];
    }
    C = sum_standard_concentrationAvg / n;

    // 找到网格化检测质控设备每组测量的平均值，并求出所有样本的总平均值
    vector<double> net_concentrationAvg; // Cj
    for(j = 0; j < n; j++){
        double concentrationSum = 0;
        for(i = 1; i < concentration_pm[0].size(); i++){
            concentrationSum += concentration_pm[j][i];
        }
        net_concentrationAvg.push_back(concentrationSum / net_count);
    }
    double sum_net_concentrationAvg = 0; // C
    for(j = 0; j < n; j++){
        sum_net_concentrationAvg += net_concentrationAvg[j];
    }
    R = sum_net_concentrationAvg / n;

    cout << "C: " << C << " R: " << R << endl;

    // 用已给公式求relevance rate
    double sum1 = 0;
    double sum2 = 0;
    double sum3 = 0;
    for(j = 0; j < n; j++){
        sum1 += (standard_concentrationAvg[j] - R) * (net_concentrationAvg[j] - C);
        sum2 += pow((standard_concentrationAvg[j] - R), 2);
        sum3 += pow((net_concentrationAvg[j] - C), 2);
    }
    double relevanceRate = sum1 / sqrt(sum2 * sum3);
    cout << relevanceRate << endl;
    if(relevanceRate >= 0.85){
        return 1;
    }
    else{
        return 0;
    }
}

int main() {
    ifstream ifile("test.txt");
    if(ifile.fail()){
        cout << "wrong file" << endl;
        return -1;
    }

    stringstream ss;
    string buffer;
    vector<vector<double> > concentration_PM;

    while(getline(ifile,buffer)){
        ss << buffer;
        double temp;
        int i = 0;
        vector<double> temp1;
        while(ss >> temp){
            temp1.push_back(temp);
            i++;
        }
        ss.clear();
        ss.str("");
        buffer.clear();
        concentration_PM.push_back(temp1);
    }

    cout << get_relevance_rate(concentration_PM) << endl;
    return 0;
}
