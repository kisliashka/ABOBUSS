#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include "Course_project.h"
//#include "symbols.h"
using namespace std;

int main()
{
    setlocale(LC_ALL, "RUS");
    Compile("pascal.txt");
    //o_file("pascal.txt");
    return 0;
}