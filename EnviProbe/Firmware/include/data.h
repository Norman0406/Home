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
    void clear();

#ifdef HAS_BME680
    std::vector<uint8_t> &bsecState();
#endif

private:
    const std::string m_dataFile{"/data.json"};
    const Configuration &m_config;

#ifdef HAS_BME680
    std::vector<uint8_t> m_bsecState;
#endif
};
}  // namespace envi_probe
