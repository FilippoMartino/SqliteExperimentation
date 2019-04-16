#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sqlite3.h>

//specificare il percorso
#define DB_FOLDER "db/"
#define HTML_FOLDER "html/"
//speccificare il nome del file temporaneo che verrà composto
#define TMP_FILE_NAME "html/temp.html"

//grandezza massima buffer gestione regex 
#define MAX_ERROR_MSG 0x1000

/*
  Funzione che facilita la compilazione di una regex
  gestisce gli errori e ritorna un valore
*/
static int compile_regex (regex_t * r, const char * regex_text){

    int status = regcomp(r, regex_text, REG_EXTENDED|REG_NEWLINE);

    if (status != 0) { 
	    char error_message[MAX_ERROR_MSG];
	    regerror (status, r, error_message, MAX_ERROR_MSG);
      printf ("Regex error compiling '%s': %s\n",
              regex_text, error_message);
            return -1;
    }
    
    return 0;

  }
  
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
char* get_db_name(char* file_path){

  FILE* file = fopen(file_path, "r");
  //buffer per lettura sequenziale del file
  char buffer[512];
  
  //string in cui mettiamo il contenuto da trovare
  char* match;

  //facciamo dei cicli per 512 caratteri finchè il file non finisce
  while(fgets(buffer, 512, file) != NULL) {
     match = strstr(buffer, "CONNECT TO");
     //se troviamo quello che stiamo cercando usciamo
     if(match != NULL)
        break;
  }

  //chiudiamo il file
  fclose(file);

  if (match == NULL){
    printf("Si è verificato un problema durante la ricerca del nome del db\n");
    return NULL;
  }
  
  //Passiamo adesso alla ricerca del nome del database
  //regex in cui verrà compilato il comando
  regex_t regex;
  //comando contenente la regex in formato stringa
  char* regex_text = "<sql(\s+database=(.+))?\s+query=(.*;)\s*\/>";
  
  //procediamo alla compilazione della regex
  compile_regex(&regex, regex_text);
  
  //numero dei matches che consentiamo di trovare
  int n_matches;
  //vettore con i matches effettivi
  regmatch_t m[n_matches];
  //eseguiamo la regex
  regexec(r, p, n_matches, m, 0);
  
  
  
  
  
  
  
  
  

  //essendo il nome del db preceduto dai caratteri CONNECT TO sposto di 11 il puntatore
  char* temp = strdup(match) + 11;
  //variabile in cui vado ad inserire il nome del db (può essere lungo al massimo 100 caratteri)
  char db_name[100];

     //scorro l'array con il nome del db finchè non incorro in uno spazio
     for (int i = 0; (temp[i]) != ' ' && (temp[i] != '<'); i++){
        //DEBUG
        printf("match[%d]: %c\n", i, temp[i]);
        db_name[i] = temp[i];

     }

  //vado a restituire il puntatore
  return strdup(db_name);


}

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

  //concateno il nome del file passatoci con il percorso
  char* file_name = strcat(strdup(HTML_FOLDER),strdup(argv[1]));
  sqlite3* my_db;
  //creo il primo file
  FILE* original_file = fopen(file_name, "r");

  if (original_file == NULL){
    printf("Attenzione, il file inserito non esiste\n");
    return -1;
  }

  //dopo aver'verificato che nel primo ci sia qualcosa apro il secondo
  FILE* temp_file = fopen(strdup(TMP_FILE_NAME), "w");
  char ch;

  if (temp_file == NULL){
    printf("Attenzione, si è verificato un errore con il sistema, contattare il programmatore\n");
    return -1;
  }

  while ((ch = fgetc(original_file)) != EOF)
      fputc(ch, temp_file);

  //non avendone più bisogno, chiudo sia il file originale
  fclose(original_file);
  //che il temporaneo, spetterà alle funzioni cambiarlo
  fclose(temp_file);
  char* db_name;

  //se ho trovato il nome del db lo stampo altrimenti termino il programma
  if (get_db_name(TMP_FILE_NAME) != NULL)
    db_name = get_db_name(TMP_FILE_NAME);
  else
    return -1;

  printf("db_name: %s\n", db_name);



  //
  // if (sqlite3_open(db_name, &my_db)){
  //   printf("Impossibile accedere al database: %s\n", sqlite3_errmsg(my_db));
  //   return -1;
  // }else
  //   printf("Database aperto con successo\n");
  //
  //
  //
  // int ret;
  // char* error_message = 0;
  // char* sql;
  //
  // //query per la creazione di una tabella (uso la drop visto che ricompilo ed eseguo il codice molte votle)
  // sql = "DROP TABLE Famiglia;" \
  //       "CREATE TABLE Famiglia("  \
  //       "id INT PRIMARY KEY NOT NULL, " \
  //       "nome VARCHAR(50) NOT NULL," \
  //       "cognome VARCHAR(50) NOT NULL," \
  //       "anni INT NOT NULL);";
  //
  // /*
  //     Eseguo la query con la funzione sqlite3_exec(), che pretende:
  //
  //     my_db:    oggetto database aperto
  //     sql:      stringa con query da eseguire
  //     callback: funzione che riceverà i dati "protagonisti" della query, funziona
  //               per le interrogazioni, non per la creazione di tabelle e per l'inserimento
  //               di valori
  // */
  //
  // ret = sqlite3_exec(my_db, sql, null_object, 0, &error_message);
  //
  // if( ret != SQLITE_OK ){
  //     printf("Errore durante la creazione della tabella: %s\n", error_message);
  //     sqlite3_free(error_message);
  //     return -1;
  //  } else
  //     printf("Tabella creata con successo\n");
  //
  // /*
  //   Andiamo adesso ad inserire alcuni valori all'interno della Tabella
  //   (nella query che si compone possono essere inseriti più valori)
  // */
  //
  // sql =  "INSERT INTO Famiglia (id, nome, cognome, anni) "  \
  //        "VALUES (1, 'Filippo', 'Martino', 18); " \
  //        "INSERT INTO Famiglia (id, nome, cognome, anni) "  \
  //        "VALUES (2, 'Mario', 'Martino', 50); "
  //        "INSERT INTO Famiglia (id, nome, cognome, anni) "  \
  //        "VALUES (3, 'Laura', 'Pagge', 50); "
  //        "INSERT INTO Famiglia (id, nome, cognome, anni) "  \
  //        "VALUES (4, 'Anna', 'Martino', 18); ";
  //
  // ret = sqlite3_exec(my_db, sql, null_object, 0, &error_message);
  //
  // if( ret != SQLITE_OK ){
  //    printf("Errore durante l'inserimento dei dati nella tabella: %s\n", error_message);
  //    sqlite3_free(error_message);
  //    return -1;
  // } else
  //   printf("Dati inseriti con successo\n");
  //
  // /*  Proviamo a fare un paio di interrogazioni   */
  //
  // sql = "SELECT * FROM Famiglia";
  // printf("==========Selezione di tutti gli elementi==========\n\n");
  // execute_query(my_db, sql);
  // sql = "SELECT * FROM Famiglia WHERE anni>30";
  // printf("==========Selezione di tutti i record con con più di 30 anni==========\n\n");
  // execute_query(my_db, sql);
  // sql = "SELECT Famiglia.cognome FROM Famiglia WHERE anni > 30";
  // printf("==========Selezione dei cognomi delle persone con più di 30 anni==========\n\n");
  // execute_query(my_db, sql);
  // sql = "SELECT Famiglia.nome FROM Famiglia WHERE anni<30";
  // printf("==========Selezione di tutti ci nomi delle persone con meno di 30 anni==========\n\n");
  // execute_query(my_db, sql);
  //
  //
  // sqlite3_free(error_message);
  // sqlite3_close(my_db);
  //


  return 0;
}
