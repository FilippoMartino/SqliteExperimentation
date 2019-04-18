#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <errno.h>
#include <sqlite3.h>

//specificare il percorso
#define DB_FOLDER "db/"
#define HTML_FOLDER "html/"
//speccificare il nome del file temporaneo che verrà composto
#define TMP_FILE_NAME "html/temp.html"

//grandezza massima buffer gestione regex
#define MAX_ERROR_MSG 0x1000

//struttura per crezione tabella
typedef struct {

  //stringa con query
  char* query;
  //posizione della query nel file originale
  int query_index;

}QUERY_INFO;


//Prototipi

static int compile_regex (regex_t *, const char *);
void execute_query(sqlite3*, char*);
int callback(void *, int, char **, char **);
int null_object();
char* get_db_name(char*);
QUERY_INFO* get_query(char*);
void make_table(char*, QUERY_INFO*);
int get_file_size(char*);
void remove_range_from_file(char*, int, int);

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

  //Prelevo i nomi del db concatenandone il nome al percorso
  //Attenzione allo strdup
  char* db_name = strcat(strdup(DB_FOLDER), strdup(get_db_name(TMP_FILE_NAME)));

  sqlite3* my_db;

  if (sqlite3_open(db_name, &my_db)){
    printf("Impossibile accedere al database: %s\n", sqlite3_errmsg(my_db));
    return -1;
  }else
    printf("Database aperto con successo\n");

  int ret;
  char* error_message = 0;

  QUERY_INFO* my_query_info = get_query(TMP_FILE_NAME);
  printf("query: %s\n", my_query_info->query);

  /*
    Eseguo la query con la funzione sqlite3_exec(), che pretende:

    my_db:    oggetto database aperto
    sql:      stringa con query da eseguire
    callback: funzione che riceverà i dati "protagonisti" della query, funziona
              per le interrogazioni, non per la creazione di tabelle e per l'inserimento
              di valori

  */

  execute_query(my_db, my_query_info->query);

  sqlite3_free(error_message);
  sqlite3_close(my_db);
  //


  return 0;

}

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

  /*
    L'idea sarebbe quella di intercettare la stringa contentente il nome
    del db a cui connetterci e successivamente eliminare la riga con il
    corrispondente <sql> dal file temporaneo che abbiamo creato

  */
char* get_db_name(char* file_path){

    FILE* file = fopen(file_path, "r");
    //buffer per lettura sequenziale del file
    int file_size = get_file_size(file_path);
    char buffer[file_size + 1];

    //Copio il file in un buffer
    fread(buffer, file_size, sizeof(char), file);

    //chiudo il file
    fclose(file);

    //Passiamo adesso alla ricerca del nome del database

    //regex in cui verrà compilato il comando
    regex_t regex;
    //il buffer lo preferisco come puntatore
    char* file_content = strdup(buffer);;
    //comando contenente la regex in formato stringa
    //quella di tosello -> "<sql(\\s+database=(.+))?\\s+query=(.*;)\\s*\\/>"
    /*
      Regex fatta da me: cerca un carattere sql, successivamente accetta
      un numero indefinito di spazi che possono esserci come non esserci (\s*)
      poi cerca la parola chiave 'database=', dopo di essa sono accettati un numero
      indefinito di spazii che possono esserci come non esserci;
      nelle parentesi tonde c'è ciò che desideriamo intercettare: il nome del Database
      l'espressione [^ ]* significa "accetta qualsiasi carattere che non sia uno spazio"
      in modo da evitarci problemi di spazi dopo il nome del db; accetto successivamente un
      numero indefinito si spazi fino al /> che chiude il tag
    */
    char* regex_text = "<sql\\s*database=\\s*([^ ]*)\\s*\\/>";

    //procediamo alla compilazione della regex
    compile_regex(&regex, regex_text);

    //numero dei matches che consentiamo di trovare
    int n_matches = 10;
    /*
      vettore con i matches effettivi, struttura:

      regmatch_t{

        int rm_so; puntatore all'inizio dell'occorrenza
        int rm_eo; puntatore alla fine dell'occorrenza
      }

    */
    regmatch_t matches_array[n_matches];
    //eseguiamo la regex
    regexec(&regex, file_content, n_matches, matches_array, 0);

    //ottengo il nome del db dal secondo gruppo
    char *result;
    //alloco result
    result = (char*)malloc(matches_array[1].rm_eo - matches_array[1].rm_so);
    //copio, partendo dalla posizione del buffer interessata, la lunghezza del nome del db
    strncpy(result, &buffer[matches_array[1].rm_so], matches_array[1].rm_eo - matches_array[1].rm_so);
    char* db_name = strdup(result);
    //deallochiamo la stringa di cortesia
    free(result);

    //provvediamo adesso a rimuovere la linea dal file
    remove_range_from_file(file_path, matches_array[0].rm_so, matches_array[0].rm_eo);

    return db_name;
  }

