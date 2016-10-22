/*
 * File:   utility_test.cpp
 * Author: German Gambon
 *
 * Created on 23 de abril de 2016, 17:53
 */

#include <cstdlib>
#include <cstdint>
#include <iostream>
#include "utility.h"

using namespace std;

void test_ftoaEng() {
    char buf[16];
    int8_t exp;
    float value;
    int digits = 5;

    cout << "ftoaEng():" << endl << endl;

    value = 1.02030405E4;
    cout << "value > 1.0" << endl;
    cout << value << endl;
    ftoaEng(buf, sizeof(buf), &exp, value, digits);
    cout << buf << 'E' << (int)exp << endl << endl;

    value = 1.02030405E-4;
    cout << "1.0 > value > 0.0" << endl;
    cout << value << endl;
    ftoaEng(buf, sizeof(buf), &exp, value, digits);
    cout << buf << 'E' << (int)exp << endl << endl;

    value = -1.02030405E-4;
    cout << "0.0 > value > -1.0" << endl;
    cout << value << endl;
    ftoaEng(buf, sizeof(buf), &exp, value, digits);
    cout << buf << 'E' << (int)exp << endl << endl;

    value = -1.02030405E4;
    cout << "-1.0 > value" << endl;
    cout << value << endl;
    ftoaEng(buf, sizeof(buf), &exp, value, digits);
    cout << buf << 'E' << (int)exp << endl << endl;

    cout << "Short buffer test:" << endl;
    ftoaEng(buf, sizeof(buf), &exp, value, digits);
    cout << "Result: " << buf << endl << endl;
}

void test_ftoaEngExp() {
    char buf[16];
    int8_t exp;
    float value;
    int digits = 5;

    cout << "ftoaEngExp():" << endl << endl;

    value = 1.02030405E4;
    cout << "value > 1.0" << endl;
    cout << value << endl;
    ftoaEngExp(buf, sizeof(buf), &exp, value, digits);
    cout << buf << endl << endl;

    value = 1.02030405E-4;
    cout << "1.0 > value > 0.0" << endl;
    cout << value << endl;
    ftoaEngExp(buf, sizeof(buf), &exp, value, digits);
    cout << buf << endl << endl;

    value = -1.02030405E-4;
    cout << "0.0 > value > -1.0" << endl;
    cout << value << endl;
    ftoaEngExp(buf, sizeof(buf), &exp, value, digits);
    cout << buf << endl << endl;

    value = -1.02030405E4;
    cout << "-1.0 > value" << endl;
    cout << value << endl;
    ftoaEngExp(buf, sizeof(buf), &exp, value, digits);
    cout << buf << endl << endl;

    cout << "Short buffer test:" << endl;
    ftoaEng(buf, sizeof(buf), &exp, value, digits);
    cout << "Result: " << buf << endl << endl;
}

void test_ftoaFix() {
    char buf[16];
    float value;
    int digits = 6;

    cout << "ftoaFix():" << endl << endl;

    value = 1.2345E2;
    cout << "value > 1.0" << endl;
    cout << value << endl;
    ftoaFix(buf, sizeof(buf), value, digits);
    cout << buf << endl << endl;

    value = 1.2345E-2;
    cout << "1.0 > value > 0.0" << endl;
    cout << value << endl;
    ftoaFix(buf, sizeof(buf), value, digits);
    cout << buf << endl << endl;

    value = -1.2345E-2;
    cout << "0.0 > value > -1.0" << endl;
    cout << value << endl;
    ftoaFix(buf, sizeof(buf), value, digits);
    cout << buf << endl << endl;

    value = -1.2345E2;
    cout << "-1.0 > value" << endl;
    cout << value << endl;
    ftoaFix(buf, sizeof(buf), value, digits);
    cout << buf << endl << endl;
}

void test_ftoaSci() {
    char buf[16];
    int8_t exp;
    float value;
    int digits = 4;

    cout << "ftoaSci():" << endl << endl;

    value = 1.2345E5;
    cout << "value > 1.0" << endl;
    cout << value << endl;
    ftoaSci(buf, sizeof(buf), &exp, value, digits);
    cout << buf << 'E' << (int)exp << endl << endl;

    value = 1.2345E-5;
    cout << "1.0 > value > 0.0" << endl;
    cout << value << endl;
    ftoaSci(buf, sizeof(buf), &exp, value, digits);
    cout << buf << 'E' << (int)exp << endl << endl;

    value = -1.2345E-5;
    cout << "0.0 > value > -1.0" << endl;
    cout << value << endl;
    ftoaSci(buf, sizeof(buf), &exp, value, digits);
    cout << buf << 'E' << (int)exp << endl << endl;

    value = -1.2345E5;
    cout << "-1.0 > value" << endl;
    cout << value << endl;
    ftoaSci(buf, sizeof(buf), &exp, value, digits);
    cout << buf << 'E' << (int)exp << endl << endl;

    cout << "Short buffer test:" << endl;
    ftoaSci(buf, sizeof(buf), &exp, value, digits);
    cout << "Result: " << buf << endl << endl;
}

void test_ftoaSciExp() {
    char buf[16];
    int8_t exp;
    float value;
    int digits = 4;

    cout << "ftoaSciExp():" << endl << endl;

    value = 1.2345E5;
    cout << "value > 1.0" << endl;
    cout << value << endl;
    ftoaSciExp(buf, sizeof(buf), &exp, value, digits);
    cout << buf << endl << endl;

    value = 1.2345E-5;
    cout << "1.0 > value > 0.0" << endl;
    cout << value << endl;
    ftoaSciExp(buf, sizeof(buf), &exp, value, digits);
    cout << buf << endl << endl;

    value = -1.2345E-5;
    cout << "0.0 > value > -1.0" << endl;
    cout << value << endl;
    ftoaSciExp(buf, sizeof(buf), &exp, value, digits);
    cout << buf << endl << endl;

    value = -1.2345E5;
    cout << "-1.0 > value" << endl;
    cout << value << endl;
    ftoaSciExp(buf, sizeof(buf), &exp, value, digits);
    cout << buf << endl << endl;

    cout << "Short buffer test:" << endl;
    ftoaSciExp(buf, sizeof(buf), &exp, value, digits);
    cout << "Result: " << buf << endl << endl;
}

int main(int argc, char** argv) {
    // test_ftoaEng();
    // test_ftoaEngExp();
    // test_ftoaFix();
    // test_ftoaSci();
    test_ftoaSciExp();

    // TODO: check how they behave for special cases as 0.0 and off limits
    return 0;
}
