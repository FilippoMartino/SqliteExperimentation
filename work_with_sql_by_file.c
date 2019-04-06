#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sqlite3.h>

//specificare il percorso
#define DB_FOLDER "db/"
//speccificare il nome del file temporaneo che verrà composto
#define TMP_FILE_NAME "temp.html"

int callback(void *query_result, int cells_number, char **rows, char **rows_index) {

   for(int i = 0; i < cells_number; i++) {
      //se nella cella è presente un dato lo stampa, altrimenti inserisce NULL
      printf("%s: %s\n", rows_index[i], rows[i] ? rows[i] : "NULL");
   }
   printf("\n");
   return 0;
}

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

/*
  L'idea sarebbe quella di intercettare la stringa contentente il nome
  del db a cui connetterci e successivamente eliminare la riga con il
  corrispondente <sql> dal file temporaneo che abbiamo creato

*/
char* get_db_name(char* file_name){}

/*
  L'idea sarebbe quella di, una volta preso il nome del file da parsificare,
  crearne una copia (il classico temp) e esaudire le richieste nei tag <sql>
  specificate nel file originale
*/
int main(int argc, char const *argv[]) {

  if(argc < 2){
    printf("ERROR: USAGE %s FILE_NAME\n", argv[0]);
    return -1;
  }

  char* file_name = strdup(argv[1]);
  sqlite3* my_db;
  File* original_file = fopen(file_name, 'r');
  File* dest_file = fopen(strdup(TMP_FILE_NAME), 'w');
  char ch;

  while ((ch = fgetc(original_file)) != EOF)
      fputc(ch, target);










  if (sqlite3_open(db_name, &my_db)){
    printf("Impossibile accedere al database: %s\n", sqlite3_errmsg(my_db));
    return -1;
  }else
    printf("Database aperto con successo\n");



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
