#include "clause_functions.h"
#include <iostream>
#include <algorithm>

void applica_flip(int16_t lit, std::vector<int>& assignment, Blocco* blocco_iniziale) {
    
    //flip
    uint16_t var = std::abs(lit);
    assignment[var] = 1 - assignment[var];
    int new_val = assignment[var];

    Blocco* curr = blocco_iniziale;

    do {
        int idx_pos = -1;
        int idx_neg = -1;
        int idx_avanza = -1;

        for (uint16_t j = 0; j < curr->num_letterali; ++j) {
            int16_t titolo_j = curr->titolo[j];
            if (std::abs(titolo_j) == var) {
                if (idx_avanza == -1) idx_avanza = j; 
                if (titolo_j == (int16_t)var) idx_pos = j;
                if (titolo_j == -(int16_t)var) idx_neg = j;
            }
        }

        if (idx_avanza == -1) break;

        uint16_t M = curr->num_clausole;
        int num_words = (3 * M + 63) / 64;

        // Aggiornamento
        for (int w = 0; w < num_words; ++w) {
            uint64_t old = curr->soddisfatte[w];
            uint64_t bv_pos = (idx_pos >= 0) ? curr->bitvector[idx_pos][w] : 0ULL;
            uint64_t bv_neg = (idx_neg >= 0) ? curr->bitvector[idx_neg][w] : 0ULL;

            if (new_val == 1) {
                curr->soddisfatte[w] = old + bv_pos - bv_neg;
            } else {
                curr->soddisfatte[w] = old - bv_pos + bv_neg;
            }
        }

        curr = curr->next_per_letterale[idx_avanza];

    } while (curr != blocco_iniziale && curr != nullptr);
}


// Funzione che applica il flip di un letterale
void applica_flip_vecchio(int16_t lit, std::vector<int>& assignment, Blocco* blocco_iniziale) {

    uint16_t var = std::abs(lit);

    // flip
    assignment[var] = 1 - assignment[var];
    int new_val = assignment[var];

    Blocco* blocco = blocco_iniziale;

    do {
        int idx_pos = -1;
        int idx_neg = -1;

        // Trovo (se esistono) x e -x nel titolo del blocco
        for (uint16_t j = 0; j < blocco->num_letterali; ++j) {
            if (blocco->titolo[j] ==  var) idx_pos = j;
            if (blocco->titolo[j] == -var) idx_neg = j;
        }

        uint16_t M = blocco->num_clausole;
        int num_words = (3 * M + 63) / 64;

        for (int w = 0; w < num_words; ++w) {
            uint64_t old = blocco->soddisfatte[w];
            uint64_t bv_pos = (idx_pos >= 0) ? blocco->bitvector[idx_pos][w] : 0ULL;
            uint64_t bv_neg = (idx_neg >= 0) ? blocco->bitvector[idx_neg][w] : 0ULL;

            if (new_val == 1) {
                // 0 -> 1 : +pos, -neg
                blocco->soddisfatte[w] = old + bv_pos - bv_neg;
            } else {
                // 1 -> 0 : -pos, +neg
                blocco->soddisfatte[w] = old - bv_pos + bv_neg;
            }
        }

        // continua la catena per il letterale da cui siamo partiti
        int idx_chain = (lit > 0 ? idx_pos : idx_neg);
        if (idx_chain < 0) {
            std::cerr << "ERRORE: letterale " << lit << " non trovato nel blocco @" << blocco << "\n";
            break;
        }

        blocco = blocco->next_per_letterale[idx_chain];

    } while (blocco != blocco_iniziale);
    //std::cout << "\nApplicato flip su tutti i blocchi contenenti il letterale " << lit << "\n";
}

