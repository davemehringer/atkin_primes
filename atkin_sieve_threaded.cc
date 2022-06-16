#include <future>
#include <math.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

using namespace std;

void first_loop(vector<bool>& sieve, long long limit, uint n_min, uint n_max) {
    long long x = 1;
    long long x2 = 1;
    while (x2 <= limit) {
        long long y = 1;
        if (n_min - 4*x2 > 0) {
            y = int(sqrt(n_min - 4*x2));
        }
        long long y2 = y*y;
        while (y2 <= min(n_max - 3*x2, limit)) {
            long long n2 = 3*x2 + y2;
            if (n2 >= n_min && n2 <= n_max) {
                if (n2 % 12 == 7) {
                    auto i = (n2 - 5)/2;
                    sieve[i] = ! sieve[i];
                }
            }
            long long n1 = 4*x2 + y2;
            if (n1 >= n_min && n1 <= n_max) {
                long long m1 = n1 % 12;
                if (n1 <= limit && (m1 == 1 || m1 == 5)) {
                    auto i = (n1 - 5)/2;
                    sieve[i] = ! sieve[i];
                }
            }
            ++y;
            y2 = y*y;
        }
        long long z = x;
        if (3*x2 - n_min > 0) {
            z = min((long long)(sqrt(3*x2 - n_min)), x);
        }
        while (z >= 1) {
            auto n3 = 3*x2 - z*z;
            if (n3 > n_max) {
                break;
            }
            if (n3 >= n_min && n3 <= n_max && n3 % 12 == 11) {
                auto i = (n3 - 5)/2;
                sieve[i] = ! sieve[i];
            }
            --z;
        }
        ++x;
        x2 = x*x;
    }
}

vector<long long> sieveOfAtkin(long long limit, uint nthreads) {
    auto off = limit % 2 == 0 ? 5 : 6;
    auto size = (limit - off)/2 + 1; 
    vector<bool> sieve(size, false);
    /*
    vector<promise<vector<long long>>> promises(nthreads);
    vector<future<vector<long long>>> futures(nthreads);
    for (uint i=0; i<nthreads; ++i) {
        futures[i] = promises[i].get_future();
    }
    */
    vector<thread> threads(nthreads);
    uint n_min = 0;
    uint n_max = 0;
    uint n_nom = int((limit - 4)/nthreads) - 1;
    uint mod = (limit - 4) % nthreads;
    // cout << "mod " << mod << endl;
    for (uint i = 0; i < nthreads; ++i) {
        n_min = i == 0 ? 5 : n_max + 1;
        n_max = n_min + n_nom;
        if (i < mod) {
            ++n_max;
        }
        // cout << "n_min " << n_min << " n_max " << n_max << endl;
        threads[i] = thread(&first_loop, /*move(promises[i]),*/ ref(sieve), limit, n_min, n_max);
    }
    for (uint i=0; i<nthreads; ++i) {
        threads[i].join();
    }
    //first_loop(sieve, limit);
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
    uint nthreads = 1;
    if (argc >= 2) {
        string x(argv[1]);
        stringstream y(x);
        y >> count;
    }
    if (argc >= 3) {
        string x(argv[2]);
        stringstream y(x);
        y >> nthreads;
    }
    //cout << "count " << count << " nthreads " << nthreads << endl;
    auto primes = sieveOfAtkin(count, nthreads);
    cout << primes.size() << endl;
    if (count <= 1000) {
        for (const auto& p: primes) {
            cout << p << endl;
        }
    }
    return 0;
}
