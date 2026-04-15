#include "SaveSystem.h"
#include <fstream>

bool SaveSystem::exists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

bool SaveSystem::save(const std::string& path, const SaveData& data) {
    std::ofstream f(path);
    if (!f.is_open()) return false;
    f << "LEVEL "        << data.level        << "\n";
    f << "LIXI "         << data.lixiCount    << "\n";
    f << "HEALTH "       << data.playerHealth << "\n";
    f << "JUMP_LEVEL "   << data.jumpLevel    << "\n";
    f << "CHARGE_LEVEL " << data.chargeLevel  << "\n";
    f << "HEALTH_LEVEL " << data.healthLevel  << "\n";
    f << "HEAT_LEVEL "   << data.heatLevel    << "\n";
    f << "SHOVEL "       << (data.hasShovel ? 1 : 0) << "\n";
    return true;
}

bool SaveSystem::load(const std::string& path, SaveData& data) {
    std::ifstream f(path);
    if (!f.is_open()) return false;
    std::string key;
    while (f >> key) {
        if      (key == "LEVEL")        f >> data.level;
        else if (key == "LIXI")         f >> data.lixiCount;
        else if (key == "HEALTH")       f >> data.playerHealth;
        else if (key == "JUMP_LEVEL")   f >> data.jumpLevel;
        else if (key == "CHARGE_LEVEL") f >> data.chargeLevel;
        else if (key == "HEALTH_LEVEL") f >> data.healthLevel;
        else if (key == "HEAT_LEVEL")   f >> data.heatLevel;
        else if (key == "SHOVEL") {
            int s; f >> s;
            data.hasShovel = (s != 0);
        }
    }
    return true;
}
