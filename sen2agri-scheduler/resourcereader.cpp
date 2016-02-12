#include "resourcereader.hpp"

ResourceReader::ResourceReader()
{

}

std::vector<NodeLoad> ResourceReader::readSystemLoad()
{
    std::vector<NodeLoad> nodes;

    // TODO: call Persistence Manager

    return nodes;
}

bool ResourceReader::areResourcesAvailable()
{
    bool available = false;
    float weighted = 0;

    std::vector<NodeLoad> nodes = readSystemLoad();
    for (auto& node : nodes)
    {
        weighted += node.load_percent;
        if (node.load_percent < 0.3)
            available = true;
    }
    weighted /= nodes.size();
    if (weighted < 0.7)
        available = true;

    return available;
}
