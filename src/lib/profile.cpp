#pragma once
#include <bits/stdc++.h>
using namespace std;

map<string, long long> profile_sum, profile_cnt;
map<string, chrono::steady_clock::time_point> profile_start;

#define START_PROFILE start_profile(__func__)
#define STOP_PROFILE stop_profile(__func__)

void start_profile(const string& name) {
#ifdef PROFILE
    assert(!profile_start.count(name));
    profile_start[name] = chrono::steady_clock::now();
#endif
}

void stop_profile(const string& name) {
#ifdef PROFILE
    assert(profile_start.count(name));
    profile_sum[name] +=
        chrono::duration_cast<chrono::nanoseconds>(chrono::steady_clock::now() - profile_start[name]).count();
    profile_cnt[name]++;
    profile_start.erase(name);
#endif
}

void print_profile_result() {
#ifdef PROFILE
    cout << "********** PROFILE RESULTS **********" << endl;
    vector<pair<string, long long>> total;
    for (auto& v : profile_sum) total.emplace_back(v.first, v.second);
    sort(total.begin(), total.end(), [](auto& a, auto& b) { return a.second > b.second; });
    for (auto& v : total) {
        cout << "[" << v.first << "] ";
        cout << "time: " << v.second / 1000000 << "[ms] ";
        cout << "count: " << profile_cnt[v.first] << endl;
    }
#endif
}
