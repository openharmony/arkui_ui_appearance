/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
 */

#include "sunrise_sunset_calc.h"

#include <chrono>
#include <iomanip>
#include <sstream>

namespace {

// ---- Angle conversion ----
constexpr double DEG2RAD = M_PI / 180.0;
constexpr double RAD2DEG = 180.0 / M_PI;

// ---- Zenith angle ----
/** Official sunrise and sunset zenith: 90°50', including refraction and the solar radius. */
constexpr double ZENITH = 90.833;

// ---- Time conversion ----
constexpr double MINS_PER_HALF_DAY = 720.0;
constexpr double MINS_PER_DEGREE_LON = 4.0;
constexpr double HOURS_PER_DAY = 24.0;

// ---- Julian calendar ----
constexpr double J2000_JD = 2451545.0;
constexpr double DAYS_PER_JULIAN_CENT = 36525.0;
constexpr double JULIAN_DAYS_PER_YEAR = 365.25;
constexpr double JULIAN_MONTH_COEFF = 30.6001;
constexpr double JULIAN_YEAR_OFFSET = 4716.0;
constexpr double JULIAN_DAY_OFFSET = 1524.5;

// ---- Solar geometric mean longitude coefficients ----
constexpr double MEAN_LONG_A = 280.46646;
constexpr double MEAN_LONG_B = 36000.76983;
constexpr double MEAN_LONG_C = 0.0003032;

// ---- Solar mean anomaly coefficients ----
constexpr double MEAN_ANOM_A = 357.52911;
constexpr double MEAN_ANOM_B = 35999.05029;
constexpr double MEAN_ANOM_C = 0.0001537;

// ---- Earth orbital eccentricity coefficients ----
constexpr double ECC_A = 0.016708634;
constexpr double ECC_B = 0.000042037;
constexpr double ECC_C = 0.0000001267;

// ---- Solar equation of center coefficients ----
constexpr double EQC_A0 = 1.914602;
constexpr double EQC_A1 = 0.004817;
constexpr double EQC_A2 = 0.000014;
constexpr double EQC_B0 = 0.019993;
constexpr double EQC_B1 = 0.000101;
constexpr double EQC_C0 = 0.000289;

// ---- Solar apparent longitude correction coefficients ----
constexpr double OMEGA_A = 125.04;
constexpr double OMEGA_B = 1934.136;
constexpr double ABERR_A = 0.00569;
constexpr double ABERR_B = 0.00478;

// ---- Obliquity coefficients ----
constexpr double OBLIQ_SEC_A = 21.448;
constexpr double OBLIQ_SEC_B = 46.815;
constexpr double OBLIQ_SEC_C = 0.00059;
constexpr double OBLIQ_SEC_D = 0.001813;
constexpr double OBLIQ_DEGREES = 23.0;
constexpr double OBLIQ_MINUTES = 26.0;
constexpr double OBLIQ_CORR_COEFF = 0.00256;

// ---- Equation of time coefficients ----
constexpr double EOT_RAD2MIN = 4.0;
constexpr double EOT_E_SINM_COEFF = 2.0;
constexpr double EOT_EY_COEFF = 4.0;
constexpr double EOT_Y2_COEFF = 0.5;
constexpr double EOT_E2_COEFF = 1.25;

// ---- Special values ----
constexpr double POLAR_MARK = -1.0;
constexpr double UTC_NOON_HOUR = 12.0;

/**
 * Calculates the Julian day.
 */
double CalcJulianDay(int year, int month, int day, double hour)
{
    if (month <= 2) {
        year -= 1;
        month += 12;
    }
    int centuryDiv = year / 100;
    int gregorianCorr = 2 - centuryDiv + centuryDiv / 4;
    return floor(JULIAN_DAYS_PER_YEAR * (year + JULIAN_YEAR_OFFSET)) +
        floor(JULIAN_MONTH_COEFF * (month + 1)) +
        day + hour / HOURS_PER_DAY + gregorianCorr - JULIAN_DAY_OFFSET;
}

/**
 * Converts a Julian day to Julian centuries since J2000.0.
 */
double CalcJulianCent(double jd)
{
    return (jd - J2000_JD) / DAYS_PER_JULIAN_CENT;
}

/**
 * Calculates the solar geometric mean longitude in degrees.
 */
double CalcMeanLong(double t)
{
    double meanLong = MEAN_LONG_A + MEAN_LONG_B * t + MEAN_LONG_C * t * t;
    return fmod(fmod(meanLong, 360.0) + 360.0, 360.0);
}

/**
 * Calculates the solar geometric mean anomaly in degrees.
 */
double CalcMeanAnomaly(double t)
{
    return MEAN_ANOM_A + MEAN_ANOM_B * t - MEAN_ANOM_C * t * t;
}

/**
 * Calculates the eccentricity of Earth's orbit.
 */
double CalcEccentricity(double t)
{
    return ECC_A - ECC_B * t - ECC_C * t * t;
}

/**
 * Calculates the solar equation of center.
 */
double CalcEqOfCenter(double t, double m)
{
    double mRad = m * DEG2RAD;
    return (EQC_A0 - EQC_A1 * t - EQC_A2 * t * t) * sin(mRad) +
        (EQC_B0 - EQC_B1 * t) * sin(2.0 * mRad) +
        EQC_C0 * sin(3.0 * mRad);
}

/**
 * Calculates the solar apparent longitude in degrees, including nutation and aberration corrections.
 */
double CalcAppLong(double t, double lTrue)
{
    double omega = OMEGA_A - OMEGA_B * t;
    return lTrue - ABERR_A - ABERR_B * sin(omega * DEG2RAD);
}

/**
 * Calculates the mean obliquity of the ecliptic in degrees.
 */
double CalcMeanObliq(double t)
{
    double sec = OBLIQ_SEC_A -
        t * (OBLIQ_SEC_B + t * (OBLIQ_SEC_C - t * OBLIQ_SEC_D));
    return OBLIQ_DEGREES +
        (OBLIQ_MINUTES + sec / DarkModeConstants::SECS_PER_MIN) / DarkModeConstants::SECS_PER_MIN;
}

/**
 * Calculates the corrected obliquity in degrees, including nutation correction.
 */
double CalcObliqCorr(double t, double meanObliq)
{
    double omega = OMEGA_A - OMEGA_B * t;
    return meanObliq + OBLIQ_CORR_COEFF * cos(omega * DEG2RAD);
}

/**
 * Calculates the solar declination in degrees.
 */
double CalcDeclination(double obliqCorr, double appLong)
{
    return asin(sin(obliqCorr * DEG2RAD) * sin(appLong * DEG2RAD)) * RAD2DEG;
}

/**
 * Calculates the equation of time in minutes.
 */
double CalcEqOfTime(double meanLong, double meanAnomaly, double e, double obliqCorr)
{
    double y = tan(obliqCorr * DEG2RAD / 2.0) * tan(obliqCorr * DEG2RAD / 2.0);
    double meanLongRad = meanLong * DEG2RAD;
    double meanAnomalyRad = meanAnomaly * DEG2RAD;
    double sin2MeanLong = sin(2.0 * meanLongRad);
    double sinMeanAnomaly = sin(meanAnomalyRad);
    double cos2MeanLong = cos(2.0 * meanLongRad);
    double sin4MeanLong = sin(4.0 * meanLongRad);
    double sin2MeanAnomaly = sin(2.0 * meanAnomalyRad);
    double eqTime = EOT_RAD2MIN * RAD2DEG *
        (y * sin2MeanLong - EOT_E_SINM_COEFF * e * sinMeanAnomaly +
        EOT_EY_COEFF * e * y * sinMeanAnomaly * cos2MeanLong -
        EOT_Y2_COEFF * y * y * sin4MeanLong -
        EOT_E2_COEFF * e * e * sin2MeanAnomaly);
    return eqTime;
}

/**
 * Calculates the sunrise or sunset hour angle in degrees.
 * @returns The hour angle in degrees, or POLAR_MARK for polar day or polar night.
 */
double CalcHourAngle(double lat, double dec)
{
    double cosH =
        (cos(ZENITH * DEG2RAD) - sin(lat * DEG2RAD) * sin(dec * DEG2RAD)) /
        (cos(lat * DEG2RAD) * cos(dec * DEG2RAD));
    if (cosH > 1.0) {
        return POLAR_MARK; // Polar night.
    }
    if (cosH < -1.0) {
        return POLAR_MARK; // Polar day.
    }
    return acos(cosH) * RAD2DEG;
}

/**
 * Calculates sunrise or sunset in UTC minutes.
 * @returns UTC minutes, or POLAR_MARK for polar day or polar night.
 */
double CalcSunTimeMin(double lat, double lon, double jd, bool isSunrise)
{
    double t = CalcJulianCent(jd);
    double meanLong = CalcMeanLong(t);
    double meanAnomaly = CalcMeanAnomaly(t);
    double e = CalcEccentricity(t);
    double eqOfCenter = CalcEqOfCenter(t, meanAnomaly);
    double lTrue = meanLong + eqOfCenter;
    double appLong = CalcAppLong(t, lTrue);
    double meanObliq = CalcMeanObliq(t);
    double obliqCorr = CalcObliqCorr(t, meanObliq);
    double dec = CalcDeclination(obliqCorr, appLong);
    double eqTime = CalcEqOfTime(meanLong, meanAnomaly, e, obliqCorr);

    double ha = CalcHourAngle(lat, dec);
    if (ha == POLAR_MARK) {
        return POLAR_MARK;
    }

    // Solar noon in UTC minutes.
    double solarNoon = MINS_PER_HALF_DAY - MINS_PER_DEGREE_LON * lon - eqTime;

    if (isSunrise) {
        return solarNoon - ha * MINS_PER_DEGREE_LON;
    } else {
        return solarNoon + ha * MINS_PER_DEGREE_LON;
    }
}

} // anonymous namespace

