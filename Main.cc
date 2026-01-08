#include <ctime>
#include <cstdlib>
#include <utility>
#include <array>
#include <fstream> 
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <ctime>
#include <sstream> 
#include <algorithm> 
#include <cctype>    
#include <unordered_map> 
#include <random>    
#include <chrono>

#include "clause_functions.h"



int main(int argc, char** argv) {
    std::clock_t start = std::clock();
    const double TIMEOUT_SECONDS = 300.0; // 5 minuti = 300 secondi
    
    // 1. Verifico argomenti
    if (argc != 2) {
        std::cerr << "Utilizzo: " << argv[0] << " <input.cnf>\n\n";
        return 1;
    }

    std::string filepath = argv[1];
    auto [num_vars, num_clauses] = leggi_header_cnf(filepath);

    // 2. Dichiarazione di blocchi, assignment e letterale_to_blocchi 
    std::cout << "Numero variabili: " << num_vars << "\n";
    std::cout << "Numero clausole: " << num_clauses << "\n";

    std::vector<Blocco> blocchi;

    std::unordered_map<int16_t, std::vector<size_t>> letterale_to_blocchi;
    letterale_to_blocchi.reserve(num_vars + 1);
    
    std::vector<int> assignment;
    assignment.reserve(num_vars + 1);
    assignment.resize(num_vars + 1);

    // Setup per lo shuffle 
    std::random_device rd;
    std::mt19937 g(rd());

    // 3. Inizializzazione assegnamento casuale
    srandom(time(0));
    for (int i = 1; i <= num_vars; i++) {
        assignment[i] = random() % 2;
    }

    int max_letterali = MAX_N;
    int max_clausole  = MAX_M;

    // 4. Parsing ed inserimento clausole
    parser_cnf(
        filepath,
        blocchi,
        letterale_to_blocchi,
        assignment,
        max_letterali,
        max_clausole
    );

    // 5 Costruzione dei puntatori 
    costruisci_catene(blocchi, num_vars);

    // 6. Inizio riparazione
    int repair_count = 0;
    const int max_repair_attempts = 2000;

    // 6.1 Inizializzo clausole false 
    std::vector<std::pair<size_t, int>> unsat_clauses;
    unsat_clauses.reserve(num_clauses);
    unsat_clauses = clausole_unsat(blocchi);
    std::cout << "\nLe clausole non soddisfatte iniziali sono: " << unsat_clauses.size() << "\n";

    while (repair_count < max_repair_attempts && !unsat_clauses.empty()) {
     // CONTROLLO TIMEOUT
    double elapsed = (std::clock() - start) / (double)CLOCKS_PER_SEC;
    if (elapsed > TIMEOUT_SECONDS) {
        std::cout << "TIMEOUT RAGGIUNTO: interrotto dopo " << elapsed << " secondi\n";
        break;
    }
    repair_count++;
    std::shuffle(unsat_clauses.begin(), unsat_clauses.end(), g);

        for (auto it = unsat_clauses.begin(); it != unsat_clauses.end(); ) {
            size_t blocco_idx = it->first;
            int idx = it->second;
            
            Blocco& blocco = blocchi[blocco_idx];

            // Se è già SAT (per qualche motivo), rimuovo da unsat_clauses
            if (clausola_soddisfatta(blocco, idx)) {
                it = unsat_clauses.erase(it);
                continue;
            }

            // Trovo letterale migliore da riparare
            int16_t migliore = migliore_flip(blocchi, blocco, idx, num_vars);
                
            // Applico il flip
            applica_flip(migliore, assignment, &blocco);

            // Verifico di nuovo se ora è soddisfatta
            if (clausola_soddisfatta(blocco, idx)) {
                it = unsat_clauses.erase(it);
            } else {
                it++; // clausola non riparata, passo alla prossima
            }
        }

        // 6.2 Ricalcolo delle clausole UNSAT
        unsat_clauses = clausole_unsat(blocchi);
        if (unsat_clauses.empty()) std::cout << "\n\nFINE SAT SOLVER !\n";
    }
   
    std::cout << "\nAssegnamento finale delle variabili:\n\n";
    for (int i = 1; i <= num_vars; i++){
        std::cout << "x" << i << " = " << assignment[i] << ", ";      
    }
   
     if (!unsat_clauses.empty()) {
        std::cout << "\n\nCLAUSOLE UNSAT RIMASTE: " << unsat_clauses.size() << "\n";
    } 
    
    double cpu_time = (std::clock() - start) / (double)CLOCKS_PER_SEC;
    std::cout << "CPU Time: " << cpu_time << " secondi\n";

    return 0;

}
