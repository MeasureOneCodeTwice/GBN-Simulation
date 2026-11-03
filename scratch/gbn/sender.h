#pragma once

#include "ns3/applications-module.h"

using namespace ns3;


std::string message() {
    return "Hello from subfile";
}

class GbnSender : public SourceApplication {
    public:
    private:
};
