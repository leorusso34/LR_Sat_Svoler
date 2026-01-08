# LR_Sat_Svoler - ITA
SAT Solver - Local Search & Bitwise Optimization
Progetto di tesi di Leonardo Russo (Università di Ferrara).

Come usare il risolutore

COMPILAZIONE: Esegui *make* per automatizzare la compilazione di Main.cc e clause_functions.cc tramite il compilatore GCC.
PULIZIA: Esegui *make clean* per eliminare l'eseguibile e i file oggetto prodotti.
ESECUZIONE: Utilizza il comando *./sat_solver <nome_file.cnf>* per avviare la ricerca locale sull'istanza CNF fornita.

Dettagli Tecnici

Architettura: Basato su Local Search con organizzazione dati a blocchi per minimizzare i cache miss e ottimizzare la località in memoria.
Bitwise Optimization: Uso di bitvector e operazioni bitwise per l'aggiornamento costante del satcount delle clausole.

*Tutti i diritti riservati. Codice pubblicato a scopo puramente illustrativo.*






# LR_Sat_Svoler - ENG
SAT Solver - Local Search & Bitwise Optimization
Thesis project by Leonardo Russo (University of Ferrara).

How to use the solver

COMPILE: Run *make* to automate the compilation of Main.cc and clause_functions.cc using the GCC compiler.
CLEAN: Run *make clean* to delete the executable and the object files produced.
RUN: Use the command ./sat_solver <filename.cnf> to start local search on the provided CNF instance.

Technical Details

Architecture: Based on Local Search with block-based data organization to minimize cache misses and optimize memory locality.
Bitwise Optimization: Uses bitvectors and bitwise operations to constantly update the clause satcount.

*All rights reserved. Code published for illustrative purposes only.*
