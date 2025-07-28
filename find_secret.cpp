#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <nlohmann/json.hpp>
#include <cmath>
#include <cassert>
#include <algorithm>
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
    return llround(c);  // Round to nearest integer
}
void solve(const string& filename) {
    ifstream fin(filename);
    if (!fin) {
        cerr << "Cannot open file: " << filename << endl;
        return;
    }
    json j;
    try {
        fin >> j;
    } catch (...) {
        cerr << "Invalid JSON in file: " << filename << endl;
        return;
    }
    int n = j["keys"]["n"];
    int k = j["keys"]["k"];
    vector<pair<long long, long long>> points;
    vector<pair<int, json>> share_entries;
    // Collect and sort keys (except "keys" metadata)
    for (auto& el : j.items()) {
        if (el.key() == "keys") continue;
        try {
            int x = stoi(el.key());
            share_entries.emplace_back(x, el.value());
        } catch (...) {
            cerr << "Invalid key format: " << el.key() << endl;
        }
    }
    sort(share_entries.begin(), share_entries.end());
    for (int i = 0; i < k && i < share_entries.size(); ++i) {
        int x = share_entries[i].first;
        json valueObj = share_entries[i].second;
        int base;
        try {
            if (valueObj["base"].is_string())
                base = stoi(valueObj["base"].get<string>());
            else
                base = valueObj["base"];
        } catch (...) {
            cerr << "Invalid base format at x = " << x << endl;
            continue;
        }
        string value = valueObj["value"];
        try {
            long long y = decode(value, base);
            points.emplace_back(x, y);
        } catch (exception& e) {
            cerr << "Decoding error at x = " << x << ": " << e.what() << endl;
        }
    }
    if (points.size() < k) {
        cerr << "Not enough valid points to reconstruct secret in file: " << filename << endl;
        return;
    }
    long long secret = lagrange_interpolate_c(points);
    cout << "Secret from " << filename << ": " << secret << endl;
}
int main() {
    solve("testcase1.json");
    solve("testcase2.json");
    return 0;
}
