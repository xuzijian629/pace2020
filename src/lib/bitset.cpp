#pragma once
#include <bits/stdc++.h>
using namespace std;

class Bitset {
    using ull = unsigned long long;
    vector<ull> bits;

public:
    Bitset();
    int size() const;
    void set(int k);
    void reset(int k);
    void reset();
    bool test(int k) const;
    bool any() const;
    Bitset& operator&=(const Bitset& rhs);
    Bitset& operator|=(const Bitset& rhs);
    Bitset& operator^=(const Bitset& rhs);
    Bitset operator&(const Bitset& rhs) const;
    Bitset operator|(const Bitset& rhs) const;
    Bitset operator^(const Bitset& rhs) const;
    int _Find_first() const;
    int _Find_next(int k) const;
    int count() const;
    bool operator<(const Bitset& rhs) const;
    bool operator==(const Bitset& rhs) const;
    bool operator!=(const Bitset& rhs) const;
};

Bitset::Bitset() {}
int Bitset::size() const { return bits.size() << 6; }
void Bitset::set(int k) {
    int idx = k >> 6;
    if (idx < bits.size()) {
        bits[idx] |= 1ull << (k & 63);
    } else {
        bits.resize(idx + 1);
        bits[idx] |= 1ull << (k & 63);
    }
}
void Bitset::reset(int k) {
    int idx = k >> 6;
    if (idx < bits.size()) {
        bits[idx] &= ~(1ull << (k & 63));
    }
}
void Bitset::reset() { bits.clear(); }
bool Bitset::test(int k) const {
    int idx = k >> 6;
    if (idx < bits.size()) {
        return bits[idx] >> (k & 63) & 1;
    }
    return false;
}
bool Bitset::any() const {
    for (ull b : bits)
        if (b) return true;
    return false;
}
Bitset& Bitset::operator&=(const Bitset& rhs) {
    int ret_size = bits.size();
    if (rhs.bits.size() < ret_size) {
        ret_size = rhs.bits.size();
        bits.resize(ret_size);
    }
    for (int i = 0; i < ret_size; i++) {
        bits[i] &= rhs.bits[i];
    }
    return *this;
}
Bitset& Bitset::operator|=(const Bitset& rhs) {
    int ret_size = bits.size();
    if (rhs.bits.size() > ret_size) {
        ret_size = rhs.bits.size();
        bits.resize(ret_size);
    }
    for (int i = 0; i < ret_size; i++) {
        bits[i] |= rhs.bits[i];
    }
    return *this;
}
Bitset& Bitset::operator^=(const Bitset& rhs) {
    int ret_size = bits.size();
    if (rhs.bits.size() > ret_size) {
        ret_size = rhs.bits.size();
        bits.resize(ret_size);
    }
    for (int i = 0; i < ret_size; i++) {
        bits[i] ^= rhs.bits[i];
    }
    return *this;
}
Bitset Bitset::operator&(const Bitset& rhs) const { return Bitset(*this) &= rhs; }
Bitset Bitset::operator|(const Bitset& rhs) const { return Bitset(*this) |= rhs; }
Bitset Bitset::operator^(const Bitset& rhs) const { return Bitset(*this) ^= rhs; }
int Bitset::_Find_first() const {
    int cum = 0;
    for (ull b : bits) {
        if (b) {
            return cum + __builtin_ctzll(b);
        } else {
            cum += 64;
        }
    }
    return cum;
}
int Bitset::_Find_next(int k) const {
    int idx = k >> 6;
    int cum = idx << 6;
    k &= 63;
    bool first = true;
    for (int i = idx; i < bits.size(); i++) {
        ull b = bits[i];
        if (first) {
            if (k != 63) {
                b &= ~((1ull << (k + 1)) - 1);
                if (b) {
                    return cum + __builtin_ctzll(b);
                }
            }
            cum += 64;
            first = false;
        } else {
            if (b) {
                return cum + __builtin_ctzll(b);
            } else {
                cum += 64;
            }
        }
    }
    return cum;
}
int Bitset::count() const {
    int ret = 0;
    for (ull b : bits) {
        if (b) ret += __builtin_popcount(b);
    }
    return ret;
}
// これは unordered_set の定義用にとりあえずの順序を定義しておけばおｋ
bool Bitset::operator<(const Bitset& rhs) const {
    if (bits.size() != rhs.bits.size()) return bits.size() < rhs.bits.size();
    for (int i = 0; i < bits.size(); i++) {
        if (bits[i] != rhs.bits[i]) return bits[i] < rhs.bits[i];
    }
    return false;
}
bool Bitset::operator==(const Bitset& rhs) const {
    for (int i = 0; i < min(bits.size(), rhs.bits.size()); i++) {
        if (bits[i] != rhs.bits[i]) return false;
    }
    if (bits.size() > rhs.bits.size()) {
        for (int i = rhs.bits.size(); i < bits.size(); i++) {
            if (bits[i]) return false;
        }
    } else if (bits.size() < rhs.bits.size()) {
        for (int i = bits.size(); i < rhs.bits.size(); i++) {
            if (rhs.bits[i]) return false;
        }
    }
    return true;
}
bool Bitset::operator!=(const Bitset& rhs) const {
    for (int i = 0; i < min(bits.size(), rhs.bits.size()); i++) {
        if (bits[i] != rhs.bits[i]) return true;
    }
    if (bits.size() > rhs.bits.size()) {
        for (int i = rhs.bits.size(); i < bits.size(); i++) {
            if (bits[i]) return true;
        }
    } else if (bits.size() < rhs.bits.size()) {
        for (int i = bits.size(); i < rhs.bits.size(); i++) {
            if (rhs.bits[i]) return true;
        }
    }
    return false;
}
