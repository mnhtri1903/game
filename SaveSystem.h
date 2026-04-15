#ifndef SAVESYSTEM_H
#define SAVESYSTEM_H

#include <string>

struct SaveData {
    int  level       = 1;
    int  lixiCount   = 0;
    int  playerHealth= 100;
    int  jumpLevel   = 0;   // 0-3
    int  chargeLevel = 0;   // 0-3
    int  healthLevel = 0;   // 0-3
    int  heatLevel   = 0;   // 0-2
    bool hasShovel   = false;
};

namespace SaveSystem {
    bool save  (const std::string& path, const SaveData& data);
    bool load  (const std::string& path, SaveData& data);
    bool exists(const std::string& path);
}

#endif
