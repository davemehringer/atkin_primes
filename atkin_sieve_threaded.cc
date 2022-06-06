#include <iostream>
#include <vector>
#include <sstream>
#include <thread>
#include <future>
#include <set>

using namespace std;

void test(string msg) {
    cout << msg << endl;
}

void compute(
    vector<bool> sieve, promise<pair<vector<bool>, set<long long>>> && p,
    long long limit, uint threadID, uint nthreads
) {
    long long x = threadID + 1;
    long long x2 = x*x;
    while (x2 <= limit) {
        long long y = 1;
        long long y2 = 1;
        while (y2 <= limit) {
            long long n2 = 3 * x2 + y2;
            if (n2 > limit) {
                break;
            }
            long long n1 = 4 * x2 + y2;
            long long m1 = n1 % 12;
            if (n1 <= limit && (m1 == 1 || m1 == 5)) {
                auto i = (n1 - 5) / 2;
                sieve[i] = !sieve[i];
            }
            if (n2 % 12 == 7) {
                auto i = (n2 - 5) / 2;
                sieve[i] = !sieve[i];
            }
            ++y;
            y2 = y * y;
        }
        long long z = x;
        while (z >= 1) {
            long long n3 = 3 * x2 - z * z;
            if (n3 > limit) {
                break;
            }
            if (n3 % 12 == 11) {
                auto i = (n3 - 5) / 2;
                sieve[i] = !sieve[i];
            }
            --z;
        }
        x += nthreads;
        x2 = x * x;
    }
    // exclude contains the indices, not the values, that need to be
    // excluded
    set<long long> exclude;
    long long r = 5 + 2*threadID;
    long long r2 = r * r;
    while (r2 <= limit) {
        auto i = (r - 5) / 2;
        if (sieve[i]) {
            for (long long j = r2; j <= limit; j += 2 * r2) {
                auto k = (j - 5) / 2;
                exclude.insert(k);
            }
        }
        r += 2*nthreads;
        r2 = r * r;
    }
    p.set_value(make_pair(sieve, exclude));
}

vector<long long> sieveOfAtkin(long long limit, int nthreads) {
    auto off = limit % 2 == 0 ? 5 : 6;
    auto size = (limit - off) / 2 + 1;
    vector<bool> sieve;
    sieve.assign(size, false);
    vector<promise<pair<vector<bool>, set<long long>>>> promises(nthreads);
    vector<future<pair<vector<bool>, set<long long>>>> futures(nthreads);
    for (uint i=0; i<nthreads; ++i) {
        futures[i] = promises[i].get_future();
    }

    vector<unique_ptr<thread>> threads(nthreads);
    for (uint i = 0; i < nthreads; ++i) {
        threads[i].reset(
            new thread(&compute, sieve, move(promises[i]), limit, i, nthreads)
        );
    }
    auto sbegin = sieve.begin();
    auto send = sieve.end();
    vector<set<long long>> excludes(nthreads);
    for (uint i=0; i<nthreads; ++i) {
        threads[i]->join();
        auto vr = futures[i].get();
        auto v = vr.first;
        excludes[i] = vr.second;
        if (i == 0) {
            sieve = v;
        }
        else {
            // TODO investigate valarrays to do this quicker
            auto viter = v.begin();
            auto siter = sbegin;
            while (siter != send) {
                *siter = *siter ^ *viter;
                ++siter;
                ++viter;
            }
        }
    }
    for (auto siter=excludes.begin(); siter!=excludes.end(); ++siter) {
        for (const auto& n: *siter) {
            sieve[n] = false;
        }
    }

    vector<long long> primes;
    long long num = 5;
    for (const auto &s : sieve) {
        if (s) {
            primes.push_back(num);
        }
        num += 2;
    }
    primes.insert(primes.begin(), 3);
    primes.insert(primes.begin(), 2);
    return primes;
}

long long stringToInt(const char *s) {
    string x(s);
    stringstream y(x);
    long long z;
    y >> z;
    return z;
}

int main(int argc, char *argv[]) {
    long long count = 100;
    uint nthreads = 1;
    if (argc >= 2) {
        count = stringToInt(argv[1]);
        if (argc >= 3) {
            nthreads = (int)stringToInt(argv[2]);
        }
    }
    auto primes = sieveOfAtkin(count, nthreads);
    cout << "n primes " << primes.size() << endl;
    /*
    for (const auto& p: primes) {
        cout << p << endl;
    }
    */
    
    return 0;
}