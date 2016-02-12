#ifndef RESOURCEREADER_H
#define RESOURCEREADER_H

#include <vector>
#include <QString>

struct NodeLoad
{
    QString node_name;
    float   load_percent;
};

class ResourceReader
{
public:
    ResourceReader();
    std::vector<NodeLoad> readSystemLoad();
    bool areResourcesAvailable();
};

#endif // RESOURCEREADER_H
