#include "data.h"

#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>

#include "configuration.h"
#include "exceptions.h"

#define FORMAT_SPIFFS_IF_FAILED true

namespace envi_probe {
Data::Data(const Configuration &config) : m_config{config} {}

void Data::load() {
    log_i("Loading data from %s", m_dataFile.c_str());

    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
        throw ConfigException("SPIFFS Mount Failed");
    }

    if (!SPIFFS.exists("/data.json")) {
        save();
        ESP.restart();
    }

    File dataFile = SPIFFS.open(m_dataFile.c_str(), "r");

    if (!dataFile) {
        throw ConfigException("Data file could not be opened");
    }

    log_i("Opened data file");

    size_t size = dataFile.size();

    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    size_t bytesRead = dataFile.readBytes(buf.get(), size);

    if (bytesRead != size) {
        throw ConfigException("Could not read data file");
    }

    JsonDocument jsonDocument;
    auto error = deserializeJson(jsonDocument, buf.get());

    if (!error) {
        JsonObject json = jsonDocument.as<JsonObject>();

        String serializedJson;
        serializeJson(jsonDocument, serializedJson);
        log_i("Json: %s", serializedJson.c_str());

#ifdef HAS_BME680
        JsonArray bsecState = json["bsec_state"].as<JsonArray>();
        for (auto value : bsecState) {
            m_bsecState.push_back(value.as<uint8_t>());
        }
#endif
    } else {
        throw ConfigException("Failed to load json configuration");
    }

    dataFile.close();

    log_i("Data loaded");
}

void Data::save() {
    log_i("Saving data to %s", m_dataFile.c_str());

    JsonDocument jsonDocument = JsonObject();

#ifdef HAS_BME680
    JsonArray bsecState = jsonDocument["bsec_state"].to<JsonArray>();
    for (auto value : m_bsecState) {
        bsecState.add(value);
    }
#endif

    File dataFile = SPIFFS.open(m_dataFile.c_str(), "w");
    if (!dataFile) {
        throw ConfigException("Failed to open data file for writing");
    }

    String serializedJson;
    serializeJson(jsonDocument, serializedJson);
    log_i("Json: %s", serializedJson.c_str());

    serializeJson(jsonDocument, dataFile);
    dataFile.close();

    log_i("Data saved");
}

void Data::clear() {
    log_i("Clearing data");

    SPIFFS.remove(m_dataFile.c_str());

    log_i("Data cleared");
}

#ifdef HAS_BME680
std::vector<uint8_t> &Data::bsecState() { return m_bsecState; }
#endif
}  // namespace envi_probe
