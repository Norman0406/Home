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
    if (m_config.debugOutput()) {
        Serial.println("loading data");
    }

    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
        throw ConfigException("SPIFFS Mount Failed");
    }

    if (SPIFFS.exists("/data.json")) {
        File dataFile = SPIFFS.open("/data.json", "r");

        if (!dataFile) {
            throw ConfigException("Data file could not be opened");
        }

        if (m_config.debugOutput()) {
            Serial.println("opened data file");
        }

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

            if (m_config.debugOutput()) {
                serializeJson(jsonDocument, Serial);
                Serial.println();
            }

            JsonArray bsecState = json["bsec_state"].as<JsonArray>();
            for (auto value : bsecState) {
                m_bsecState.push_back(value.as<uint8_t>());
            }
        } else {
            throw ConfigException("failed to load json configuration");
        }

        dataFile.close();
    }
}

void Data::save() {
    if (m_config.debugOutput()) {
        Serial.println("saving configuration");
    }

    JsonDocument jsonDocument;

    JsonArray bsecState = jsonDocument["bsec_state"].to<JsonArray>();
    for (auto value : m_bsecState) {
        bsecState.add(value);
    }

    File dataFile = SPIFFS.open("/data.json", "w");
    if (!dataFile) {
        throw ConfigException("failed to open data file for writing");
    }

    if (m_config.debugOutput()) {
        serializeJson(jsonDocument, Serial);
        Serial.println();
    }

    serializeJson(jsonDocument, dataFile);
    dataFile.close();
}

std::vector<uint8_t> &Data::bsecState() { return m_bsecState; }
}  // namespace envi_probe