namespace SunriseSunsetUtils {

SunriseSunsetResult CalculateSunriseSunset(double lat, double lon, int64_t timestampMs)
{
    time_t rawTime = static_cast<time_t>(timestampMs / DarkModeConstants::MS_PER_SEC);
    struct tm utcTm;
    gmtime_r(&rawTime, &utcTm);

    // Use UTC noon as the base time for the Julian day.
    double jd = CalcJulianDay(
        utcTm.tm_year + 1900, utcTm.tm_mon + 1, utcTm.tm_mday, UTC_NOON_HOUR);

    // First iteration.
    double sunriseMin = CalcSunTimeMin(lat, lon, jd, true);
    double sunsetMin = CalcSunTimeMin(lat, lon, jd, false);

    // Refine the Julian day with the first result for a more accurate second iteration.
    if (sunriseMin != POLAR_MARK) {
        double jdRise = jd + sunriseMin / DarkModeConstants::MINS_PER_DAY;
        sunriseMin = CalcSunTimeMin(lat, lon, jdRise, true);
    }
    if (sunsetMin != POLAR_MARK) {
        double jdSet = jd + sunsetMin / DarkModeConstants::MINS_PER_DAY;
        sunsetMin = CalcSunTimeMin(lat, lon, jdSet, false);
    }

    // Convert the results to UTC seconds.
    double sunriseSec = POLAR_MARK;
    if (sunriseMin != POLAR_MARK) {
        double adjustedMin = fmod(fmod(sunriseMin, static_cast<double>(DarkModeConstants::MINS_PER_DAY)) +
            DarkModeConstants::MINS_PER_DAY, DarkModeConstants::MINS_PER_DAY);
        sunriseSec = round(adjustedMin * DarkModeConstants::SECS_PER_MIN);
    }

    double sunsetSec = POLAR_MARK;
    if (sunsetMin != POLAR_MARK) {
        double adjustedMin = fmod(fmod(sunsetMin, static_cast<double>(DarkModeConstants::MINS_PER_DAY)) +
            DarkModeConstants::MINS_PER_DAY, DarkModeConstants::MINS_PER_DAY);
        sunsetSec = round(adjustedMin * DarkModeConstants::SECS_PER_MIN);
    }

    return { sunriseSec, sunsetSec };
}

std::string FormatTimeFromSec(double utcSec, int timezoneOffset)
{
    if (utcSec == POLAR_MARK) {
        return "---";
    }
    // Apply the time zone offset and normalize to [0, SECS_PER_DAY).
    double localSec = utcSec - timezoneOffset * DarkModeConstants::SECS_PER_MIN;
    localSec = fmod(fmod(localSec, static_cast<double>(DarkModeConstants::SECS_PER_DAY)) +
        DarkModeConstants::SECS_PER_DAY, DarkModeConstants::SECS_PER_DAY);
    // Round to the nearest minute.
    int32_t totalMin = static_cast<int32_t>(round(localSec / DarkModeConstants::SECS_PER_MIN));
    int32_t hours = totalMin / DarkModeConstants::MINS_PER_HOUR;
    int32_t minutes = totalMin % DarkModeConstants::MINS_PER_HOUR;

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours << ":"
        << std::setw(2) << std::setfill('0') << minutes;
    return oss.str();
}

} // namespace SunriseSunsetUtils

