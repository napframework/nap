#ifndef _MONGOOSE_UTILS_H
#define _MONGOOSE_UTILS_H

#include <iostream>

using namespace std;

namespace Mongoose
{
    class Utils
    {
        public:
            static string htmlEntities(string data);
            static void sleep(int ms);
    };
}

#endif

