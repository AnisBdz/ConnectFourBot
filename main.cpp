
#include <iostream>
#include <mutex>

#include "arbitre.h"

// #include <immintrin.h>
#include <tmmintrin.h>
#include <chrono>

using namespace std;

void playground();

short int mat[6][8] __attribute__ ((aligned (16))) =
                        {{ 3, 4,  5,  7,  5, 4, 3, 0},
                         { 4, 6,  8, 10,  8, 6, 4, 0},
                         { 5, 8, 11, 13, 11, 8, 5, 0},
                         { 5, 8, 11, 13, 11, 8, 5, 0},
                         { 4, 6,  8, 10,  8, 6, 4, 0},
                         { 3, 4,  5,  7,  5, 4, 3, 0}};

int main()
{
    //initialise la graine du générateur aléatoire
    // std::srand(20000);

    // création de l'Arbitre (graine , joueur 1, joueur 2 , nombre de parties)
    // Variation::activated = false;
    // Arbitre a (9999, player::A_, player::RAND, 1);
    // a.challenge();

    // Variation::activated = true;
    // Arbitre b (9999, player::A_, player::BRUTAL_, 100);
    // b.challenge();

    playground();
    return 0;
}

int sum_sse() {
    __m128i v = _mm_setr_epi16(0, 0, 0, 0, 0, 0, 0, 0);
    for (int i = 0; i < 6; i++) {
        auto w = _mm_mullo_epi16(*((__m128i*)(mat[i])), *((__m128i*)(mat[i])));
        v = _mm_add_epi16(v, w);
    }


    int sum = _mm_extract_epi16(v, 0);
    sum += _mm_extract_epi16(v, 0);
    sum += _mm_extract_epi16(v, 2);
    sum += _mm_extract_epi16(v, 3);
    sum += _mm_extract_epi16(v, 4);
    sum += _mm_extract_epi16(v, 5);
    sum += _mm_extract_epi16(v, 6);

    return sum;
}

int sum_normal() {
    int sum = 0;

    for (int x = 0; x < 7; x++) {
        for (int y = 0; y < 6; y++) {
            sum += mat[y][x];
        }
    }

    return sum;
}

void playground() {
    int sum;
    int l = 200;
    auto t0 = std::chrono::system_clock::now();
    for (int i = 0; i < l; i++) {
        sum = sum_normal();
    }
    auto t1 = std::chrono::system_clock::now();
    for (int i = 0; i < l; i++) {
        sum = sum_sse();
    }
    auto t2 = std::chrono::system_clock::now();

    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);

    std::cout << "norm took: " << duration1.count() << "\n";
    std::cout << "sse  took: " << duration2.count() << "\n";

}