// ============================================================
// SunriseSunsetInfo implementation
// ============================================================

SunriseSunsetInfo::SunriseSunsetInfo(double lat, double lon, int64_t timestampMs)
{
    // Obtain the offset from the system time zone.
    time_t rawTime = static_cast<time_t>(timestampMs / DarkModeConstants::MS_PER_SEC);
    struct tm localTm;
    localtime_r(&rawTime, &localTm);
    timezoneOffset_ = -localTm.tm_gmtoff / DarkModeConstants::SECS_PER_MIN;

    auto result = SunriseSunsetUtils::CalculateSunriseSunset(lat, lon, timestampMs);
    sunrise_ = result.sunrise;
    sunset_ = result.sunset;

    if (sunrise_ == POLAR_MARK && sunset_ == POLAR_MARK) {
        condition_ = SunCondition::POLAR_NIGHT;
    } else if (sunset_ == POLAR_MARK) {
        condition_ = SunCondition::POLAR_DAY;
    } else {
        condition_ = SunCondition::NORMAL;
    }
}

int32_t SunriseSunsetInfo::ToLocalMinutes(double utcSec) const
{
    double localSec = utcSec - timezoneOffset_ * DarkModeConstants::SECS_PER_MIN;
    localSec = fmod(fmod(localSec, static_cast<double>(DarkModeConstants::SECS_PER_DAY)) +
        DarkModeConstants::SECS_PER_DAY, DarkModeConstants::SECS_PER_DAY);
    return static_cast<int32_t>(round(localSec / DarkModeConstants::SECS_PER_MIN));
}

