/*
 * File:   utility.h
 * Author: German Gambon
 *
 * Created on 23 de abril de 2016, 11:42
 *
 * Notes:
 * - These functions are useful for an 8-bit microcontroller without FPU,
 *   since the way they are implemented run faster than using math.
 */
#ifndef UTILITY_H
#define UTILITY_H

#include <cstdint>
#include <cstdio>
#include <cstring>

static const char *off_limits = "OffLimits";

/**
 * Using this because I cannot use the abs function for float's of cmath due to
 * a bug in the gcc version of minsys.
 */
template<typename T>
static T abs(T value) { return value > 0.0 ? value : -value; }

/**
 * Returns the result of 10^exp_, with exp_ >= 0. Returns signed since the
 * standard says that an operation involving signed and unsigned int gets
 * implicitly converted to unsigned, therefore returning signed does not
 * unexpectedly alter an operation with unsigned int.
 *
 * @param exp_ exponent >= 0
 *
 * @return result of 1E(exp_)
 */
inline int32_t base10pow(uint8_t exp_) {
    switch (exp_) {
    case 0:  return 1;
    case 1:  return 10;
    case 2:  return 100;
    case 3:  return 1000;
    case 4:  return 10000;
    case 5:  return 100000;
    case 6:  return 1000000;
    case 7:  return 10000000;
    case 8:  return 100000000;
    default: return 1000000000;
    }
}

/**
 * Returns the float/double result of 10^exp_ in the range [1E-15, 1E15]
 */
template<typename T>
inline T fbase10pow(int8_t exp_) {
    if (exp_ >= 0) {
        switch (exp_) {
        case 0:  return 1E0;
        case 1:  return 1E1;
        case 2:  return 1E2;
        case 3:  return 1E3;
        case 4:  return 1E4;
        case 5:  return 1E5;
        case 6:  return 1E6;
        case 7:  return 1E7;
        case 8:  return 1E8;
        case 9:  return 1E9;
        case 10: return 1E10;
        case 11: return 1E11;
        case 12: return 1E12;
        case 13: return 1E13;
        case 14: return 1E14;
        default: return 1E15;
        }
    } else {
        switch (exp_) {
        case -1:  return 1E-1;
        case -2:  return 1E-2;
        case -3:  return 1E-3;
        case -4:  return 1E-4;
        case -5:  return 1E-5;
        case -6:  return 1E-6;
        case -7:  return 1E-7;
        case -8:  return 1E-8;
        case -9:  return 1E-9;
        case -10: return 1E-10;
        case -11: return 1E-11;
        case -12: return 1E-12;
        case -13: return 1E-13;
        case -14: return 1E-14;
        default:  return 1E-15;
        }
    }
}

/**
 * Return the exponent for scientific notation of an integer value, i.e.
 * floor(log10(value)). It is equivalent to the number of digits for integer
 * values.
 */
inline int8_t sciexp(int32_t value) {
    if (value < 0)
        value = -value;
    if      (value < 10)         return 1;
    else if (value < 100)        return 2;
    else if (value < 1000)       return 3;
    else if (value < 10000)      return 4;
    else if (value < 100000)     return 5;
    else if (value < 1000000)    return 6;
    else if (value < 10000000)   return 7;
    else if (value < 100000000)  return 8;
    else if (value < 1000000000) return 9;
                                 return 10;
}

/**
 * Return the exponent for scientific notation of a float value, i.e.
 * floor(log10(value)).
 */
