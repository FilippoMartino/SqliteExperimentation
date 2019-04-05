#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sqlite3.h>

#define DB_FOLDER "db/"

/*
  Questa funzione serve per stampare il risultato della chiamata alla funzoione
  sqlite3_exec
*/
int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      //se nella cella è presente un dato lo stampa, altrimenti inserisce NULL
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

int main(int argc, char const *argv[]) {

  if(argc < 2){
    printf("ERROR: USAGE %s DB_NAME\n", argv[0]);
    return -1;
  }

  /*
    Il codice sottostante concatena il nome del db fornitoci
    dall'utente con la cartella in cui sono organizzati i database
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

  //query per la creazione di una tabella
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
      callback: funzione che riceverà i dati "protagonisti" della query, utile
                per ricevere il risultato delle interrogazioni, ma funziona anche
                (come in questo caso) per ricevere i dati dello schema
  */

  ret = sqlite3_exec(my_db, sql, callback, 0, &error_message);

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

  ret = sqlite3_exec(my_db, sql, callback, 0, &error_message);

  if( ret != SQLITE_OK ){
     printf("Errore durante l'inserimento dei dati nella tabella: %s\n", error_message);
     sqlite3_free(error_message);
     return -1;
  } else
    printf("Dati inseriti con successo\n");

  sqlite3_free(error_message);
  sqlite3_close(my_db);

  return 0;
}
