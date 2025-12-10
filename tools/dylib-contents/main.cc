#define PATTERN_SHOW_SELF_REGISTERING
#include "../../patterns.h"

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        pattern::Constructors::load_all_from_library("./",argv[i]);
    }
    std::cout<<"Contents"<<std::endl<<pattern::Constructors::to_string()<<std::endl;
}

