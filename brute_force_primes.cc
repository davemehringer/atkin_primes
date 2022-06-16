#include <iostream>
#include <math.h>
#include <vector>

using namespace std;


vector<int> bruteForcePrimes(int limit) {
    vector<int> primes(1, 3);
    for (int i=5; i<=limit; i += 2) {
        int sri = int(sqrt(i));
        bool isPrime = true;
        for (const auto& p: primes) {
            if (p > sri) {
                break;
            }
            if (i % p == 0) {
                isPrime = false;
                break;
            }
        }
        if (isPrime) {
            primes.push_back(i);
        }
    }
    primes.insert(primes.begin(), 2);
    return primes;
}

int main() {
    auto primes = bruteForcePrimes(10000000);
    cout << "n primes " << primes.size() << endl;
    /*
    for (const auto& p: primes) {
        cout << p << endl;
    }
    */
    return 0;
}