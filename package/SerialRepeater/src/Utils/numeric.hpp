#ifndef NUMERIC_HPP
#define NUMERIC_HPP

#include <cmath>
#include <stdint.h>
#include <stdio.h>
#include <glib-2.0/glib.h>

static inline uint32_t to_precision(double scale)
{
    if (scale == 1.0) return 0;
    if (scale == 0.1) return 1;
    if (scale == 0.01) return 2;
    if (scale == 0.001) return 3;
    if (scale == 0.0001) return 4;
    if (scale == 0.00001) return 5;
    if (scale == 0.000001) return 6;
    if (scale == 0.0000001) return 7;
    if (scale == 0.00000001) return 8;
    if (scale == 0.000000001) return 9;
    if (scale == 0.0000000001) return 10;
    if (scale == 0.00000000001) return 11;
    if (scale == 0.000000000001) return 12;

    return 0;
}

static inline double to_scale(uint32_t precision)
{
    switch (precision) {
    case 0:
        return 1;
    case 1:
        return 0.1;
    case 2:
        return 0.01;
    case 3:
        return 0.001;
    case 4:
        return 0.0001;
    case 5:
        return 0.00001;
    case 6:
        return 0.000001;
    case 7:
        return 0.0000001;
    case 8:
        return 0.00000001;
    case 9:
        return 0.000000001;
    case 10:
        return 0.0000000001;
    case 11:
        return 0.00000000001;
    case 12:
        return 0.000000000001;
    default:
        return 1;
    }
}

static const double POW10_LUT[13] =
{
    1.0,
    10.0,
    100.0,
    1000.0,
    10000.0,
    100000.0,
    1000000.0,
    10000000.0,
    100000000.0,
    1000000000.0,
    10000000000.0,
    100000000000.0,
    1000000000000.0
};

static inline double lf_truncate(double val, uint32_t precision, bool half_adjust)
{
    double integer = floor(val);
    double decimal = val - integer;
    double factor = POW10_LUT[precision]; // pow(10, precision);

    decimal *= factor;
    decimal = half_adjust ? floor(decimal + 0.5) : floor(decimal);
    decimal /= factor;

    return (integer + decimal);
}

static inline char *lf_to_str(double val, uint32_t precision, char *buf)
{
    if (!buf) return NULL;

    const char *fmt = "%lf";
    switch (precision) {
    case 0:
        fmt = "%.0lf"; break;
    case 1:
        fmt = "%.1lf"; break;
    case 2:
        fmt = "%.2lf"; break;
    case 3:
        fmt = "%.3lf"; break;
    case 4:
        fmt = "%.4lf"; break;
    case 5:
        fmt = "%.5lf"; break;
    case 6:
        fmt = "%.6lf"; break;
    case 7:
        fmt = "%.7lf"; break;
    case 8:
        fmt = "%.8lf"; break;
    case 9:
        fmt = "%.9lf"; break;
    case 10:
        fmt = "%.10lf"; break;
    case 11:
        fmt = "%.11lf"; break;
    case 12:
        fmt = "%.12lf"; break;
    default:
        fmt = "%lf"; break;
    }

    sprintf(buf, fmt, val);
    return buf;
}

#endif // NUMERIC_HPP
