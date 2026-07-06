#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace arsonkupik {

constexpr double kDefaultSampleRate = 48000.0;
constexpr int kMaxChannels = 8;

double db_to_gain(double db);
double gain_to_db(double gain);
double clamp(double value, double lo, double hi);
double clamp01(double value);

} // namespace arsonkupik
