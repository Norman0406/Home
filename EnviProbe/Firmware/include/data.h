#pragma once

#include <string>
#include <vector>

namespace envi_probe {
class Configuration;

class Data {
public:
    Data(const Configuration &config);

    void load();
    void save();

    std::vector<uint8_t> &bsecState();

private:
    const Configuration &m_config;
    std::vector<uint8_t> m_bsecState;
};
}  // namespace envi_probe
