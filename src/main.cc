/*************************************************************************
*
*              Author: b51
*                Mail: b51live@gmail.com
*            FileName: main.cc
*
*          Created On: Thu Jul  9 23:51:12 2020
*     Licensed under The MIT License [see LICENSE for details]
*
************************************************************************/

#include <iostream>
#include <fstream>
#include <vector>
#include <ceres/ceres.h>
#include <Eigen/Core>
#include <Eigen/Dense>

#include "GaussNewton.h"

void CallPythonPlot(double m1, double c1, double m2, double c2) {
  // plot_curve.py Usage: python plot_curve.py sample.txt m c
  std::string cmd = "python ../scripts/plot_curve.py ../data/sample.txt ";
  cmd += std::to_string(m1) + " " + std::to_string(c1) + " " + std::to_string(m2)
         + " " + std::to_string(c2);
  std::cout << cmd << std::endl;
  system(cmd.c_str());
}

void ReadDataFromFile(const std::string& fullpath,
                      std::vector<std::vector<double>>& datas) {
  std::ifstream f;
  // open file with file path
  f.open(fullpath.c_str());
  std::string line;

  // read data frome file line toucjby line
  while (std::getline(f, line)) {
    std::istringstream iss(line);
    std::vector<double> data;
    std::string str_data;
    iss >> str_data;
    if (str_data == "#")
      continue;

    // std::stod, convert string -> double
    data.emplace_back(std::stod(str_data));
    while (iss >> str_data)
      data.emplace_back(std::stod(str_data));

    datas.emplace_back(data);
  }
  // close file
  f.close();
}

struct ExponentialResidual {
  ExponentialResidual(double x, double y)
      : x_(x), y_(y) {}
  template <typename T> bool operator()(const T* const m,
                                        const T* const c,
                                        T* residual) const {
    /* Fill here */
    residual[0] = T(y_) - exp(m[0] * T(x_) + c[0]);
    return true;
  }
 private:
  const double x_;
  const double y_;
};

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " [data path]" << std::endl;
    return -1;
  }
  double m = 0.0;
  double c = 0.0;
  std::vector<std::vector<double>> datas;
  ReadDataFromFile(argv[1], datas);

  /**
   *  Optimize curve with ceres, check link below
   *  http://ceres-solver.org/nnls_tutorial.html#curve-fitting
   */
  ceres::Problem problem;
  for (size_t i = 0; i < datas.size(); i++) {
    problem.AddResidualBlock(
        new ceres::AutoDiffCostFunction<ExponentialResidual, 1, 1, 1>(
            new ExponentialResidual(datas[i][0], datas[i][1])),
        nullptr, &m, &c);
  }

  ceres::Solver::Options options;
  options.max_num_iterations = 25;
  options.linear_solver_type = ceres::DENSE_QR;
  options.minimizer_progress_to_stdout = true;
  ceres::Solver::Summary summary;
  ceres::Solve(options, &problem, &summary);
  std::cout << summary.BriefReport() << "\n";
  std::cout << "Initial m: " << 0.0 << " c: " << 0.0 << "\n";
  std::cout << "Final   m: " << m << " c: " << c << "\n";

  /**
   *  Optimize curve with Gauss Newton Non-linear Least Squares method
   *  which written in your own code
   */

  std::cout<<std::endl<<"GAUSS NEWTON"<<std::endl;


  int max_iterations = 50;//GN算法拟合
  std::vector<double> variables;
  GaussNewton gauss_newton(datas, max_iterations);
  gauss_newton.Optimize();
  variables = gauss_newton.GetOptimizedVariables();
  CallPythonPlot(m, c, variables[m], variables[c]);


  std::cout<<std::endl<<"LM"<<std::endl;

  max_iterations = 100;//LM算法拟合
  std::vector<double> variables1;
 GaussNewton lm(datas, max_iterations);
 lm.Optimize();
 variables = lm.GetOptimizedVariables();
 CallPythonPlot(m, c, variables[m], variables[c]);



  return 0;
}
