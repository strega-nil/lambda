#pragma once

#ifndef UB_NO_LAM
#define RLAM(...) [&](__VA_ARGS__) -> decltype(auto)
#define LAM(...) [&](__VA_ARGS__)
#endif