template<typename T>
inline int8_t fsciexp(T value) {
    if (value < 0.0)
        value = -value;
    if (value >= 1.0) {
        if      (value < 1E1)  return 0;
        else if (value < 1E2)  return 1;
        else if (value < 1E3)  return 2;
        else if (value < 1E4)  return 3;
        else if (value < 1E5)  return 4;
        else if (value < 1E6)  return 5;
        else if (value < 1E7)  return 6;
        else if (value < 1E8)  return 7;
        else if (value < 1E9)  return 8;
        else if (value < 1E10) return 9;
        else if (value < 1E11) return 10;
        else if (value < 1E12) return 11;
        else if (value < 1E13) return 12;
        else if (value < 1E14) return 13;
        else if (value < 1E15) return 14;
                               return 15;
    } else {
        if      (value == 0.0)  return 0;
        else if (value > 1E-1)  return -1;
        else if (value > 1E-2)  return -2;
        else if (value > 1E-3)  return -3;
        else if (value > 1E-4)  return -4;
        else if (value > 1E-5)  return -5;
        else if (value > 1E-6)  return -6;
        else if (value > 1E-7)  return -7;
        else if (value > 1E-8)  return -8;
        else if (value > 1E-9)  return -9;
        else if (value > 1E-10) return -10;
        else if (value > 1E-11) return -11;
        else if (value > 1E-12) return -12;
        else if (value > 1E-13) return -13;
        else if (value > 1E-14) return -14;
                                return -15;
    }
}

/**
 * Returns the string representation of a float number in engineering notation.
 * Does the most basic checks on the inputs. It is used by other functions as
 * the basic building block and is static for that reason.
 *
 * @param buf destination buffer
 * @param sz sizeof(buffer)
 * @param exp out value with the exponent of the number, which is a multiple of 3
 * @param value float/double value to convert to string. Range is (1e-16, 1e16),
 * otherwise a short error string is copied into the buffer.
 * @param precision number of digits to print. Minimum is 3.
 *
 * @return the exponent, which is a multiple of 3.
 */
template<typename T>
static uint8_t ftoaEngBase(char *buf, size_t sz, int8_t *exp, T value, int8_t precision = 3) {
    int8_t exp_;
    bool negative;
    int16_t intp; // int part
    int16_t decp; // decimal part

	if (precision < 3)
        precision = 3;
    if (value < 0.0) {
        value = -value;
        negative = true;
    } else {
        negative = false;
    }
    if (value >= 1.0) {
        if (value < 1E3) exp_ = 0;
        else if (value < 1E6) exp_ = 3;
        else if (value < 1E9) exp_ = 6;
        else if (value < 1E12) exp_ = 9;
        else if (value < 1E15) exp_ = 12;
        else {
            return 0;
        }
        value = value / (T) base10pow(exp_); // move comma left
    } else {
        if (value > 1E-3) exp_ = -3;
        else if (value > 1E-6) exp_ = -6;
        else if (value > 1E-9) exp_ = -9;
        else if (value > 1E-12) exp_ = -12;
        else if (value > 1E-15) exp_ = -15;
        else {
            return 0;
        }
        value = value * (T) base10pow(abs(exp_)); // move comma right
    }
    intp = (int16_t) value;
	int8_t rest = precision - sciexp(intp);
	uint8_t i;
	if (negative)
		i = sprintf(buf, "-%d", intp);
	else
		i = sprintf(buf, "%d", intp);
	if (rest > 0) {
		decp = (value - (T) intp) * (T) base10pow(rest); // move comma right
		char format[7];
		sprintf(format, ".%%0%dd", rest);
		i += sprintf(&buf[i], format, decp);
	}
	*exp = exp_;
    return i; // chars written
}

/**
 * Get the string representation of a float number in engineering notation and
 * with the exponent in format 'eN'. Buffer size is checked first. Returns the
 * number of chars written, and value of 0 means a failure.
 *
 * @param buf destination buffer
 * @param sz sizeof(buffer)
 * @param exp out value with the exponent of the number, which is a multiple of 3
 * @param value float/double value to convert to string. Range is (1e-16, 1e16),
 * otherwise a short error string is copied into the buffer.
 * @param precision number of digits to print. Minimum is 3.
 *
 * @return the number of chars written
 */
