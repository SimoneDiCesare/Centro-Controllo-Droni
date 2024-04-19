#include <iostream>
#include <string>
#include "ClassDroni.hpp"
#include "channel.hpp"

int main(int argc, char* argv[]){
    Channel *c = new Channel (0);   
    c -> connect();
}