// Funzione che inserisce la clausola presa in input nel Blocco
void inserisci_clausola(Blocco& blocco, int16_t clausola[CLAUSOLA_SIZE], const std::vector<int> assignment, std::unordered_map<int16_t, std::vector<size_t>>& letterale_to_blocchi, size_t blocco_idx) {
    uint16_t M = blocco.num_clausole;
    uint16_t N = blocco.num_letterali;

    // Inserisco clausola nel blocco
    for (int i = 0; i < CLAUSOLA_SIZE; i++) {
        blocco.clausole[M][i] = clausola[i];
    }

    // Calcolo dei bit in soddisfatte
    uint16_t soddisfatti = 0;

    for (int i = 0; i < CLAUSOLA_SIZE; i++) {
        int16_t lit = clausola[i];
        int var = std::abs(lit);
        int valore = assignment[var];
        bool soddisfa = (lit > 0 && valore == 1) || (lit < 0 && valore == 0);
        if (soddisfa) soddisfatti++;

        // Cerco letterale nel titolo
        int idx = -1;
        for (uint16_t j = 0; j < N; j++) {
            if (blocco.titolo[j] == lit) {
                idx = j;
                break;
            }
        }

        // Se non presente, lo aggiungo
        if (idx == -1) {
            idx = blocco.num_letterali;
            blocco.titolo[blocco.num_letterali++] = lit;
        }

        // Aggiorno bitvector
        int bv_bit_index = 3 * M;
        int bv_word_index = bv_bit_index / 64;
        int bv_bit_offset = bv_bit_index % 64;
        blocco.bitvector[idx][bv_word_index] |= (1ULL << bv_bit_offset);

        // Aggiorno la mappa puntatori
        auto& lista_blocchi = letterale_to_blocchi[std::abs(lit)];
        if (std::find(lista_blocchi.begin(), lista_blocchi.end(), blocco_idx) == lista_blocchi.end())
            letterale_to_blocchi[abs(lit)].push_back(blocco_idx);
    }
    
    // Scrivo i 3 bit di soddisfatte
    int bit_index = 3 * M;
    int word_index = bit_index / 64;
    int bit_offset = bit_index % 64;
    blocco.soddisfatte[word_index] &= ~(7ULL << bit_offset); // azzero
    blocco.soddisfatte[word_index] |= ((uint64_t)soddisfatti << bit_offset); // scrivo

    blocco.num_clausole++;
}

// Funzione che calcola il miglior letterale da flippare 
int16_t migliore_flip(const std::vector<Blocco>& blocchi, const Blocco& blocco_corrente, uint16_t indice_clausola, int num_vars) {
    
    const int16_t* clausola_fast = blocco_corrente.clausole[indice_clausola];
    
    // 20% di probabilità di scelta completamente casuale
    if ((std::rand() % 100) < 20) {
        return clausola_fast[std::rand() % CLAUSOLA_SIZE];
    }    

    // Array di score parziali, indicizzato per variabile
    std::vector<int> score_parziali(num_vars + 1);

    // Calcolo gli score parziali di tutti i letterali
    for (const Blocco& blocco : blocchi) {
        uint16_t M = blocco.num_clausole;
        int num_words = (3 * M + 63) / 64;

        for (uint16_t idx = 0; idx < blocco.num_letterali; idx++) {
            int16_t lit = blocco.titolo[idx];
            int var = std::abs(lit);

            int score = 0;

            for (int w = 0; w < num_words; ++w) {
                uint64_t s = blocco.soddisfatte[w];
                uint64_t b = blocco.bitvector[idx][w];

                bool is_neg = (lit < 0);
                uint64_t sodd_new = is_neg ? (s - b) : (s + b);

                uint64_t appena_soddisfatte = (~s) & sodd_new;
                uint64_t merge_aggiunte = (appena_soddisfatte | (appena_soddisfatte >> 1) | (appena_soddisfatte >> 2)) & 0x9249249249249249ULL;

                int in_piu = __builtin_popcountll(merge_aggiunte);

                uint64_t appena_insoddisfatte = s & (~sodd_new);
                uint64_t merge_tolte = (appena_insoddisfatte | (appena_insoddisfatte >> 1) | (appena_insoddisfatte >> 2)) & 0x9249249249249249ULL;

                int in_meno = __builtin_popcountll(merge_tolte);

                score += (in_piu - in_meno);
            }

            score_parziali[var] += score;
        }
    }

    // Ora sceglo il letterale migliore tra quelli della clausola
    const int16_t* clausola = blocco_corrente.clausole[indice_clausola];

    int max_score = -100;
    std::vector<int16_t> candidati;

    for (int i = 0; i < CLAUSOLA_SIZE; ++i) {
        int16_t lit = clausola[i];
        int var = std::abs(lit);
        int score = score_parziali[var];

        if (score > max_score) {
            max_score = score;
            candidati.clear();
            candidati.push_back(lit);
        } else if (score == max_score) {
            candidati.push_back(lit); // Gestione pareggio
        }
    }

    if (candidati.empty()) return 0;

    // Restituisco uno a caso tra i migliori
    return candidati[std::rand() % candidati.size()];
}

