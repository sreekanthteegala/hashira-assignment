#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <nlohmann/json.hpp>
#include <cmath>
#include <cassert>

using json = nlohmann::json;
using namespace std;

// Convert string in given base to integer
long long decode(const string& value, int base) {
    long long result = 0;
    for (char c : value) {
        int digit;
        if (isdigit(c)) digit = c - '0';
        else if (isalpha(c)) digit = tolower(c) - 'a' + 10;
        else throw runtime_error("Invalid character in base string");
        if (digit >= base) throw runtime_error("Digit out of base range");
        result = result * base + digit;
    }
    return result;
}

// Lagrange interpolation at x=0 to find the constant term
long long lagrange_interpolate_c(const vector<pair<long long, long long>>& points) {
    long double c = 0;
    int k = points.size();
    for (int i = 0; i < k; ++i) {
        long double xi = points[i].first;
        long double yi = points[i].second;
        long double li = 1;
        for (int j = 0; j < k; ++j) {
            if (i == j) continue;
            long double xj = points[j].first;
            li *= (-xj) / (xi - xj);
        }
        c += yi * li;
    }
    // Round to nearest integer, as coefficients are integers
    return llround(c);
}

void solve(const string& filename) {
    ifstream fin(filename);
    if (!fin) {
        cerr << "Cannot open file: " << filename << endl;
        return;
    }
    json j;
    fin >> j;
    int n = j["keys"]["n"];
    int k = j["keys"]["k"];
    vector<pair<long long, long long>> points;
    int count = 0;
    for (auto& el : j.items()) {
        if (el.key() == "keys") continue;
        if (++count > k) break; // Only use k points
        int x = stoi(el.key());
        int base = stoi(el.value()["base"].get<string>());
        string value = el.value()["value"];
        long long y = decode(value, base);
        points.push_back({x, y});
    }
    long long secret = lagrange_interpolate_c(points);
    cout << secret << endl;
}

int main() {
    // Print secrets for both testcases
    solve("testcase1.json");
    solve("testcase2.json");
    return 0;
}

// g++ -std=c++17 -o find_secret find_secret.cpp
find_secret.exe