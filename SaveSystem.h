#ifndef SAVESYSTEM_H
#define SAVESYSTEM_H

#include <string>

struct SaveData {
    int  level       = 1;
    int  lixiCount   = 0;
    int  playerHealth= 100;
    int  jumpLevel   = 0; 
    int  chargeLevel = 0; 
    int  healthLevel = 0;   
    int  heatLevel   = 0;   
    bool hasShovel   = false;
};

namespace SaveSystem {
    bool save  (const std::string& path, const SaveData& data);
    bool load  (const std::string& path, SaveData& data);
    bool exists(const std::string& path);
}

#endif
