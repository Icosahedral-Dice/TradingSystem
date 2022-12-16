/**
 * utilities.hpp
 * Offers utility functions for bond pricing services
 *
 * @authors Mingsen Wang
 */

#ifndef utilities_hpp
#define utilities_hpp

#include <iostream>

#include <string>

#include "products.hpp"
#include <utility>
#include <map>
#include "boost/date_time/gregorian/gregorian.hpp"

#include <chrono>
#include <ctime>



using namespace std;
using namespace boost::gregorian;

// Convert numeric price to bond notation
float ConvertPrice(const string& str_price) {
    // "100-xyz" -> 100 + xy / 32 + z / 256
    auto delimiter_pos = str_price.find('-');
    
    // Integer
    float res = stoi(str_price.substr(0, delimiter_pos));
    
    // xy
    res += stoi(str_price.substr(delimiter_pos + 1, 2)) / 32.;
    
    // z
    char last_char = str_price[delimiter_pos + 3];
    if (last_char == '+') {
        res += 1. / 64.;
    } else {
        res += (last_char - '0') / 256.;  // int('0') == 48
    }
    return res;
}

string ConvertPrice(float f_price) {
    int integer = int(f_price);
    string res = to_string(integer) + '-';
    
    f_price -= integer;
    f_price *= 32.;
    integer = int(f_price);
    if (integer < 10) {
        // Pad with 0 if only one digit
        res += '0';
    }
    
    res += to_string(integer);
    
    f_price -= integer;
    f_price *= 8.;
    integer = int(f_price);
    if (integer == 4.) {
        res += '+';
    } else {
        res += to_string(integer);
    }
    
    return res;
}



map<int, pair<string, date>> kBondMapMaturity({
    {2, {"91282CFX4", {2024, Nov, 30}}},
    {3, {"91282CFW6", {2025, Nov, 15}}},
    {5, {"91282CFZ9", {2027, Nov, 30}}},
    {7, {"91282CFY2", {2029, Nov, 30}}},
    {10, {"91282CFV8", {2032, Nov, 15}}},
    {20, {"912810TM0", {2042, Nov, 30}}},
    {30, {"912810TL2", {2052, Nov, 15}}}
});

map<string, pair<int, date>> kBondMapCusip({
    {"91282CFX4", {2, {2024, Nov, 30}}},
    {"91282CFW6", {3, {2025, Nov, 15}}},
    {"91282CFZ9", {5, {2027, Nov, 30}}},
    {"91282CFY2", {7, {2029, Nov, 30}}},
    {"91282CFV8", {10, {2032, Nov, 15}}},
    {"912810TM0", {20, {2042, Nov, 30}}},
    {"912810TL2", {30, {2052, Nov, 15}}}
});

// Fetch cusip object from maturity (years)
// 2Y, 3Y, 5Y, 7Y, 10Y, 20Y, and 30Y
// Issue date: 2022/11/30
string FetchCusip(int maturity) {
    return kBondMapMaturity[maturity].first;
}

Bond FetchBond(int maturity) {
    return Bond(kBondMapMaturity[maturity].first, CUSIP, "US" + to_string(maturity) + "Y", 0., kBondMapMaturity[maturity].second);
}

Bond FetchBond(const string& cusip) {
    return Bond(cusip, CUSIP, "US" + to_string(kBondMapCusip[cusip].first) + "Y", 0., kBondMapCusip[cusip].second);
}

string GetTimestamp() {
    // Get current time
    auto current_time = chrono::system_clock::now();
    
    // Get millisecond as string
    auto sec = chrono::time_point_cast<chrono::seconds>(current_time);
    auto millisec = chrono::duration_cast<chrono::milliseconds>(current_time - sec);
    auto millisec_count = millisec.count();
    string millisec_string = to_string(millisec_count);
    
    // Forcing three digits of millisecond
    if (millisec_count < 10) {
        millisec_string = "00" + millisec_string;
    } else if (millisec_count < 100) {
        millisec_string = "0" + millisec_string;
    }
    
    time_t curr_time_t = chrono::system_clock::to_time_t(current_time);
    string second_string(24, 0);
    strftime(second_string.data(), 24, "%F %T", localtime(&curr_time_t));
    
    return second_string + '.' + millisec_string;
}

#endif /* utilities_hpp */
