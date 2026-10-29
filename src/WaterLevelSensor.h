#pragma once
#include <Arduino.h>

// Wraps a JSN-SR04T (waterproof ultrasonic) distance sensor and turns raw
// echo timing into a filtered fill percentage. Handles the two failure
// modes that actually matter on a tank: a splashy surface (noisy single
// reads) and a sensor that's stopped responding entirely.
class WaterLevelSensor {
public:
    WaterLevelSensor(uint8_t trigPin, uint8_t echoPin, uint16_t tankHeightCm, uint16_t deadbandCm);

    void begin();

    // Takes a burst of samples, throws out outliers, and returns a
    // median-filtered fill percentage (0-100). Updates fault state as
    // a side effect -- call isFaulted() after to check the read was good.
    uint8_t readPercent();

    bool isFaulted() const { return consecutiveBadReads_ >= faultThreshold_; }
    void setFaultThreshold(uint8_t reads) { faultThreshold_ = reads; }

private:
    uint8_t trigPin_;
    uint8_t echoPin_;
    uint16_t tankHeightCm_;
    uint16_t deadbandCm_;
    uint8_t consecutiveBadReads_ = 0;
    uint8_t faultThreshold_ = 5;

    // Single raw echo->cm read. Returns -1 if the sensor times out
    // (no echo, or something out of the sensor's usable range).
    int16_t sampleOnceCm();
};
