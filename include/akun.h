#ifndef AKUN_H
#define AKUN_H

#include <iostream>
#include <string>

using namespace std;

struct Akun {
    string username, password;
    int wins, losses, games;
};

extern int friendGraph[10][10];

// Deklarasi fungsi-fungsi akun
void loadGraph();
void simpanGraph();
int loadAkun(Akun akun[], int maxSize);
void simpanAkun(Akun akun[], int count);
int loginAkun(Akun akun[], int count, string username, string password);
bool daftarAkun(Akun akun[], int &count, string username, string password);
void updateStats(Akun akun[], int count, string username, bool menang);

#endif