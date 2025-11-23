#pragma once

#include <vector>

namespace junox {

/**
 * Ring buffer for delay effects.
 */
class RingBuffer {
public:
    RingBuffer(int size)
        : buffer(size, 0.0f)
        , writePos(0)
    {
    }

    void writeSample(float sample) {
        buffer[writePos] = sample;
        writePos = (writePos + 1) % buffer.size();
    }

    float readSample(float delaySamples) const {
        // Calculate read position
        float readPos = writePos - delaySamples;
        while (readPos < 0.0f) {
            readPos += buffer.size();
        }
        
        // Linear interpolation
        const int i = static_cast<int>(readPos);
        const float frac = readPos - i;
        const int i1 = i % buffer.size();
        const int i2 = (i + 1) % buffer.size();
        
        return buffer[i1] + frac * (buffer[i2] - buffer[i1]);
    }

    void reset() {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        writePos = 0;
    }

private:
    std::vector<float> buffer;
    int writePos;
};

} // namespace junox
