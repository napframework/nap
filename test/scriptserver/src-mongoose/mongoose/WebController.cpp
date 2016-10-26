#include "WebController.h"
#include "Session.h"

namespace Mongoose
{        
    WebController::WebController(int gcDivisor_) 
        : 
        Controller(),
        gcDivisor(gcDivisor_),
        counter(0)
    {
    }

    void WebController::preProcess(Request &request, Response &response)
    {
        mutex.lock();
        counter++;

        if (counter > gcDivisor) {
            counter = 0;
            sessions->garbageCollect();
        }
        mutex.unlock();

        Session &session = sessions->get(request, response);
        session.ping();
        response.setHeader("Content-Type", "text/html");
    }
}
