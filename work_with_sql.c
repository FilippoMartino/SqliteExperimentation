#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sqlite3.h>

#define DB_FOLDER "db/"

/*
  Questa funzione serve per stampare il risultato della chiamata alla funzoione
  sqlite3_exec, ecco com'è strutturata:

   void*:   Data provided in the 4th argument of sqlite3_exec()
   int:     The number of columns in row
   char**:  An array of strings representing fields in the row
   char**:  An array of strings representing column names

   La funzione riceve comunque solo una riga di database per volta
);
*/
int callback(void *query_result, int cells_number, char **rows, char **rows_index) {

   for(int i = 0; i < cells_number; i++) {
      //se nella cella è presente un dato lo stampa, altrimenti inserisce NULL
      printf("%s: %s\n", rows_index[i], rows[i] ? rows[i] : "NULL");
   }
   printf("\n");
   return 0;
}

/*
  Uso questa funzione per ricevere i risultati (insistenti) della query
  per le operazioni di creazione Tabella, in questo modo ottimizzo
  l'esecuizione del codice

*/
int null_object(){  return 0;  }

void execute_query(sqlite3* my_db, char* sql){

  char* error_message = 0;
  int ret = sqlite3_exec(my_db, sql, callback, 0, &error_message);

  if( ret != SQLITE_OK ){
      printf("Errore durante l'interrogazione: %s\n", error_message);
      sqlite3_free(error_message);
   } else

  sqlite3_free(error_message);


}

int main(int argc, char const *argv[]) {

  if(argc < 2){
    printf("ERROR: USAGE %s DB_NAME\n", argv[0]);
    return -1;
  }

  /*
    Il codice sottostante concatena il nome del db fornitoci
    dall'utente con la cartella in cui soncognomeo organizzati i database
  */

  char* db_name = strcat(strdup(DB_FOLDER), strdup(argv[1]));
  //dichiarazione del database
  sqlite3* my_db;

  /*
    Essendo la libreria per lavorare con sqlite3 programmata in case c
    non necessitiamo di istanziare degli oggetti, le funzioni che usiamo
    per aprire un db e gestire possibili errori sono:

    sqlite3_open():   il primo parametro è il nome del database che verrà aperto
                      (se non trova nessun db con questo nome lo crea)
    sqlite3_errmsg(): è una verà e propria libreria "errno-like" che gestisce
                      tutti i possibili errori che possono durante l'operazione
                      di apertura, il parametro da pasargli è il db che abbiamo
                      dichiarato in precedenza, che controllrà per gli errori
    sqlite3_close();: Unico parametro il db che verrà chiuso

  */

  if (sqlite3_open(db_name, &my_db)){
    printf("Impossibile accedere al database: %s\n", sqlite3_errmsg(my_db));
    return -1;
  }else
    printf("Database aperto con successo\n");

  /*
    Ci prepariamo adesso ad inserire/manipolare e stampare
    sul database, necessitiamo:

    ret:  variabile per intercettare i valori di ritorno delle chiamate alle
          funzioni di modifica del db
    error_message: variabile da passare per referenza alle varie funzioni in cui
                   viene salvata la possibile tipologia d'errore generata
                   DEVE essere deistanziato con mediante la funzione
                   sqlite3_free()
    sql:  stringa con la query da eseguire sul db
  */

  int ret;
  char* error_message = 0;
  char* sql;

  //query per la creazione di una tabella (uso la drop visto che ricompilo ed eseguo il codice molte votle)
  sql = "DROP TABLE Famiglia;" \
        "CREATE TABLE Famiglia("  \
        "id INT PRIMARY KEY NOT NULL, " \
        "nome VARCHAR(50) NOT NULL," \
        "cognome VARCHAR(50) NOT NULL," \
        "anni INT NOT NULL);";

  /*
      Eseguo la query con la funzione sqlite3_exec(), che pretende:

      my_db:    oggetto database aperto
      sql:      stringa con query da eseguire
      callback: funzione che riceverà i dati "protagonisti" della query, funziona
                per le interrogazioni, non per la creazione di tabelle e per l'inserimento
                di valori
  */

  ret = sqlite3_exec(my_db, sql, null_object, 0, &error_message);

  if( ret != SQLITE_OK ){
      printf("Errore durante la creazione della tabella: %s\n", error_message);
      sqlite3_free(error_message);
      return -1;
   } else
      printf("Tabella creata con successo\n");

  /*
    Andiamo adesso ad inserire alcuni valori all'interno della Tabella
    (nella query che si compone possono essere inseriti più valori)
  */

  sql =  "INSERT INTO Famiglia (id, nome, cognome, anni) "  \
         "VALUES (1, 'Filippo', 'Martino', 18); " \
         "INSERT INTO Famiglia (id, nome, cognome, anni) "  \
         "VALUES (2, 'Mario', 'Martino', 50); "
         "INSERT INTO Famiglia (id, nome, cognome, anni) "  \
         "VALUES (3, 'Laura', 'Pagge', 50); "
         "INSERT INTO Famiglia (id, nome, cognome, anni) "  \
         "VALUES (4, 'Anna', 'Martino', 18); ";

  ret = sqlite3_exec(my_db, sql, null_object, 0, &error_message);

  if( ret != SQLITE_OK ){
     printf("Errore durante l'inserimento dei dati nella tabella: %s\n", error_message);
     sqlite3_free(error_message);
     return -1;
  } else
    printf("Dati inseriti con successo\n");

  /*  Proviamo a fare un paio di interrogazioni   */

  sql = "SELECT * FROM Famiglia";
  printf("==========Selezione di tutti gli elementi==========\n\n");
  execute_query(my_db, sql);
  sql = "SELECT * FROM Famiglia WHERE anni>30";
  printf("==========Selezione di tutti i record con con più di 30 anni==========\n\n");
  execute_query(my_db, sql);
  sql = "SELECT Famiglia.cognome FROM Famiglia WHERE anni > 30";
  printf("==========Selezione dei cognomi delle persone con più di 30 anni==========\n\n");
  execute_query(my_db, sql);
  sql = "SELECT Famiglia.nome FROM Famiglia WHERE anni<30";
  printf("==========Selezione di tutti ci nomi delle persone con meno di 30 anni==========\n\n");
  execute_query(my_db, sql);


  sqlite3_free(error_message);
  sqlite3_close(my_db);



  return 0;
}