template<typename T>
inline uint8_t ftoaEngExp(char *buf, size_t sz, int8_t *exp, T value, int8_t precision = 3) {
    const char *err = off_limits;
	if (sz < precision + 7) { // precision + '-', '.', '\0', 'E-XX'
		buf[0] = '\0';
		return 0;
	}
	uint8_t i = ftoaEngBase<T>(buf, sz, exp, value, precision);
	if (i != 0) {
		i += sprintf(&buf[i], "e%d", *exp);
	} else {
		strncpy(buf, err, sz);
	}
	return i;
}

/**
 * Get the string representation of a float number in engineering notation.
 * Buffer size is checked first. Returns the number of chars written, and a
 * value of 0 means a failure.
 *
 * @param buf destination buffer
 * @param sz sizeof(buffer)
 * @param exp out value with the exponent of the number, which is a multiple of 3
 * @param value float/double value to convert to string. Range is (1e-16, 1e16),
 * otherwise a short error string is copied into the buffer.
 * @param precision number of digits to print. Minimum is 3.
 *
 * @return the number of chars written
 */
template<typename T>
inline uint8_t ftoaEng(char *buf, size_t sz, int8_t *exp, T value, int8_t precision = 3) {
    const char *err = off_limits;
	if (sz < precision + 3) { // precision + '-', '.', '\0'
		buf[0] = '\0';
		return 0;
	}
	uint8_t i = ftoaEngBase<T>(buf, sz, exp, value, precision);
	if (i == 0)
		strncpy(buf, err, sz);
	return i;
}

/**
 * Returns the string representation of a float number in fixed-point notation.
 *
 * @param buf destination buffer
 * @param sz sizeof(buffer)
 * @param value float/double value to convert to string
 * @param digits number of digits to print
 * @param exp_ comma position. Positive values move the comma left. Range is
 * (1e-16, 1e16), otherwise a short error string is copied into the buffer.
 */
template<typename T>
inline uint8_t ftoaFix(char *buf, uint8_t sz, T value, uint8_t precision) {
    const char *err = off_limits;
    int32_t intp; // int part
    int32_t decp; // decimal part

    intp = (int32_t) value;
	decp = (value - (float) intp) * fbase10pow<T>(precision); // move comma right
	uint8_t intplen = sciexp(intp);
	uint8_t decplen = sciexp(decp);
	if (intplen + decplen + 3 > sz ) { // int len + dec len + '.' + '-' + '\0'
		strncpy(buf, err, sz);
		return 0;
	}
	uint8_t i;
	char format[8];
	sprintf(format, ".%%0%dld", precision);
	if (decp < 0) {
		i = sprintf(buf, "-%d", -intp); // so that the minus gets written even if intp = 0
		sprintf(&buf[i], format, -decp);
	}
	else {
		i = sprintf(buf, "%d", intp);
		sprintf(&buf[i], format, decp);
	}
	return i;
}

/**
 * Returns the string representation of a float number in pseudo fixed-point notation.
 *
 * @param buf destination buffer
 * @param sz sizeof(buffer)
 * @param value float/double value to convert to string
 * @param digits number of digits to print
 * @param exp_ comma position. Positive values move the comma left. Range is
 * (1e-16, 1e16), otherwise a short error string is copied into the buffer.
 */
template<typename T>
inline uint8_t ftoaFix2(char *buf, uint8_t sz, T value, uint8_t digits) {
    const char *err = off_limits;
    int32_t intp; // int part
    int32_t decp; // decimal part
	uint8_t i;

    intp = (int32_t) value;
	uint8_t intplen = sciexp(intp);
    uint8_t aux = digits - intplen; // num decimals = digits - integers
    if (aux < 0) { // no decimal part
		if (intplen >= sz) {
			strncpy(buf, err, sz);
			return 0;
		}
		i = sprintf(buf, "%ld", intp);
	}
    else {
        decp = (value - (float) intp) * fbase10pow<T>(aux); // move comma right
		uint8_t decplen = sciexp(decp);
		if (intplen + decplen + 2 >= sz ) { // int len + dec len + '.' + '-'
			strncpy(buf, err, sz);
			return;
		}
		char format[9];
		sprintf(format, "-%%ld.%%0%dd", aux);
		if (decp < 0)
			i = sprintf(buf, format, -intp, -decp);
		else
			i = sprintf(buf, &format[1], intp, decp);
	}
	return i;
}

