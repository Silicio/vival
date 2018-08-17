#include <iostream>
#include <ctime>
#include "simpleini/SimpleIni.h"
#include "prototype.pro"
#include "version.h"



/** PARAMETRI:
*
*   1)  FILE DEL TRACCIATO DA VALIDARE
*   2)  FILE DI CONFIGURAZIONE DEL TRACCIATO
*/

/** FILE INI:
*
*   TYPE:
*   N   Tipo campo numerico senza decimali
*   Nx  Tipo campo numerico con x decimali
*   A   Tipo campo alfanumerico
*
*   MANDATORY:
*   Y   Il campo è obbligatorio
*   N   Il campo non è obbligatorio
*/

/** CONTROLLI:
*
*   1)  Lunghezza totale tracciato
*/

/** NORMALIZZAZIONE:
*
*   1)  Fill del campo
*   2)  Allineamento a DX o SX
*/

using namespace std;

#define MAX_BUFFER_SIZE     1024
#define MAX_FILENAME_SIZE 	64
#define ON                  1
#define OFF                 0

const char logfile[] = "vival.log";

struct CONFIG {
  char name[32];
  char type[8];
  char len[8];
  char col_pos[8];
  char align[8];
  char function[8];
  char mandatory[8];
  char dec_sep[8];
  int  i_dec_sep;
  char tho_sep[8];
  char filler[8];
};

class check_status{
  int error_flag;
  char error[100];

public:
  check_status(){
    for(error_flag = 99; error_flag != 0; error_flag--) error[error_flag] = '0';
  };

   int set_errors_flag(int check_number){
    if (check_number < 100){
      error_flag = 1;
      error[check_number] = '1';
      return atoi( &error[check_number] );
    }
    else return 0;
  }

  int get_errors_flag(int check_number){
    if (check_number < 100){
      return atoi( &error[check_number] );
    }
    else return 0;
  }

  int get_error_status(){
    return error_flag;
  }

};