QUERY_INFO* get_query(char* file_path){

  FILE* file = fopen(file_path, "r");
  //buffer per lettura sequenziale del file
  int file_size = get_file_size(file_path);
  char buffer[file_size + 1];
  //Copio il file in un buffer
  fread(buffer, file_size, sizeof(char), file);
  //chiudo il file
  fclose(file);

  //Passiamo adesso alla ricerca della query

  //regex in cui verrà compilato il comando
  regex_t regex;
  //il buffer lo preferisco come puntatore
  char* file_content = strdup(buffer);
  //testo della regex
  char* regex_text = "<sql\\s*query=\\s*(.*;)\\s*\\/>";

  //procediamo alla compilazione della regex
  compile_regex(&regex, regex_text);
  int n_matches = 10;

  regmatch_t matches_array[n_matches];
  //eseguiamo la regex
  regexec(&regex, file_content, n_matches, matches_array, 0);

  //ottengo il nome del db dal secondo gruppo
  char *result;
  //alloco result
  result = (char*)malloc(matches_array[1].rm_eo - matches_array[1].rm_so);
  //copio, partendo dalla posizione del buffer interessata, la lunghezza del nome del db
  strncpy(result, &buffer[matches_array[1].rm_so], matches_array[1].rm_eo - matches_array[1].rm_so);
  char* query_name = strdup(result);
  //deallochiamo la stringa di cortesia
  free(result);

  //provvediamo adesso a rimuovere la linea dal file
  remove_range_from_file(file_path, matches_array[0].rm_so, matches_array[0].rm_eo);
  //compiliamo la struttura e restituiamola
  QUERY_INFO* info = (QUERY_INFO*) malloc(sizeof(QUERY_INFO));
  info->query = strdup(query_name);
  printf("query: %s\n", info->query);
  info->query_index = matches_array[0].rm_so;

  return info;

}

void make_table(char* file_path, QUERY_INFO* query_info){

    //estrapoliamo i dati dalla struttura
    char* query = query_info->query;
    int sql_index = query_info->query_index;
    //deallochiamo lo spazio in memoria
    free(query_info);
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

int get_file_size(char* file_path){

  //apro il file in modalità lettura
  FILE* file = fopen(file_path, "r");

  //controlliamo che il file passato dall'utente esista effettivamente
  if (file == NULL) {
    printf("File Not Found!\n");
    return -1;
  }

    /*
    per utilizzo di questa fuzione vedre es calcolare_dimesione_file
    nella cartella esercizi
    */

    fseek(file, 0L, SEEK_END);

    // calcoliamo la grandezza del file passato mediante ftell()
    int file_size = ftell(file);

    // chiudo il file
    fclose(file);

    return file_size;

}

/*

  Questa funzione si occupa di rimuovere la sezione di file in cui abbiamo trovato un
  ta sql, parametri:

  char* path:   Percorso del file da cui rimuovere la sezione
  int start:    Inizio della sezione da rimuovere
  int end:      Fine della sezione da rimuovere

*/
void remove_range_from_file(char* path, int start, int end){

  //prima copio il file in un buffer:
  int file_size = get_file_size(path);
  char buffer[file_size];
  FILE* to_close = fopen(path, "r");
  fread(buffer, file_size, sizeof(char), to_close);
  fclose(to_close);
  FILE* file = fopen(path, "w");
  for(int i = 0; i < file_size; i++){
    if(i < start || i > end)
      fputc(buffer[i], file);
  }
  fclose(file);
}
