#ifndef DataSource_H
#define DataSource_H

class DataSource {
public:
    virtual std::string GetName() { return "postgis"; };

};

#endif