// Funzione che salva in un array le clausole UNSAT
std::vector<std::pair<size_t, int>> clausole_unsat(std::vector<Blocco>& blocchi) {
    std::vector<std::pair<size_t, int>> unsat;

    for (size_t blocco_idx = 0; blocco_idx < blocchi.size(); blocco_idx++) {
        const Blocco& blocco = blocchi[blocco_idx];
        uint16_t M = blocco.num_clausole;

        for (uint16_t i = 0; i < M; ++i) {
            int bit_index = 3 * i;
            int word_index = bit_index / 64;
            int bit_offset = bit_index % 64;

            uint64_t word = blocco.soddisfatte[word_index];
            uint8_t bits = (word >> bit_offset) & 0b111;

            if (bits == 0) {
                unsat.emplace_back(blocco_idx, i);
            }
        }
    }

    return unsat;
}

// Funzione che conta i letterali in comune 
int conta_letterali_in_comune(const int16_t* titolo_blocco, uint16_t num_letterali, const int16_t clausola[CLAUSOLA_SIZE]) {
    int count = 0;
    for (int i = 0; i < CLAUSOLA_SIZE; ++i) {
        for (uint16_t j = 0; j < num_letterali; ++j) {
            if (clausola[i] == titolo_blocco[j]) {
                count++;
                break;
            }
        }
    }
    return count;
}

// Funzione che raggruppa le clausole in blocchi
void raggruppa_clausola_in_blocchi(const std::array<int16_t, CLAUSOLA_SIZE>& clausola_const, std::vector<Blocco>& blocchi, std::unordered_map<int16_t, std::vector<size_t>>& letterale_to_blocchi, const std::vector<int>& assignment, int max_letterali, int max_clausole) {
    int16_t clausola[CLAUSOLA_SIZE];
    for (int i = 0; i < CLAUSOLA_SIZE; i++) {
        clausola[i] = clausola_const[i];
    }

    bool inserita = false;

    // Provo a inserirla in un blocco esistente
    for (size_t idx = 0; idx < blocchi.size(); idx++) {
        Blocco& blocco = blocchi[idx];
                       
        if (blocco.num_clausole >= max_clausole) continue;

        int in_comune = conta_letterali_in_comune(blocco.titolo, blocco.num_letterali, clausola);

        if (in_comune >= 2 && blocco.num_letterali < max_letterali) {
            inserisci_clausola(blocco, clausola, assignment, letterale_to_blocchi, idx);
            inserita = true;
            break;
        }
    }

    // Se non trovo un blocco compatibile, ne creo uno nuovo
    if (!inserita) {
        blocchi.emplace_back();
        size_t nuovo_idx = blocchi.size() - 1;

        Blocco& nuovo_blocco = blocchi[nuovo_idx];
        nuovo_blocco.num_clausole = 0;
        nuovo_blocco.num_letterali = 0;

        for (int i = 0; i < MAX_N; ++i) {
            nuovo_blocco.titolo[i] = 0;
            nuovo_blocco.next_per_letterale[i] = nullptr;
            for (int w = 0; w < MAX_WORDS; ++w)
                nuovo_blocco.bitvector[i][w] = 0ULL;
        }

        for (int w = 0; w < MAX_WORDS; ++w)
            nuovo_blocco.soddisfatte[w] = 0ULL;


        inserisci_clausola(nuovo_blocco, clausola, assignment, letterale_to_blocchi, nuovo_idx);
    }
}

// Funzione che restituisce true se la clausola idx nel blocco è soddisfatta, false altrimenti
bool clausola_soddisfatta(const Blocco& blocco, int idx) {
    int bit_index = 3 * idx;
    int word_index = bit_index / 64;
    int bit_offset = bit_index % 64;

    uint64_t word = blocco.soddisfatte[word_index];
    uint8_t valore = (word >> bit_offset) & 0b111; 

    return valore > 0;
}


