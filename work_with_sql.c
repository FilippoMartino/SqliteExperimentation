#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <sqlite3.h>

int main(int argc, char const *argv[]) {

  if(argc < 2){
    printf("ERROR: USAGE %s DB_NAME\n", argv[0]);
    return -1;
  }

  char* db_name = strdup(argv[1]);
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

  if (sqlite3_open(db_name, &my_db))
    printf("Impossibile accedere al database: %s\n", sqlite3_errmsg(my_db));
  else
    printf("Database aperto con successo\n");


  sqlite3_close(my_db);

  return 0;
}
