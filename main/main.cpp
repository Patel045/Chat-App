// #include <iostream>
#include "../utils/Logger.h"

int main(){
    Logger Log(Location::TEXT_FILE);
    Log.log("Hello World");
    Log.log({"Hello World","I am Aayush","This is C++ \nOOPs \n"});
    return 0;
}