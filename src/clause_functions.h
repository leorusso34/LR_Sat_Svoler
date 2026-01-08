#ifndef CLAUSE_FUNCTIONS_H
#define CLAUSE_FUNCTIONS_H

#define CLAUSOLA_SIZE 3
#define MAX_N 100
#define MAX_M 500
#define MAX_WORDS ((3 * MAX_M + 63)/64)

#include <iostream>
#include <array>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <utility>
#include <unordered_set> 
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <bit>
#include <fstream>
#include <sstream>
#include <string>

// Struttura blocchi
typedef struct Blocco Blocco;
struct Blocco {
    int16_t titolo[MAX_N];                                  // array di N letterali
    int16_t clausole[MAX_M][CLAUSOLA_SIZE];                 // array di clausole Mx3
    uint64_t bitvector[MAX_N][MAX_WORDS];                   // N array di uint64_t (uno per letterale)
    uint64_t soddisfatte[MAX_WORDS];                        // array di uint64_t per soddisfatte
    uint16_t num_clausole;                                  // numero di clausole finora inserite nel blocco
    uint16_t num_letterali;                                 // numero di letterali finora inseriti nel blocco
    
    Blocco* next_per_letterale[MAX_N];                      // puntatore al prossimo blocco che contiene un determinato letterale
};

// Dichiarazioni delle funzioni
void applica_flip(int16_t lit, std::vector<int>& assignment, Blocco* blocco_iniziale);

void inserisci_clausola(Blocco& blocco, int16_t clausola[CLAUSOLA_SIZE], const std::vector<int> assignment, std::unordered_map<int16_t, std::vector<size_t>>& letterale_to_blocchi, size_t blocco_idx);

int16_t migliore_flip(const std::vector<Blocco>& blocchi, const Blocco& blocco_corrente, uint16_t indice_clausola, int num_vars);

std::vector<std::pair<size_t, int>> clausole_unsat(std::vector<Blocco>& blocchi);

int conta_letterali_in_comune(const int16_t* titolo_blocco, uint16_t num_letterali, const int16_t clausola[CLAUSOLA_SIZE]);

void raggruppa_clausola_in_blocchi(const std::array<int16_t, CLAUSOLA_SIZE>& clausola_const, std::vector<Blocco>& blocchi, std::unordered_map<int16_t, std::vector<size_t>>& letterale_to_blocchi, const std::vector<int>& assignment, int max_letterali, int max_clausole);

bool clausola_soddisfatta(const Blocco& blocco, int idx);

void costruisci_catene(std::vector<Blocco>& blocchi, int num_vars);

void parser_cnf(const std::string& filepath, std::vector<Blocco>& blocchi, std::unordered_map<int16_t, std::vector<size_t>>& letterale_to_blocchi, const std::vector<int>& assignment, int max_letterali, int max_clausole);

std::pair<int, int> leggi_header_cnf(const std::string& filepath);

#endif
