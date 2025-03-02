#include <opencv2/opencv.hpp>
#include "manager.h"
#include "jsonUtils.h"

using namespace std;
using namespace cv;

int main(int argc, char* argv[])
{
    bool debugMode = false;
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            if (string(argv[i]) == "debug") {
                
                break;
            }
        }
    }
    int processID = readFromJson("ID");
    Manager manager(processID);
    manager.init();
    manager.mainDemo();
    return 0;
}