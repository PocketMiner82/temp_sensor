#ifndef TEMP_SENSOR_CACHEDMEDIANFILTER_H
#define TEMP_SENSOR_CACHEDMEDIANFILTER_H

#include <array>
#include <algorithm>

template <typename T, size_t N>
class MovingMedianCollection {
    // Main storage for the circular buffer (FIFO order)
    std::array<T, N> buffer{};

    // Index of the oldest element / write position
    size_t head_index = 0;

    // Number of valid elements currently in the buffer
    size_t filled_count = 0;

public:
    /**
     * Updates the circular buffer.
     * @param value the value to add to the collection.
     */
    void add(T value) {
        // overwrite oldest value
        buffer[head_index] = value;

        // circular logic
        head_index = (head_index + 1) % N;

        // Track fill count for the startup phase
        if (filled_count < N) {
            filled_count++;
        }
    }

    /**
     * Calculates and returns the median value.
     * @returns the value at the middle of the sorted collection.
     */
    T get_median() {
        if (filled_count == 0) {
            return T{};
        }

        // do not modify the main buffer to preserve FIFO order
        std::array<T, N> temp_buffer;

        for (size_t i = 0; i < filled_count; ++i) {
            temp_buffer[i] = buffer[i];
        }

        size_t n = filled_count / 2;

        // rearrange elements so the element at the middle is the median. faster than full sort
        std::nth_element(temp_buffer.begin(), temp_buffer.begin() + n, temp_buffer.begin() + filled_count);

        return temp_buffer[n];
    }
};

#endif //TEMP_SENSOR_CACHEDMEDIANFILTER_H