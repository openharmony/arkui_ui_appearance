/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
 */

#ifndef UI_APPEARANCE_SUNRISE_SUNSET_CALC_H
#define UI_APPEARANCE_SUNRISE_SUNSET_CALC_H

#include <ctime>
#include <string>
#include <cmath>

namespace DarkModeConstants {
    constexpr int32_t MS_PER_SEC = 1000;
    constexpr int32_t SECS_PER_MIN = 60;
    constexpr int32_t MINS_PER_HOUR = 60;
    constexpr int32_t SECS_PER_DAY = 86400;
    constexpr int32_t MINS_PER_DAY = 1440;
}

/**
 * Sunrise and sunset calculation result.
 */
struct SunriseSunsetResult {
    /** Sunrise time in UTC seconds from 00:00; -1 for polar day or polar night. */
    double sunrise;
    /** Sunset time in UTC seconds from 00:00; -1 for polar day or polar night. */
    double sunset;
};

/**
 * Calculates sunrise and sunset using the NOAA solar position algorithm.
 */
namespace SunriseSunsetUtils {

    /**
     * Uses two iterations for accuracy: the first starts at UTC noon, and the second
     * recalculates with a Julian day adjusted by the first result.
     * @param lat Latitude in the range [-90, 90].
     * @param lon Longitude in the range [-180, 180].
     * @param timestampMs Timestamp in milliseconds.
     * @returns Sunrise and sunset in UTC seconds from 00:00; -1 for polar day or polar night.
     */
    SunriseSunsetResult calculateSunriseSunset(double lat, double lon, int64_t timestampMs);

    /**
     * Formats UTC seconds as local time in HH:mm format, rounded to the nearest minute.
     * @param utcSec UTC seconds from 00:00; -1 for polar day or polar night.
     * @param timezoneOffset Time zone offset in minutes, for example -480 for UTC+8.
     * @returns The formatted time, or "---" for polar day or polar night.
     */
    std::string formatTimeFromSec(double utcSec, int timezoneOffset);

} // namespace SunriseSunsetUtils

/**
 * Sunrise and sunset condition.
 */
enum class SunCondition {
    /** Normal sunrise and sunset. */
    NORMAL,
    /** Polar day, when the sun does not set. */
    POLAR_DAY,
    /** Polar night, when the sun does not rise. */
    POLAR_NIGHT
};

/**
 * Binds coordinates and a date to sunrise and sunset condition and formatting operations.
 */
class SunriseSunsetInfo {
public:
    /**
     * Calculates sunrise and sunset using the specified coordinates and timestamp.
     * The time zone offset is obtained from the system time zone.
     * @param lat Latitude in the range [-90, 90].
     * @param lon Longitude in the range [-180, 180].
     * @param timestampMs Timestamp in milliseconds.
     */
    SunriseSunsetInfo(double lat, double lon, int64_t timestampMs);

    /**
     * @param timestampMs Timestamp in milliseconds.
     * @returns true if the timestamp is during daytime; false otherwise.
     */
    bool isDaytime(int64_t timestampMs) const;

    /**
     * @param timestampMs Timestamp in milliseconds.
     * @returns true if the timestamp is during nighttime; false otherwise.
     */
    bool isNighttime(int64_t timestampMs) const;

    /**
     * @returns true for polar day, when the sun does not set; false otherwise.
     */
    bool isPolarDay() const;

    /**
     * @returns true for polar night, when the sun does not rise; false otherwise.
     */
    bool isPolarNight() const;

    /**
     * @returns Daylight duration in minutes, or -1 for polar day or polar night.
     */
    int32_t getDayLength() const;

    /**
     * Returns sunset as minutes from midnight for use as the dark mode start time.
     * For example, 19:14 is returned as "1154".
     * @returns Sunset in minutes as a string, or an empty string for polar day or polar night.
     */
    std::string getSunsetStr() const;

    /**
     * Returns sunrise as minutes from midnight for use as the dark mode end time.
     * When local sunrise precedes local sunset, MINS_PER_DAY is added to represent the next day;
     * for example, 05:07 on the next day is returned as "1747".
     * @returns Sunrise in minutes as a string, or an empty string for polar day or polar night.
     */
    std::string getSunriseStr() const;

private:
    /**
     * @param utcSec UTC seconds from 00:00.
     * @returns Local minutes from midnight in the range [0, 1439].
     */
    int32_t toLocalMinutes(double utcSec) const;

    /** Sunrise time in UTC seconds from 00:00; -1 for polar day or polar night. */
    double sunrise_;
    /** Sunset time in UTC seconds from 00:00; -1 for polar day or polar night. */
    double sunset_;
    /** System time zone offset in minutes, for example -480 for UTC+8. */
    int timezoneOffset_;
    /** Current sunrise and sunset condition. */
    SunCondition condition_;

    static constexpr double POLAR_MARK = -1;
};

#endif // UI_APPEARANCE_SUNRISE_SUNSET_CALC_H
