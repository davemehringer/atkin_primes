#include <iostream>
#include <vector>
#include <sstream>

using namespace std;

vector<long long> sieveOfAtkin(long long limit) {
    auto off = limit % 2 == 0 ? 5 : 6;
    auto size = (limit - off)/2 + 1; 
    vector<bool> sieve;
    sieve.assign(size, false);
    long long x = 1;
    long long x2 = 1;
    while (x2 <= limit) {
        long long y = 1;
        long long y2 = 1;
        while (y2 <= limit) {
            long long n2 = 3*x2 + y2;
            if (n2 > limit) {
                break;
            }
            long long n1 = 4*x2 + y2;
            long long m1 = n1 % 12;
            if (n1 <= limit && (m1 == 1 || m1 == 5)) {
                auto i = (n1 - 5)/2;
                sieve[i] = ! sieve[i];
            }
            if (n2 % 12 == 7) {
                auto i = (n2 - 5)/2;
                sieve[i] = ! sieve[i];

            }
            ++y;
            y2 = y*y;
        }
        long long z = x;
        while (z >= 1) {
            long long n3 = 3*x2 - z*z;
            if (n3 > limit) {
                break;
            }
            if (n3 % 12 == 11) {
                auto i = (n3 - 5)/2;
                sieve[i] = ! sieve[i];
            }
            --z;
        }
        ++x;
        x2 = x*x;
    }

    long long r = 5;
    long long r2 = r*r;
    while (r2 <= limit) {
        auto i = (r - 5)/2;
        if (sieve[i]) {
            for (long long j=r2; j<=limit; j+=2*r2) {
                auto k = (j - 5)/2;
                sieve[k] = false;
            }
        }
        r += 2;
        r2 = r*r;
    }

    vector<long long> primes;
    long long num = 5;
    for (const auto& s: sieve) {
        if (s) {
            primes.push_back(num);
        }
        num += 2;
    }
    primes.insert(primes.begin(), 3);
    primes.insert(primes.begin(), 2);
    return primes;
}
    
int main(int argc, char *argv[]) {
    long long count = 100;
    if (argc >= 2) {
        string x(argv[1]);
        stringstream y(x);
        y >> count;
    }
    auto primes = sieveOfAtkin(count);
    cout << primes.size() << endl;
    /*
    for (const auto& p: primes) {
        cout << p << endl;
    }
    */
    return 0;
}