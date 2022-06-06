#include <iostream>
#include <vector>

using namespace std;

vector<long long> sieveOfAtkin(long long limit) {
    vector<bool> sieve;
    sieve.assign(limit + 1, false);
    if (limit > 2) {
        sieve[2] = true;
    }
    if (limit > 3) {
        sieve[3] = true;
    }

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
                sieve[n1] = ! sieve[n1];
            }
            if (n2 % 12 == 7) {
                sieve[n2] = ! sieve[n2];
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
            // if (n3 <= limit && n3 % 12 == 11) {
            if (n3 % 12 == 11) {
                sieve[n3] = ! sieve[n3];
            }
            --z;
        }
        x += 1;
        x2 = x*x;
    }

    long long r = 5;
    long long r2 = r*r;
    while (r2 <= limit) {
        if (sieve[r]) {
            for (long long i=r2; i<=limit; i+=r2) {
                sieve[i] = false;
            }
        }
        ++r;
        r2 = r*r;
    }

    vector<long long> primes;
    long long count = 0;
    for (const auto& s: sieve) {
        if (s) {
            primes.push_back(count);
        }
        ++count;
    }

    return primes;
}
    
int main() {
    auto primes = sieveOfAtkin(100000000);
    cout << primes.size() << endl;
    /*
    for (const auto& p: primes) {
        cout << p << endl;
    }
    */
    return 0;
}