/**
 * Returns the string representation of a float number in scientific notation.
 * Does the most basic checks on the inputs. It is used by other functions as
 * the basic building block and is static for that reason.
 *
 * @param buf destination buffer
 * @param sz sizeof(buffer)
 * @param exp out value with the exponent of the number, which is a multiple of 3
 * @param value float/double value to convert to string. Range is (1e-16, 1e16),
 * otherwise a short error string is copied into the buffer.
 * @param precision number of digits to print. Minimum is 3.
 *
 * @return the exponent, which is a multiple of 3.
 */
template<typename T>
static uint8_t ftoaSciBase(char *buf, size_t sz, int8_t *exp, T value, int8_t precision) {
    bool negative;
    int8_t intp; // int part
    int32_t decp; // decimal part

    if (value < 0.0) {
        value = -value;
        negative = true;
    } else {
        negative = false;
    }
    if (value >= 1E16) {
        return 0;
    } else if (value <= 1E-16) {
        return 0;
    }
    int8_t exp_ = fsciexp(value);
    value = value * fbase10pow<T>(-exp_); // move comma an amount `exp_`
    intp = (int8_t) value;
    int8_t rest = precision - 1; // number of decimals
	uint8_t i;
	if (negative)
		i = sprintf(buf, "-%d", intp);
	else
		i = sprintf(buf, "%d", intp);
	if (rest > 0) {
		decp = (value - (T) intp) * (T) base10pow(rest); // move comma right
		char format[7];
		sprintf(format, ".%%0%dd", rest);
		i += sprintf(&buf[i], format, decp);
	}
	*exp = exp_;
    return i; // chars written
}

/**
 * Get the string representation of a float number in scientific notation and
 * with the exponent in format 'eN'. Buffer size is checked first. Returns the
 * number of chars written, and value of 0 means a failure.
 *
 * @param buf destination buffer
 * @param sz sizeof(buffer)
 * @param exp out value with the exponent of the number, which is a multiple of 3
 * @param value float/double value to convert to string. Range is (1e-16, 1e16),
 * otherwise a short error string is copied into the buffer.
 * @param precision number of digits to print. Minimum is 3.
 *
 * @return the number of chars written
 */
template<typename T>
inline uint8_t ftoaSciExp(char *buf, size_t sz, int8_t *exp, T value, int8_t precision) {
    const char *err = off_limits;
	if (sz < precision + 7) { // precision + '-', '.', '\0', 'E-XX'
		buf[0] = '\0';
		return 0;
	}
	uint8_t i = ftoaSciBase<T>(buf, sz, exp, value, precision);
	if (i != 0) {
		i += sprintf(&buf[i], "e%d", *exp);
	} else {
		strncpy(buf, err, sz);
	}
	return i;
}

/**
 * Get the string representation of a float number in scientific notation.
 * Buffer size is checked first. Returns the number of chars written, and a
 * value of 0 means a failure.
 *
 * @param buf destination buffer
 * @param sz sizeof(buffer)
 * @param exp out value with the exponent of the number, which is a multiple of 3
 * @param value float/double value to convert to string. Range is (1e-16, 1e16),
 * otherwise a short error string is copied into the buffer.
 * @param precision number of digits to print. Minimum is 3.
 *
 * @return the number of chars written
 */
template<typename T>
inline uint8_t ftoaSci(char *buf, size_t sz, int8_t *exp, T value, int8_t precision) {
    const char *err = off_limits;
	if (sz < precision + 3) { // precision + '-', '.', '\0'
		buf[0] = '\0';
		return 0;
	}
	uint8_t i = ftoaSciBase<T>(buf, sz, exp, value, precision);
	if (i == 0)
		strncpy(buf, err, sz);
	return i;
}

#endif /* UTILITY_H */