int main(int argc, char *argv[]){

/* Variabili di servizio */
  int normalization = OFF;
  int int_columns = 0;
  int i_dec_sep = 0;
  int func_number = 0;
  int i;
  int cur_row_len;
  int row_count;
  int *data = new int[3];
  int interactive = OFF;
  int log_alert = OFF;
  char c;
  char field[8];
  char align[8];
  check_status controllo;

/* Variabili per I/O */
  char message[MAX_BUFFER_SIZE];
  char buffer[MAX_BUFFER_SIZE];
  char input_file[MAX_FILENAME_SIZE];
  char output_file[MAX_FILENAME_SIZE];
  FILE *handler_input_file  = NULL;
  FILE *handler_output_file = NULL;
  FILE *handler_log_file    = NULL;



  if(argc == 2){
    if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "-V") == 0){
      printf("Version: %s\n", AutoVersion::FULLVERSION_STRING);
      exit(EXIT_SUCCESS);
    }
  }

  if (argc != 3){
    cout << endl << "Usage: vival.exe <INPUT_FILE> <CONFIG_FILE>" << endl;
    cout << "Check parameters!" << endl;
    exit(EXIT_FAILURE);
  }

  strcpy( input_file, argv[1] );

  if( ! (handler_input_file = fopen( input_file, "r+" ) ) ) {
		perror(input_file);
		exit(EXIT_FAILURE);
	}

  CSimpleIniA ini;
  ini.SetUnicode();
  if (ini.LoadFile(argv[2]) < 0){
    perror(argv[2]);
    exit(EXIT_FAILURE);
  }


  const char *trk_name      = ini.GetValue("TRACK", "trackname");
  const char *trk_type      = ini.GetValue("TRACK", "tracktype");
  const char *trk_fld_sep   = ini.GetValue("TRACK", "trackfieldsep");
  const char *trk_len       = ini.GetValue("TRACK", "tracklength", "0");
  const char *trk_cols      = ini.GetValue("TRACK", "trackcols");

  const char *def_type      = ini.GetValue("DEFAULTS", "type");
  const char *def_align     = ini.GetValue("DEFAULTS", "alignment");
  const char *def_dec_sep   = ini.GetValue("DEFAULTS", "asciidecimalseparator");
  const char *def_tho_sep   = ini.GetValue("DEFAULTS", "asciithousandseparator");
  const char *def_mandatory = ini.GetValue("DEFAULTS", "mandatory");
  const char *def_filler    = ini.GetValue("DEFAULTS", "asciifiller");

  const int i_def_dec_sep = atoi(def_dec_sep);

  strcpy( output_file, input_file);
  strcat( output_file, ".out");

  if( ! (handler_output_file = fopen( output_file, "w+" ) ) ) {
		perror(output_file);
		goto free_resources;
	}
	clearerr(handler_output_file);

  if( ! (handler_log_file = fopen( logfile, "a+" ) ) ) {
		perror(logfile);
		goto free_resources;
	}
	clearerr(handler_log_file);


  cout << "*******************************************" << endl;
  if (trk_name)       cout << "Track name                 : " << trk_name       << endl;
  if (trk_type)       cout << "Track type                 : " << trk_type       << endl;
  if (trk_fld_sep)    cout << "Track field separator      : " << trk_fld_sep    << endl;
  if (trk_len)        cout << "Track length               : " << trk_len        << endl;
  if (trk_cols)       cout << "Track columns              : " << trk_cols       << endl;
  cout << "*******************************************" << endl;
  if (def_type)       cout << "Default type               : " << def_type       << endl;
  if (def_align)      cout << "Default alignment          : " << def_align      << endl;
  if (def_dec_sep)    cout << "Default decimal separator  : " << def_dec_sep    << endl;
  if (def_tho_sep)    cout << "Default thousand separator : " << def_tho_sep    << endl;
  if (def_mandatory)  cout << "Default mandatory          : " << def_mandatory  << endl;
  if (def_filler)     cout << "Default ascii filler       : " << def_filler     << endl;
  cout << "*******************************************" << endl;

  // TODO: aggiungere gestione parametreica per procedura interattiva.
  if (interactive == ON) {
    cout << endl << endl << "Validation process started. Activate normalization functions ? [y/n]" << endl;

    do{
      cin >> c;
    } while( c != 'Y' && c != 'y' && c != 'N' && c != 'n');

    if ( toupper(c) == 'Y') normalization = ON;
    else normalization = OFF;
  }
  else normalization = ON;

  int_columns = atoi(trk_cols);

  struct CONFIG configuration[int_columns + 1];
  memset(&configuration, '\0', sizeof(struct CONFIG) * (int_columns + 1) );

  //TODO: Inserire controlli sui valori letti da ini.

  /* First block is riserved for default values */
  if( def_align )    strcpy(configuration[0].align      , def_align);
  if( def_dec_sep ){
    strcpy(configuration[0].dec_sep    , def_dec_sep);
    configuration[0].i_dec_sep = atoi(def_dec_sep);
  }
  if( def_tho_sep )  strcpy(configuration[0].tho_sep    , def_tho_sep);
  if( def_type )     strcpy(configuration[0].type       , def_type);
  if( def_filler )   strcpy(configuration[0].filler     , def_filler);
  strcpy(configuration[0].col_pos    , "");
  strcpy(configuration[0].function   , "");
  strcpy(configuration[0].len        , "");
  strcpy(configuration[0].mandatory  , "");
  strcpy(configuration[0].name       , "");


  /* Inizialization configuration fields */
  for (i = 1; i <= int_columns; i++){
    sprintf(field, "FIELD%d", i);
    sprintf(configuration[i].align      , ini.GetValue(field, "alignment"));
    sprintf(configuration[i].col_pos    , ini.GetValue(field, "column_position"));
    //configuration[i].dec_sep = (int) ini.GetValue(field, "asciidecimalseparator");
    sprintf(configuration[i].dec_sep    , ini.GetValue(field, "asciidecimalseparator"));
    if(configuration[i].dec_sep) configuration[i].i_dec_sep = atoi(configuration[i].dec_sep);
    sprintf(configuration[i].function   , ini.GetValue(field, "function"));
    sprintf(configuration[i].len        , ini.GetValue(field, "length"));
    sprintf(configuration[i].mandatory  , ini.GetValue(field, "mandatory"));
    sprintf(configuration[i].filler     , ini.GetValue(field, "asciifiller"));
    sprintf(configuration[i].name       , ini.GetValue(field, "name"));
    sprintf(configuration[i].tho_sep    , ini.GetValue(field, "asciithousandseparator"));
    sprintf(configuration[i].type       , ini.GetValue(field, "type"));
  }
