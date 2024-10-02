#include "bme680.h"

#include "exceptions.h"

/* Configure the BSEC library with information about the sensor
    18v/33v = Voltage at Vdd. 1.8V or 3.3V
    3s/300s = BSEC operating mode, BSEC_SAMPLE_RATE_LP or BSEC_SAMPLE_RATE_ULP
    4d/28d = Operating age of the sensor in days
    generic_18v_3s_4d
    generic_18v_3s_28d
    generic_18v_300s_4d
    generic_18v_300s_28d
    generic_33v_3s_4d
    generic_33v_3s_28d
    generic_33v_300s_4d
    generic_33v_300s_28d
*/
const uint8_t bsec_config_iaq[] = {
#include "config/generic_33v_3s_4d/bsec_iaq.txt"
};

#define STATE_SAVE_PERIOD \
    UINT32_C(360 * 60 * 1000)  // 360 minutes - 4 times a day

namespace envi_probe {
void BME680::begin(Configuration &config, envi_probe::Data &data) {
    m_data = &data;

    m_debugOutput = config.debugOutput();

    // initialize BME680
    Wire.begin();
    m_bsec.begin(BME680_I2C_ADDR_SECONDARY, Wire);
    if (m_bsec.status < BSEC_OK) {
        throw SensorException("Could not initialize BME680: " + m_bsec.status);
    }

    // initialize BME680
    m_bsec.setConfig(bsec_config_iaq);
    if (m_bsec.status < BSEC_OK) {
        throw SensorException("Could not apply BME680 config");
    }

    loadBsecState(data);

    const int numSensors = 13;
    bsec_virtual_sensor_t sensorList[numSensors] = {
        BSEC_OUTPUT_RAW_TEMPERATURE,
        BSEC_OUTPUT_RAW_PRESSURE,
        BSEC_OUTPUT_RAW_HUMIDITY,
        BSEC_OUTPUT_RAW_GAS,
        BSEC_OUTPUT_IAQ,
        BSEC_OUTPUT_STATIC_IAQ,
        BSEC_OUTPUT_CO2_EQUIVALENT,
        BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
        BSEC_OUTPUT_GAS_PERCENTAGE,
        BSEC_OUTPUT_STABILIZATION_STATUS,
        BSEC_OUTPUT_RUN_IN_STATUS};

    m_bsec.updateSubscription(sensorList, numSensors, BSEC_SAMPLE_RATE_LP);

    Serial.println("BME680 initialized");
}

void BME680::setTemperatureOffset(float offset) {
    m_bsec.setTemperatureOffset(offset);
}

BME680::Data BME680::read() {
    // wait until new data is available
    while (!m_bsec.run()) {
        if (m_bsec.status < BSEC_OK) {
            throw SensorException("Error reading BME680 data");
        }
    }

    if (m_data) {
        updateBsecState(*m_data);
    }

    return {m_bsec.iaq,
            m_bsec.rawTemperature,
            m_bsec.pressure,
            m_bsec.rawHumidity,
            m_bsec.gasResistance,
            m_bsec.stabStatus,
            m_bsec.runInStatus,
            m_bsec.temperature,
            m_bsec.humidity,
            m_bsec.staticIaq,
            m_bsec.co2Equivalent,
            m_bsec.breathVocEquivalent,
            m_bsec.compGasValue,
            m_bsec.gasPercentage,
            m_bsec.iaqAccuracy,
            m_bsec.staticIaqAccuracy,
            m_bsec.co2Accuracy,
            m_bsec.breathVocAccuracy,
            m_bsec.compGasAccuracy,
            m_bsec.gasPercentageAcccuracy};
}

void BME680::loadBsecState(envi_probe::Data &data) {
    const auto &state = data.bsecState();

    if (state.size() != BSEC_MAX_STATE_BLOB_SIZE) {
        return;
    }

    uint8_t bsecState[BSEC_MAX_STATE_BLOB_SIZE] = {0};
    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++) {
        bsecState[i] = state[i];
    }

    m_bsec.setState(bsecState);

    if (m_bsec.status < BSEC_OK) {
        throw SensorException("Could not set BSEC state");
    }
}

void BME680::updateBsecState(envi_probe::Data &data) {
    bool update = false;

    /* Set a trigger to save the state. Here, the state is saved every
     * STATE_SAVE_PERIOD with the first state being saved once the algorithm
     * achieves full calibration, i.e. iaqAccuracy = 3 */
    if (m_stateUpdateCounter == 0) {
        if (m_bsec.iaqAccuracy >= 3) {
            update = true;
            m_stateUpdateCounter++;
        }
    } else {
        /* Update every STATE_SAVE_PERIOD milliseconds */
        if ((m_stateUpdateCounter * STATE_SAVE_PERIOD) < millis()) {
            update = true;
            m_stateUpdateCounter++;
        }
    }

    if (update) {
        uint8_t bsecState[BSEC_MAX_STATE_BLOB_SIZE] = {0};
        m_bsec.getState(bsecState);

        auto &state = data.bsecState();
        state.clear();
        state.resize(BSEC_MAX_STATE_BLOB_SIZE);
        for (unsigned i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++) {
            state[i] = bsecState[i];
        }
        data.save();
    }
}
}  // namespace envi_probe
