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

// Use __int128 for larger integer support (256-bit calculations)
using BigInt = __int128;

// Helper function to print BigInt
void printBigInt(BigInt num) {
    if (num == 0) {
        cout << "0";
        return;
    }
    
    string result;
    bool negative = num < 0;
    if (negative) num = -num;
    
    while (num > 0) {
        result = char('0' + num % 10) + result;
        num /= 10;
    }
    
    if (negative) result = "-" + result;
    cout << result;
}

// Convert string in given base to BigInt
BigInt decode(const string& value, int base) {
    BigInt result = 0;
    for (char c : value) {
        int digit;
        if (isdigit(c)) {
            digit = c - '0';
        } else if (isalpha(c)) {
            digit = tolower(c) - 'a' + 10;
        } else {
            throw runtime_error("Invalid character in base string: " + string(1, c));
        }
        
        if (digit >= base) {
            throw runtime_error("Digit " + to_string(digit) + " out of base " + to_string(base) + " range");
        }
        
        result = result * base + digit;
    }
    return result;
}

// Extended Euclidean Algorithm for modular inverse
BigInt extendedGCD(BigInt a, BigInt b, BigInt& x, BigInt& y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    BigInt x1, y1;
    BigInt gcd = extendedGCD(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return gcd;
}

// Modular inverse (not needed for this problem but useful for large number arithmetic)
BigInt modInverse(BigInt a, BigInt mod) {
    BigInt x, y;
    BigInt gcd = extendedGCD(a, mod, x, y);
    if (gcd != 1) {
        throw runtime_error("Modular inverse doesn't exist");
    }
    return (x % mod + mod) % mod;
}

// Lagrange interpolation at x=0 using rational arithmetic to avoid precision loss
BigInt lagrangeInterpolateC(const vector<pair<BigInt, BigInt>>& points) {
    int k = points.size();
    
    // For exact calculation, we'll compute the result as a fraction
    // and then extract the integer part
    BigInt numerator = 0;
    BigInt denominator = 1;
    
    for (int i = 0; i < k; ++i) {
        BigInt xi = points[i].first;
        BigInt yi = points[i].second;
        
        // Calculate Li(0) = product of (-xj)/(xi - xj) for all j != i
        BigInt li_num = 1;  // numerator of Li(0)
        BigInt li_den = 1;  // denominator of Li(0)
        
        for (int j = 0; j < k; ++j) {
            if (i == j) continue;
            BigInt xj = points[j].first;
            
            li_num *= (-xj);
            li_den *= (xi - xj);
        }
        
        // Add yi * Li(0) to the result
        // result += yi * li_num / li_den
        // To add fractions: a/b + c/d = (a*d + c*b) / (b*d)
        numerator = numerator * li_den + yi * li_num * denominator;
        denominator = denominator * li_den;
    }
    
    // The result should be an integer (since we're finding polynomial coefficients)
    if (numerator % denominator != 0) {
        cerr << "Warning: Result is not an integer. There might be precision issues." << endl;
    }
    
    return numerator / denominator;
}

void solve(const string& filename) {
    cout << "\n=== Processing " << filename << " ===" << endl;
    
    ifstream fin(filename);
    if (!fin) {
        cerr << "Cannot open file: " << filename << endl;
        return;
    }
    
    json j;
    try {
        fin >> j;
    } catch (const exception& e) {
        cerr << "Invalid JSON in file " << filename << ": " << e.what() << endl;
        return;
    }
    
    int n = j["keys"]["n"];
    int k = j["keys"]["k"];
    
    cout << "n (total points): " << n << endl;
    cout << "k (required points): " << k << endl;
    cout << "Polynomial degree: " << (k - 1) << endl;
    
    vector<pair<BigInt, BigInt>> points;
    vector<pair<int, json>> shareEntries;
    
    // Collect and sort keys (except "keys" metadata)
    for (auto& el : j.items()) {
        if (el.key() == "keys") continue;
        try {
            int x = stoi(el.key());
            shareEntries.emplace_back(x, el.value());
        } catch (const exception& e) {
            cerr << "Invalid key format: " << el.key() << " - " << e.what() << endl;
        }
    }
    
    sort(shareEntries.begin(), shareEntries.end());
    
    cout << "\nDecoding points:" << endl;
    
    // Process only the first k points
    for (int i = 0; i < k && i < shareEntries.size(); ++i) {
        int x = shareEntries[i].first;
        json valueObj = shareEntries[i].second;
        
        int base;
        try {
            if (valueObj["base"].is_string()) {
                base = stoi(valueObj["base"].get<string>());
            } else {
                base = valueObj["base"];
            }
        } catch (const exception& e) {
            cerr << "Invalid base format at x = " << x << ": " << e.what() << endl;
            continue;
        }
        
        string value = valueObj["value"];
        
        try {
            BigInt y = decode(value, base);
            points.emplace_back(x, y);
            
            cout << "Point " << (i + 1) << ": x=" << x << ", encoded_y=\"" << value 
                 << "\" (base " << base << "), decoded_y=";
            printBigInt(y);
            cout << endl;
            
        } catch (const exception& e) {
            cerr << "Decoding error at x = " << x << ": " << e.what() << endl;
        }
    }
    
    if (points.size() < k) {
        cerr << "Not enough valid points to reconstruct secret in file: " << filename 
             << " (got " << points.size() << ", need " << k << ")" << endl;
        return;
    }
    
    cout << "\nUsing " << points.size() << " points for interpolation:" << endl;
    for (size_t i = 0; i < points.size(); ++i) {
        cout << "(" << points[i].first << ", ";
        printBigInt(points[i].second);
        cout << ")";
        if (i < points.size() - 1) cout << ", ";
    }
    cout << endl;
    
    BigInt secret = lagrangeInterpolateC(points);
    
    cout << "\nSecret (constant term) from " << filename << ": ";
    printBigInt(secret);
    cout << endl;
}

int main() {
    cout << "=== SHAMIR'S SECRET SHARING SOLVER ===" << endl;
    
    solve("testcase1.json");
    solve("testcase2.json");
    
    cout << "\n=== SOLUTION COMPLETE ===" << endl;
    
    return 0;
}