// TODO:Calcolo della lunghezza totale del record da lunghezza singoli campi se 'tracktype = Lunghezza fissa'
// TODO:Calcolo della posizione singolo record tramite la differenza del parziale sulla lunghezza_totale e la lunghezza del campo corrente se 'tracktype = Lunghezza fissa'


/* CICLO ITERAZIONE RIGHE FILE */
  data = getData();
  row_count = 0;
  while ( fgets(buffer, MAX_BUFFER_SIZE, handler_input_file) ) {
    cur_row_len = strlen(buffer) - 1;       // Evito di considerare il carattere di acapo
    row_count++;

/** CONTROLLO SULLE RIGHE */
    if( (trk_len) &&  ( atoi(trk_len) > 0 ) && ( cur_row_len == atoi(trk_len) ) ); // It's all ok!
    else{
      log_alert = ON;
      sprintf(message, "[Riga %00000007d] %s:\t Track length %d instead of %d.\n", row_count, input_file, cur_row_len, atoi(trk_len) );
      fputs(message, handler_log_file);
    }
/*  CONTROLLO SULLE RIGHE */


    for ( i = 1; i <= int_columns; i++ ){


/** NORMALIZZAZIONE */
      if (normalization == ON){

/* ALLINEAMENTO STRINGA */
        strcpy(align, def_align);
        if ( strncmp(configuration[i].align, "SX", 2) == 0 || strncmp(configuration[i].align, "DX", 2) == 0)
          strcpy(align, configuration[i].align);

        allineastringa(&buffer[atoi(configuration[i].col_pos) - 1], atoi(configuration[i].len), align);

/** FUNZIONI */

        i_dec_sep = atoi(configuration[i].dec_sep);
        if (! isprint(i_dec_sep) ) i_dec_sep = atoi(configuration[0].dec_sep);
        if (! isprint(i_dec_sep) ) continue; // No separator No party

        func_number = atoi(configuration[i].function);
        switch(func_number){
          case 101: {
            //strcpy( align, configuration[i].align );
            removeCharFromString( &buffer[atoi(configuration[i].col_pos) - 1],
                                  atoi(configuration[i].len),
                                  (char) i_dec_sep,
                                  align);
            break;  /* Remove char from buffer */
          }

          case 102: {
            shiftStringSxSpostaSegnoDx( &buffer[atoi(configuration[i].col_pos) - 1],
                                                atoi(configuration[i].len),
                                                '-');
            break;
          }

          case 103: {

            shiftStringSxSpostaSegnoDx( &buffer[atoi(configuration[i].col_pos) - 1],
                                                atoi(configuration[i].len),
                                                '-');

            removeCharFromString( &buffer[atoi(configuration[i].col_pos) - 1],
                                  atoi(configuration[i].len),
                                  (char) i_dec_sep,
                                  align);

            break;
          }

          default: break;
        }

/* FUNZIONI **/

/* 1) ========= FILLER ========= */

        int filler;
        char type[1];

        if      ( isprint(configuration[i].filler[0]) ) filler = atoi(configuration[i].filler);//sprintf(filler, "%s", configuration[i].filler);
        else if ( isprint(configuration[0].filler[0]) ) filler = atoi(configuration[0].filler); //sprintf(filler, "%s", configuration[0].filler);//strcpy( filler, configuration[0].filler );
        else continue; /* No filler, no party */

        int j;
        int col_pos   = atoi(configuration[i].col_pos) ;
        int field_len = atoi(configuration[i].len);

        if      ( isprint(configuration[i].type[0]) )   strncpy( type, configuration[i].type, 1 );
        else if ( isprint(configuration[0].type[0]) )   strncpy( type, configuration[0].type, 1 );

        //TODO: Filling nel caso di type A
        if ( strncmp(type, "N", 1) == 0 ){
          for( j = 0; j < field_len; j++ ){
            if( isspace( buffer[col_pos + j - 1] ) )
              buffer[col_pos + j - 1] = (char) filler;
          }
        }
      }
/*  =========       =========*/


/*  NORMALIZZAZIONE **/



/** CONTROLLI SUI CAMPI */


/*  CONTROLLI SUI CAMPI **/

    }
  //  cout << buffer << endl;
      fputs(buffer, handler_output_file);

  }

  if (log_alert == ON) cout << "WARNING: There a new messages on log !" << endl;
  else cout << "Check completed successfully!" << endl;