bool SunriseSunsetInfo::IsDaytime(int64_t timestampMs) const
{
    if (condition_ == SunCondition::POLAR_DAY) {
        return true;
    }
    if (condition_ == SunCondition::POLAR_NIGHT) {
        return false;
    }
    time_t rawTime = static_cast<time_t>(timestampMs / DarkModeConstants::MS_PER_SEC);
    struct tm localTm;
    localtime_r(&rawTime, &localTm);
    int32_t currentMinutes = localTm.tm_hour * DarkModeConstants::MINS_PER_HOUR + localTm.tm_min;
    int32_t sunriseMin = ToLocalMinutes(sunrise_);
    int32_t sunsetMin = ToLocalMinutes(sunset_);
    if (sunriseMin < sunsetMin) {
        return currentMinutes >= sunriseMin && currentMinutes < sunsetMin;
    } else {
        return currentMinutes >= sunriseMin || currentMinutes < sunsetMin;
    }
}

bool SunriseSunsetInfo::IsNighttime(int64_t timestampMs) const
{
    if (condition_ == SunCondition::POLAR_NIGHT) {
        return true;
    }
    if (condition_ == SunCondition::POLAR_DAY) {
        return false;
    }
    return !IsDaytime(timestampMs);
}

bool SunriseSunsetInfo::IsPolarDay() const
{
    return condition_ == SunCondition::POLAR_DAY;
}

bool SunriseSunsetInfo::IsPolarNight() const
{
    return condition_ == SunCondition::POLAR_NIGHT;
}

int32_t SunriseSunsetInfo::GetDayLength() const
{
    if (condition_ != SunCondition::NORMAL) {
        return -1;
    }
    return (ToLocalMinutes(sunset_) - ToLocalMinutes(sunrise_) + DarkModeConstants::MINS_PER_DAY) %
        DarkModeConstants::MINS_PER_DAY;
}

std::string SunriseSunsetInfo::GetSunsetStr() const
{
    if (condition_ != SunCondition::NORMAL) {
        return "";
    }
    return std::to_string(ToLocalMinutes(sunset_));
}

std::string SunriseSunsetInfo::GetSunriseStr() const
{
    if (condition_ != SunCondition::NORMAL) {
        return "";
    }
    int32_t sunriseMinutes = ToLocalMinutes(sunrise_);
    int32_t sunsetMinutes = ToLocalMinutes(sunset_);
    // Add one day when sunrise is earlier than sunset so the result denotes the next day.
    int32_t totalMinutes = (sunriseMinutes < sunsetMinutes) ?
        sunriseMinutes + DarkModeConstants::MINS_PER_DAY : sunriseMinutes;
    return std::to_string(totalMinutes);
}