void costruisci_catene(std::vector<Blocco>& blocchi, int num_vars) {
    std::vector<Blocco*> primo_blocco_per_var(num_vars + 1, nullptr);
    std::vector<Blocco*> ultimo_blocco_per_var(num_vars + 1, nullptr);
    std::vector<int> ultimo_idx_per_var(num_vars + 1, -1);

    for (Blocco& b : blocchi) {
        std::unordered_set<int> variabili_elaborate;
        
        for (uint16_t j = 0; j < b.num_letterali; ++j) {
            int16_t lit = b.titolo[j];
            int var = std::abs(lit);
            
            if (variabili_elaborate.count(var)) continue;
            variabili_elaborate.insert(var);
            
            if (primo_blocco_per_var[var] == nullptr) {
                primo_blocco_per_var[var] = &b;
            } else {
                Blocco* prev = ultimo_blocco_per_var[var];
                int prev_idx = ultimo_idx_per_var[var];
                prev->next_per_letterale[prev_idx] = &b;
            }
            
            ultimo_blocco_per_var[var] = &b;
            ultimo_idx_per_var[var] = j;
        }
    }
    
    for (int v = 1; v <= num_vars; ++v) {
        if (primo_blocco_per_var[v] != nullptr && ultimo_blocco_per_var[v] != nullptr) {
            Blocco* last = ultimo_blocco_per_var[v];
            int last_idx = ultimo_idx_per_var[v];
            last->next_per_letterale[last_idx] = primo_blocco_per_var[v];
        }
    }
}

void costruisci_catene_vecchio(std::vector<Blocco>& blocchi) {
    std::unordered_map<int16_t, Blocco*> primo_blocco_per_letterale;
    std::unordered_map<int16_t, Blocco*> ultimo_blocco_per_letterale;

    // collego ciascun blocco al successivo per ogni letterale
    for (Blocco& b : blocchi) {
        for (uint16_t j = 0; j < b.num_letterali; ++j) {
            int16_t lit = b.titolo[j];

            if (!primo_blocco_per_letterale.count(lit)) {
                // primo blocco per questo letterale
                primo_blocco_per_letterale[lit] = &b;
            } else {
                Blocco* prev = ultimo_blocco_per_letterale[lit];
                // trovo la posizione di lit nel titolo del blocco precedente
                for (uint16_t k = 0; k < prev->num_letterali; ++k) {
                    if (prev->titolo[k] == lit) {
                        prev->next_per_letterale[k] = &b;
                        break;
                    }
                }
            }

            // aggiorno ultimo blocco per questo letterale
            ultimo_blocco_per_letterale[lit] = &b;
        }
    }

    for (auto& [lit, first] : primo_blocco_per_letterale) {
        Blocco* last = ultimo_blocco_per_letterale[lit];
        for (uint16_t j = 0; j < last->num_letterali; ++j) {
            if (last->titolo[j] == lit) {
                last->next_per_letterale[j] = first;
                break;
            }
        }
    }

    int idx_blocco = 0;
    for (Blocco& b : blocchi) {
        for (uint16_t j = 0; j < b.num_letterali; ++j) {
            if (b.next_per_letterale[j]) {
                // trovo indice del blocco successivo
                int next_idx = -1;
                for (size_t k = 0; k < blocchi.size(); ++k) {
                    if (&blocchi[k] == b.next_per_letterale[j]) {
                        next_idx = static_cast<int>(k);
                        break;
                    }
                }
            }
        }
        idx_blocco++;
    }
}


// Funzione parser che legge il file di input e inserisce la clausola nei blocchi
void parser_cnf(const std::string& filepath, std::vector<Blocco>& blocchi, std::unordered_map<int16_t, std::vector<size_t>>& letterale_to_blocchi, const std::vector<int>& assignment, int max_letterali, int max_clausole) {
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Impossibile aprire il file: " + filepath);
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == 'c') continue;  // commenti
        if (line[0] == 'p') continue;                 // header

        std::istringstream iss(line);
        std::array<int16_t, CLAUSOLA_SIZE> clausola{};
        int16_t lit;
        int idx = 0;

        while (iss >> lit && lit != 0) {
            if (idx >= CLAUSOLA_SIZE) {
                throw std::runtime_error("\nClausola troppo lunga (> CLAUSOLA_SIZE)");
            }
            clausola[idx++] = lit;
        }

        raggruppa_clausola_in_blocchi(
            clausola,
            blocchi,
            letterale_to_blocchi,
            assignment,
            max_letterali,
            max_clausole
        );
    }
}


// Funzione che legge num_vars e num_clasues dal file di input
std::pair<int, int> leggi_header_cnf(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("\nImpossibile aprire file: " + filepath);
    }

    std::string line;
    int num_vars = 0, num_clauses = 0;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == 'c') continue;

        if (line[0] == 'p') {
            std::istringstream iss(line);
            std::string tmp1, tmp2;
            iss >> tmp1 >> tmp2 >> num_vars >> num_clauses;
            break;
        }
    }

    if (num_vars == 0 || num_clauses == 0) {
        throw std::runtime_error("\nHeader CNF non trovato o invalido");
    }

    return {num_vars, num_clauses};
}
