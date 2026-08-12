#ifndef CAKE_4_H
#define CAKE_4_H

#include <string>
#include <array>

std::string encode(int n, const std::array<std::array<long long, 2000>, 2000>& w);
void decode(int n, const std::string& s);
int query(int l, int r);

#endif