free_resources:
  if (handler_output_file) fclose(handler_output_file);
  handler_output_file = NULL;

  if (handler_input_file) fclose(handler_input_file);
  handler_input_file = NULL;

  if (handler_log_file) fclose(handler_log_file);
  handler_log_file = NULL;

end_job:
  return EXIT_SUCCESS;
}

/*  ****************************************************************************
    * IMPLEMENTAZIONE DELLE FUNZIONI AUSILIARIE                                *
    ****************************************************************************/

int* getData() {
 time_t data = time(NULL);      //data salvata in data
 tm* tempo = localtime(&data);  //inizializza struttura tm
 int* times = new int[3];       //array che contiene gm, m, a
 times[0] = tempo->tm_mday;     //giorno mese
 times[1] = tempo->tm_mon;      //mese
 times[2] = tempo->tm_year;     //anno

return times;
}

/*  ****************************************************************************
    * IMPLEMENTAZIONE DELLE FUNZIONI DA FILE INI                               *
    ****************************************************************************/

int removeCharFromString(char *string, int length, char ch, char *align){
  int i = 0;
  int j = 0;
  int counter = 0;
  //char buf101[MAX_BUFFER_SIZE];

  //strcpy(buf101, string);
 // memset(buf101, ' ', MAX_BUFFER_SIZE);
 // buf101[MAX_BUFFER_SIZE - 1] = '\0';

  if ( strcmp("DX", align ) == 0){
    /* Right alignment */
    for(j = i = length - 1; i >= 0; i--){
      //if( string[i] != ch) buf101[j--] = string[i];
      if( string[i] != ch) string[j--] = string[i];
      else counter++;
    }
    for(j = counter ; j >= 0; j--){
      string[j] = ' ';
    }
  }
  else{
    /* Left alignment */
    for(i = 0; i < MAX_BUFFER_SIZE && i < length; i++){
      if( string[i] != ch) string[j++] = string[i];
      else counter++;
    }
    for(j = length - counter - 1; j < length; j++){
      string[j] = ' ';
    }
  }

  //strcpy(string, buf101);

  return counter;
}



int shiftStringSxSpostaSegnoDx(char *buffer, int length, char ch){
  int i;
  int marker = OFF;

  if ( isalnum(buffer[0]) ){
    buffer[length - 1] = ' ';
    return EXIT_SUCCESS;
  }

  for( i = 0; i < length - 1; i++ ){

    if(buffer[i] == ch) marker = ON;

    if (buffer[i + 1] != ch) buffer[i] = buffer[i + 1];
  }
  if (marker == ON) buffer[length - 1] = ch;
  else buffer[length - 1] = ' ';

  return EXIT_SUCCESS;
}


int allineastringa(char *stringa, int length, char *side){
  int start = -1, stop = -1, i;
  int border = 1;
  int count = 0;
  char temp[length + 1];

  for (i = 0; i < length; i++) temp[i] = ' '; temp[i] = '\0';

  for (i = 0; i < length; i++){
    if ( isgraph(stringa[i]) && start < 0 ) start = i;
    if ( isgraph(stringa[i]) ) stop = i;
  }
  if (start == -1) return EXIT_FAILURE;
  if (strcmp(side, "DX") == 0) strncpy(&temp[length - (stop - start) - 1], &stringa[start], stop - start + 1);
  else strncpy(temp, &stringa[start], stop - start + 1);
  strncpy(stringa, temp, length);

  return EXIT_SUCCESS;
}
