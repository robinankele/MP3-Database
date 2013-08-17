//------------------------------------------------------------------------------
// ex3.c
//
//  MP3 Database
//
//  Group: 6059 study assistant Jörg Müller
//
// Authors:        Raphael Sommer 0931955
//                 Ralph Ankele   0931953
//                 Robin Ankele   0931951
//                 Muesluem Atas  0931946
//
// created :       11.11.2010 (by Robin Ankele)
// Latest Changes: 11.11.2010 (by Robin Ankele)
//------------------------------------------------------------------------------
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

//------------------------------------------------------------------------------
/// defines
#define HEADER_BINARY "EPDB"
#define BLOCK_SIZE 20
#define WARNING_FILE_EXISTS printf("warning: file %s exists and will be replace\
d. Do you want to proceed? (y/n)", filename);

//------------------------------------------------------------------------------
/// structs
typedef struct
{
  unsigned int id;
  double rating;
  unsigned int content_lenght;
} Header;

typedef struct
{
  char *title;
  char *filename;
  char *artist;
  char *comment;
} Content;

typedef struct MP3Database_
{
  Header header;
  Content content;
  struct MP3Database_ *next;
  struct MP3Database_ *prev;
} MP3Database;

//------------------------------------------------------------------------------
/// forward declarations
MP3Database *CommandPrompt(int *sum_of_entries, MP3Database *database, int *run, 
  char* filename);
MP3Database *checkCommand(char *input, int *sum_of_entries, 
  MP3Database *database, int *run, char* filename);
int checkInt(char *input);
int checkDouble(char *input);
int checkForPoint(char *input);
int checkForAlp(char *input);
int checkFirstChar(char *input);
int biggestID(int *sum_of_entries, MP3Database *database);
void print(int id, char *title, char *artist, int *biggest_value);
int newParameters(int *sum_of_entries, MP3Database *database);
/// error
void error(signed int error_code, char *filename);
char *errorText(char *front_text, char *filename, char *end_text);
/// convert
int charToInt(char *input);
double charToDouble(char *input);
char *intToChar(int input);
char *doubleToChar(double input);
/// read from Console
char *getDatabaseName(char **parameter);
char *getString();
char *readConsoleChar(char *name, int *abort);
/// commands
MP3Database *new(int *sum_of_entries, MP3Database *database);
void view(int *sum_of_entries, MP3Database *database);
void edit(int *sum_of_entries, MP3Database *database);
void delete(int *sum_of_entries, MP3Database *database);
void list(int *sum_of_entries, MP3Database *database);
void export(int *sum_of_entries, MP3Database *database);
void quit(int *run, char *filename, int *sum_of_entries, MP3Database *database);
/// database
MP3Database *writeDatabase(int existing_value, int id, Header new_header, 
  Content new_content, int *sum_of_entries, MP3Database *database, int sort);
MP3Database *sortDatabase(int id, Header new_header, Content new_content, 
  int *sum_of_entries, MP3Database *database);
MP3Database *readDatabase(int id, int *sum_of_entries, MP3Database *database, 
  int *no_entry_found);
MP3Database *searchFirstEntry(int *sum_of_entries, MP3Database *database);
void deleteDatabase(int id, int *sum_of_entries, MP3Database *database);
/// doubly linked lists
MP3Database *newDatabase(Header new_header, Content new_content);
MP3Database *newEntry(MP3Database *database, Header new_header, 
  Content new_content);
void deleteEntry(MP3Database *database);
/// file management
MP3Database *readDatabaseFile(char *filename, int *sum_of_entries);
void writeDatabaseFile(char *filename, int *sum_of_entries,
  MP3Database *database);
void writeHtmlFile(char *filename, MP3Database* database, int sum_of_entries);
/// memory management
void freeDatabase(MP3Database *database, int sum_of_entries);
void freeMemory(char* mem);
char *memoryExtension(char *extendvariable, int *size);
/// read from file
char* readFile(char* filename, int* file_size);
double getDouble(char* content, int start, char* filename);
unsigned int getInteger(char* content, int start, char* filename);
char* getCString(char* content, int* start);
void getHeader(char* content, Header* header, int* str_show, char* filename);
void getContent(char* content, Content* cont, int* str_show);
void checkID(Header* header, int number_data, char* filename);
unsigned int getContentLength(Content* content);

//------------------------------------------------------------------------------
/// The main program.
///
/// @param argc not used
/// @param argv here comes the input text
///
/// @return 0 when ok 
///                -1 when more than one parameter or parameter starts without
///                    -o=
///                -2 when out of memory
///                -3 when cannot read dbfile
///                -4 when dbfile corrupt
///                -5 when cannot write dbfile
int main(int argc, char *argv[])
{
  int run = 1, sum_of_entries = 0;
  char *dbname;
  MP3Database *database;

  if(argc != 2)
    error(-1,"");
  dbname = getDatabaseName(argv);
  database = readDatabaseFile(dbname, &sum_of_entries);  

  while(run)
    database = CommandPrompt(&sum_of_entries, database, &run, dbname);
  
  freeDatabase(database, sum_of_entries);
  return 0;
}

//------------------------------------------------------------------------------
/// This function prints the CommandPrompt
/// @param sum_of_entries the sum of all entrys in the double linked list
/// @param database content and header of one entry
/// @param recent_value the recent entry in database
/// @param run is set to 0 if program ends 1 if programm running
/// @param filename name of the database-file
/// @return returns the database
MP3Database *CommandPrompt(int *sum_of_entries, MP3Database *database, int *run,
  char* filename)
{ 
  char* input;
  MP3Database *new_database;
  printf("esp> ");
  input = getString();
  new_database = checkCommand(input, sum_of_entries, database, run, filename);
  return new_database;
  //if(input)
  //  freeMemory(input);
}

//------------------------------------------------------------------------------
/// checks the command line parameter and returns the name of the db-file
/// @param parameter is the content of the commandline parameter 
/// @return returns the name of db-file
char *getDatabaseName(char **parameter)
{
  char *argv = parameter[1];
  char *db_name; 
  
  if(argv[0] != '-'  || argv[1] != 'o' || argv[2] != '=')
    error(-1,"");
  db_name = &argv[3]; 
  return db_name;
}

//------------------------------------------------------------------------------
/// Reads the input-text from STDIN and stores it in a char-array
/// @return returns the inputtext
char *getString()
{
  int counter = 0;
  int total_block_size = 20;
  char letter;
  char *text;

  text = (char*) malloc(BLOCK_SIZE * sizeof(char));
  if(text)
  {
    while((letter = getchar()) != '\n')
    {
      if(counter >= total_block_size)
        text = memoryExtension(text, &total_block_size);
      text[counter] = letter; 
      counter++;
    }
    text[counter] = 0;
  }
  else
  {
    freeMemory(text);
    error(-2,"");
  }
  return text;
}

//------------------------------------------------------------------------------
/// Checks if the user input is a command. If true the command is executed. If
/// false a error is printed.
/// @param input is the user typed input 
/// @param sum_of_entries  number of entrys in the database
/// @param database one entry of the database
/// @param run is set to 0 if program ends 1 if programm running
/// @param filename name of the database-file
/// @return returns the database
MP3Database *checkCommand(char *input, int *sum_of_entries,
  MP3Database *database, int *run, char* filename)
{
  MP3Database *new_database;
  if(!strcasecmp(input, "new"))
  {
    new_database = new(sum_of_entries, database);
    return new_database;
  }
  if(!strcasecmp(input, "view"))
  {
    view(sum_of_entries, database);
    return database;
  }
  if(!strcasecmp(input, "edit"))
  {
    edit(sum_of_entries, database);
    return database;
  }
  if(!strcasecmp(input, "delete"))
  {
    delete(sum_of_entries, database);
    return database;
  }
  if(!strcasecmp(input, "list"))
  {    
    list(sum_of_entries, database);
    return database;
  }
  if(!strcasecmp(input, "export"))
  {
    export(sum_of_entries, database); 
    return database;
  }
  if(!strcasecmp(input, "quit"))
  {
    quit(run, filename, sum_of_entries, database);
    return database;
  }

  printf("error: unknown command\n"); 
  database = CommandPrompt(sum_of_entries, database, run, filename);
  return database;
}

//------------------------------------------------------------------------------
/// This function check if input is from type integer
/// @param input is the input which is checked if integer
/// @return returns 0 if integer, returns 1 if not integer
int checkInt(char *input)
{
  int count = 0;
  if(!(input[0] >= '0'  && input[0] <= '9') && (input[0] != '+'))
  {
    printf("error: invalid parameter - please enter an integer number\n");
    return 1; 
  }
  for(count = 1; count < strlen(input); count++)
  {
    if(!(input[count] >= '0' && input[count] <= '9'))
    {
      printf("error: invalid parameter - please enter an integer number\n");
      return 1; 
    }
  }
  return 0;
}

//------------------------------------------------------------------------------
/// Check first Character
/// @param input is the input character which is checked
/// @return returns 1 if an error occur and 0 if not
int checkFirstChar(char *input)
{
  if(!(input[0] >= '0'  && input[0] <= '9') && 
    (input[0] != '.') && (input[0] != '-') && (input[0] != '+'))
  { 
    printf("error: invalid parameter - please enter a floating point number\n");
    return 1;
  }
  else
  {
    return 0;
  }
}

//------------------------------------------------------------------------------
/// Check for Alphabet
/// @param input is the input character which is checked
/// @return returns 1 if an error occur and 0 if not
int checkForAlp(char *input)
{
  int count = 1;
  for(; input[count] != '\0'; count++)
  {
    if(!((input[count] >= '0') && (input[count] <= '9')))
    {
      if(input[count] != '.')
      {
        printf("error: invalid parameter - please enter a floating point number\
\n");
        return 1;
      }
    }
  }
  return 0;
}
//------------------------------------------------------------------------------
/// Check for Point
/// @param input is the input character which is checked
/// @return returns 1 if an error occur and 0 if not
int checkForPoint(char *input)
{
  int count = 0;
  int point_count = 0;
  
  for(; input[count] != '\0'; count++)
  {
    if(input[count] == '.')
    {
    point_count++;
    }
  }
  
  if(point_count != 1)
  {
    printf("error: invalid parameter - please enter a floating point number\n");
    return 1;
  }
  return 0;
}

//------------------------------------------------------------------------------
/// Check if double
/// @param input is the input which is checked if double
/// @return returns 0 if integer, returns 1 if not integer
int checkDouble(char *input)
{
  double double_input;

  if(checkFirstChar(input) || checkForAlp(input) || checkForPoint(input))
  {
    return 1;
  }
  double_input = charToDouble(input);
  if(double_input < -5.0 || double_input > 5.0)
  {
    printf("error: invalid parameter - please enter a value between -5.0 and \
5.0\n"); 
    return 1;
  }
  return 0;
}
//------------------------------------------------------------------------------
/// Adds a new Entry in the double linked list
/// @param sum_of_entries  number of entrys in the database
/// @param database one entry of the database
/// @return returns the database
MP3Database *new(int *sum_of_entries, MP3Database *database)
{
  unsigned int id_int;
  int check_id = 1, abort = 0, no_entry_found = 1;
  char *id, *title, *artist, *file, *comment, *rating;
  Header new_header;
  Content new_content;
  
  while(1)
  {
    check_id = 1;
    while(check_id)
    {
      id = readConsoleChar("id", &abort); 
      check_id = checkInt(id); 
    }
    id_int = charToInt(id);
    database = readDatabase(id_int, sum_of_entries, database, &no_entry_found);
    if(no_entry_found)
      printf("error: id already in use\n");
    else
      break;
  }
  title = readConsoleChar("title", &abort);
  artist = readConsoleChar("artist", &abort);
  file = readConsoleChar("file", &abort);

  check_id = 1;
  while(check_id)
  {
    rating = readConsoleChar("rating", &abort); 
    check_id = checkDouble(rating);
  }
  comment = readConsoleChar("comment", &abort);
  
  new_content.title = title;
  new_content.filename = file;
  new_content.artist = artist;
  new_content.comment = comment;
  new_header.id = charToInt(id);
  new_header.rating = charToDouble(rating);
  new_header.content_lenght = sizeof(new_content);
  
  if(*sum_of_entries == 0)
    database = writeDatabase(0, charToInt(id), new_header, new_content,
      sum_of_entries, NULL, 0);
  else
    database = writeDatabase(0, charToInt(id), new_header, new_content,
      sum_of_entries, database, 1);
  
  freeMemory(id);
  freeMemory(rating);
  return database;
}

//------------------------------------------------------------------------------
/// This function views one entry selected by id
/// @param sum_of_entries the sum of all entrys in the double linked list
/// @param database content and header of one entry
/// @return void returns nothing
void view(int *sum_of_entries, MP3Database *database)
{
  char *id;
  unsigned int id_int;
  int check_id = 1, abort = 0, no_entry_found = 1;
  MP3Database *temp_database;
  
  while(check_id)
  {
    id = readConsoleChar("id", &abort);
    check_id = checkInt(id);
  }
  id_int = charToInt(id);
  no_entry_found = 1;
  temp_database = readDatabase(id_int, sum_of_entries, database,
    &no_entry_found);
  if(no_entry_found)
  {
    printf("\n");
    printf("ID: %d\n", temp_database->header.id);
    printf("Title: %s\n",temp_database->content.title);
    printf("Artist: %s\n", temp_database->content.artist);
    printf("File: %s\n", temp_database->content.filename);
    printf("Rating: %f\n", temp_database->header.rating);
    printf("Comment: %s\n", temp_database->content.comment);
  }
  freeMemory(id);
}

//------------------------------------------------------------------------------
/// Edits a entry in database selected by id
/// @param sum_of_entries the sum of all entrys in the double linked list
/// @param database content and header of one entry
/// @return void returns nothing
void edit(int *sum_of_entries, MP3Database *database)
{
  unsigned int id_int;
  int check_id = 1, abort = 1, no_entry_found = 1, id_int_temp;
  char *id, *title, *artist, *file, *comment, *rating;
  double rating_double;
  Header new_header;
  Content new_content;
  MP3Database *recent_values;
  
  id_int_temp = newParameters(sum_of_entries, database);
  if(id_int_temp == -1)
    return;
    
  printf("New parameters:\n");
  while(1)
  {
    check_id = 1;
    while(check_id)
    {
      id = readConsoleChar("id", &abort);
      if(abort == 2)
      {
        no_entry_found = 0;
        break;
      }
      check_id = checkInt(id);
    }
    id_int = charToInt(id);
    
    database = readDatabase(id_int, sum_of_entries, database, &no_entry_found);
    recent_values = readDatabase(id_int_temp, sum_of_entries, database, 0);

    if(!no_entry_found)
    { 
      abort = 1;
      title = readConsoleChar("title", &abort);
      if(abort == 2)
        title = recent_values->content.title;

      abort = 1;
      artist = readConsoleChar("artist", &abort);
      if(abort == 2)
        artist = recent_values->content.artist;
        
      abort = 1;
      file = readConsoleChar("file", &abort);
      if(abort == 2)
        file = recent_values->content.filename;
        
      check_id = 1;
      while(check_id)
      {
        abort = 1;
        rating = readConsoleChar("rating", &abort);
        if(abort == 2)
          break;
        check_id = checkDouble(rating);
      }
      if(abort == 2)
        rating_double = recent_values->header.rating;
      else
        rating_double = charToDouble(rating);
        
      abort = 1;
      comment = readConsoleChar("comment", &abort);
      if(abort == 2)
        comment = recent_values->content.comment;

      new_content.title = title;
      new_content.artist = artist;
      new_content.filename = file;
      new_content.comment = comment;
      new_header.id = id_int;
      new_header.rating = rating_double;
      new_header.content_lenght = sizeof(new_content);
      database = writeDatabase(1, id_int_temp, new_header, new_content, 
        sum_of_entries, database, 1);
      return;
    }
    printf("error: id already in use\n");
  }
}

//------------------------------------------------------------------------------
/// Prints the Database entry with the commited id
/// @param sum_of_entries the sum of all entrys in the double linked list
/// @param database content and header of one entry
/// @return returns the commited id
int newParameters(int *sum_of_entries, MP3Database *database)
{
  MP3Database *temp_database;
  int id_exist = 1, no_entry_found = 1, check_id = 1, id_int, abort = 1;
  char *id;
  
  while(id_exist)
  {
    check_id = 1;
    while(check_id)
    {
      id = readConsoleChar("id", &abort);
      if(abort == 2)
        return -1;
      check_id = checkInt(id);
    }
    id_int = charToInt(id);
    no_entry_found = 1;
    temp_database = readDatabase(id_int, sum_of_entries, database, 
      &no_entry_found);
    if(no_entry_found)
    {
      printf("\n");
      printf("ID: %d\n", temp_database->header.id);
      printf("Title: %s\n",temp_database->content.title);
      printf("Artist: %s\n", temp_database->content.artist);
      printf("File: %s\n", temp_database->content.filename);
      printf("Rating: %f\n", temp_database->header.rating);
      printf("Comment: %s\n", temp_database->content.comment);
      id_exist = 0;
    }
    else 
    {
      printf("error: id does not exist\n"); 
      id_exist = 1;
    }
  }
  return id_int;
}

//------------------------------------------------------------------------------
/// Deletes a entry in database selected by id
/// @param sum_of_entries the sum of all entrys in the double linked list
/// @param database content and header of one entry
/// @return void returns nothing
void delete(int *sum_of_entries, MP3Database *database)
{
  char *id;
  unsigned int id_int;
  int check_id = 1, abort = 1, no_entry_found = 1;
  
  while(1)
  {
    check_id = 1;
    while(check_id)
    {
      id = readConsoleChar("id", &abort);
      if(abort == 2)
      {
        //freeMemory(id);
        return;
      }
      check_id = checkInt(id);
    }
    id_int = charToInt(id);
    
    database = readDatabase(id_int, sum_of_entries, database, 
        &no_entry_found);
    if(!no_entry_found)
      printf("error: id does not exist\n");
    else
      break;
  }
  deleteDatabase(id_int, sum_of_entries, database);
  freeMemory(id);
}

//------------------------------------------------------------------------------
/// Prints a list of all entrys in the database
/// @param sum_of_entries the sum of all entrys in the double linked list
/// @param database content and header of one entry
/// @return void returns nothing
void list(int *sum_of_entries, MP3Database *database)
{
  int count = 0, biggest_value;  
  
  if(*sum_of_entries == 0)
    return;
  
  biggest_value = biggestID(sum_of_entries, database);
  database = searchFirstEntry(sum_of_entries, database);
  
  for(count = 0; count < *sum_of_entries; count++)
  {
    print(database->header.id, database->content.title, 
      database->content.artist, &biggest_value);
    database = database->next;
  }
}

//------------------------------------------------------------------------------
/// Exports all entrys in the database into a html file
/// @param sum_of_entries the sum of all entrys in the double linked list
/// @param database content and header of one entry
/// @return void returns nothing
void export(int *sum_of_entries, MP3Database *database)
{
  int abort = 0;
  char *filename;
  char *check_input;
  
  filename = readConsoleChar("file", &abort);
  FILE *htmlfile = fopen(filename, "r");
  if(htmlfile)
  {
    WARNING_FILE_EXISTS
    while(1)
    {
      check_input = getString();
      if(!strcmp(check_input,"y")) 
      {
        if(*sum_of_entries != 0)
          database = searchFirstEntry(sum_of_entries, database);
        writeHtmlFile(filename, database, *sum_of_entries);
        freeMemory(check_input);
        freeMemory(filename);
        return;
      }
      if(!strcmp(check_input,"n"))
      {
        freeMemory(check_input);
        freeMemory(filename);
        return;
      }
      WARNING_FILE_EXISTS
    }
  }
  else
  {
    if(*sum_of_entries != 0)
      database = searchFirstEntry(sum_of_entries, database);
    writeHtmlFile(filename, database, *sum_of_entries);
    freeMemory(filename);
  }
}

//------------------------------------------------------------------------------
/// Ends the Programm
/// @param sum_of_entries the sum of all entrys in the double linked list
/// @param database content and header of one entry
/// @param run is set to 0 if program ends 1 if programm running
/// @param filename name of the database-file
/// @return void returns nothing
void quit(int *run, char *filename, int *sum_of_entries, MP3Database *database)
{
  writeDatabaseFile(filename, sum_of_entries, database);
  *run = 0;
}

//------------------------------------------------------------------------------
/// Reads a string and returns it 
/// @param name is printed out
/// @param abort if enter is pressed it is set to 2
/// @return returns the input string
char *readConsoleChar(char *name, int *abort)
{
  char *input;
  int count = 0;
  do
  {
    if(count && *abort)
    {
      *abort = 2;
      return "";
    }
    printf("  %s? ",name);
    input = getString();
    count = 1;
  }
  while(!strcmp(input,""));
  return input;
} 

//------------------------------------------------------------------------------
/// This function searches the biggest id and returns it
/// @param sum_of_entries the sum of all entrys in the double linked list
/// @param database content and header of one entry
/// @return the biggest ID in the database
int biggestID(int *sum_of_entries, MP3Database *database)
{
  int biggest_id = 0;
  int count;
  for(count = 0; count < *sum_of_entries; count++)
  {
    if(database->header.id > biggest_id)
    {
      biggest_id = database->header.id;
    }
    database = database->next;
  }
  return biggest_id;
}

//------------------------------------------------------------------------------
///  The print function makes the spaces for the output in list function
/// @param id is the id of the database entry
/// @param title is the title of the database entry
/// @param artist is the artist who performed the database entry
/// @param biggest_value is the biggest id of the database entrys
/// @return returns void
void print(int id, char *title, char *artist, int *biggest_value)
{
  int spaces, spaces_count;
  if(id == 0)
    spaces = (int)log10(*biggest_value);
  else
    spaces = ((int)log10(*biggest_value)) - ((int)log10(id));

  for(spaces_count = 0; spaces_count < spaces; spaces_count++)
    printf(" ");

  printf("%d - %s (%s)\n",id ,title, artist);
}

//------------------------------------------------------------------------------
/// Converts a string into a integer
/// @param input is the input which should converted
/// @return returns the converted input
int charToInt(char *input)
{
  return atoi(input);
}

//------------------------------------------------------------------------------
/// Converts a string into a double
/// @param input is the input which should converted
/// @return returns the converted input
double charToDouble(char *input)
{
  return atof(input);
}

//------------------------------------------------------------------------------
/// Converts a Integer into a String
/// @param input is the input which should converted
/// @return returns the converted input
char *intToChar(int input)
{
  char *buffer = (char*) malloc(sizeof(input));
  sprintf(buffer, "%d", input);
  return buffer;
}

//------------------------------------------------------------------------------
/// Converts a Double into a String
/// @param input is the input which should converted
/// @return returns the converted input
char *doubleToChar(double input)
{
  char *buffer = (char*) malloc(sizeof(input));
  sprintf(buffer, "%.7f", input);
  return buffer;
}

//------------------------------------------------------------------------------
/// throws an Error
/// @param error_code coded number which should be returned
/// @param filename name of dbfile
/// @return returns void
void error(signed int error_code, char *filename)
{
  char *printing_text;
  switch(error_code)
  {
    case -1: printf("usage: ex3 -o=dbfile\n"); break;
    case -2: printf("out of memory\n"); break;
    case -3: 
      printing_text = errorText("error: cannot read dbfile ", filename, "\n");
      printf(printing_text); 
      freeMemory(printing_text); break;
    case -4:
      printing_text = errorText("error: dbfile ", filename, " corrupt\n");
      printf(printing_text); 
      freeMemory(printing_text); break;
    case -5: 
      printing_text = errorText("error: cannot write dbfile ", filename, "\n");
      printf(printing_text); 
      freeMemory(printing_text); break;
  }
  exit(error_code);
}

//------------------------------------------------------------------------------
///  The errorText function creates an errorText for the error function
/// @param front_text is the text which is at the front of the error message
/// @param filename is the filename of the file
/// @param end_text is the text which is at the end of the error message
/// @return returns the errortext
char *errorText(char *front_text, char *filename, char *end_text)
{
  int size;
  char *error_text;
  size = (strlen(front_text) + 1) + (strlen(filename) + 1) + 
    (strlen(filename) + 1);
  error_text = (char*) malloc(size * sizeof(char));
  strcpy(error_text, front_text);
  strcat(error_text, filename);
  strcat(error_text, end_text);
  return error_text;
}

//------------------------------------------------------------------------------
/// The writeDatabase function writes a Entry to the Database
/// @param existing_value is 1 if the entry is edited or 0 when a new entry is
///        created
/// @param id is the identifyer of the Entry
/// @param new_header is the Header of the Entry
/// @param new_content is the Content of the Entry
/// @param sum_of_entries is the sum of all entry in the database
/// @param database is the database which should added to the db-file
/// @param sort is 1 if the database should be sorted 0 if not
/// @return returns a database entry
MP3Database *writeDatabase(int existing_value, int id, Header new_header,
  Content new_content, int *sum_of_entries, MP3Database *database, int sort)
{
  MP3Database *new_database;
  if(existing_value == 1)
  {
    new_database = readDatabase(id, sum_of_entries, database, 0);
    deleteDatabase(id, sum_of_entries, new_database);
    if(sort)
      new_database = sortDatabase(id, new_header, new_content, sum_of_entries, 
        new_database);
    return new_database;
  }
  else
  {
    if(sort)
      new_database = sortDatabase(id, new_header, new_content, sum_of_entries, 
        database);
    else
    {
      if(!database)
        new_database = newDatabase(new_header, new_content);
      else
        new_database = newEntry(database, new_header, new_content);
      *sum_of_entries += 1; 
    }
  }
  return new_database;
}

//------------------------------------------------------------------------------
/// The sortDatabase function sorts the database by ids
/// @param id is the identifyer of the Entry
/// @param new_header is the Header of the Entry
/// @param new_content is the Content of the Entry
/// @param sum_of_entries is the sum of all entry in the database
/// @param database is the database which should added to the db-file
/// @return returns the sorted database
MP3Database *sortDatabase(int id, Header new_header, Content new_content, 
  int *sum_of_entries, MP3Database *database)
{
  int entries_count = 0;
  int upper_border = 0, lower_border = 0;

  for(entries_count = 0; entries_count < *sum_of_entries; entries_count++)
  {
    if(database->header.id > id)
    {
      if(lower_border)
      {
        database = database->prev;
        database = writeDatabase(0, 0, new_header, new_content, sum_of_entries,
          database, 0);
        break;
      }
      database = database->prev;
      upper_border = 1;
    }  
    if(database->header.id < id)
    {
      if(upper_border)
      {
        database = writeDatabase(0, 0, new_header, new_content, sum_of_entries,
          database, 0);
        break;
      }
      database = database->next;
      lower_border = 1;
    }
  }
  return database;
}

//------------------------------------------------------------------------------
/// This function check if an entry with the select id is found and returns it
/// @param id is the identifyer of the Entry
/// @param sum_of_entries is the sum of all entry in the database
/// @param database is the database which should added to the db-file
/// @param no_entry_found is 0 if no entry is found 1 if found
/// @return returns the found database
MP3Database *readDatabase(int id, int *sum_of_entries, MP3Database *database, 
  int *no_entry_found)
{
  int entries_count = 0;
  int check_id = 0;
  for(entries_count = 0; entries_count < *sum_of_entries; entries_count++)
  {
    if(database->header.id == id)
    {
      check_id = 1;
      break;
    }
    database = database->next;
  }
  if(check_id != 1)
    *no_entry_found = 0;
  return database;
}

//------------------------------------------------------------------------------
/// This Function searches the first entry in the database
/// @param sum_of_entries is the sum of all entry in the database
/// @param database is the database which should added to the db-file
/// @return returns the Entry of the first id
MP3Database *searchFirstEntry(int *sum_of_entries, MP3Database *database)
{
  int smallest_id = database->header.id;
  int count;
  MP3Database *first_entry;
  for(count = 0; count < *sum_of_entries; count++)
  {
    if(database->header.id < smallest_id)
    {
      smallest_id = database->header.id;
    }
    database = database->next;
  }
  first_entry = readDatabase(smallest_id, sum_of_entries, database, 0);
  return first_entry;
}

//------------------------------------------------------------------------------
/// This function delets a entry in the database
/// @param id is the identifyer of the Entry
/// @param sum_of_entries is the sum of all entry in the database
/// @param database is the database which should added to the db-file
/// @return returns void
void deleteDatabase(int id, int *sum_of_entries, MP3Database *database)
{
  database = readDatabase(id, sum_of_entries, database, 0);
  deleteEntry(database);
  *sum_of_entries -= 1;
  freeDatabase(database, 1);
}

//------------------------------------------------------------------------------
/// This function creates a new database
/// @param new_header is the Header of the Entry
/// @param new_content is the Content of the Entry
/// @return returns the new database
MP3Database *newDatabase(Header new_header, Content new_content)
{
  MP3Database *database = (MP3Database*) malloc(sizeof(MP3Database));
  if(database)
  {
    database->header = new_header;
    database->content = new_content;
    database->prev = database;
    database->next = database;
  }
  else
    error(-2,"");
  return database;
}

//------------------------------------------------------------------------------
/// This function adds entries to the database
/// @param database is the database which should added to the db-file
/// @param new_header is the Header of the Entry
/// @param new_content is the Content of the Entry
/// @return returns the database with the new entry
MP3Database *newEntry(MP3Database *database, Header new_header, 
  Content new_content)
{
  MP3Database *new_database = (MP3Database*) malloc(sizeof(MP3Database));
  if(new_database)
  {
    new_database->header = new_header;
    new_database->content = new_content;
    new_database->prev = database;
    new_database->next = database->next;
    database->next = new_database;
    new_database->next->prev = new_database;
  }
  else 
    error(-2,"");
  return new_database;
}

//------------------------------------------------------------------------------
/// This function removes entries from database
/// @param database is the database which should added to the db-file
/// @return returns void 
void deleteEntry(MP3Database *database)
{
  database->next->prev = database->prev;
  database->prev->next = database->next;
}

//------------------------------------------------------------------------------
/// This function writes the double linked list to the db-file
/// @param filename is the filename of the file
/// @param sum_of_entries is the sum of all entry in the database
/// @param database is the database which should added to the db-file
/// @return returns void
void writeDatabaseFile(char *filename, int *sum_of_entries,
  MP3Database *database)
{
  int count_header, count_content;
  
  FILE *binaryfile = fopen(filename, "w");
  binaryfile = fopen(filename, "ab");
  
  
  if(binaryfile)
  {    
    fwrite(HEADER_BINARY, 4, sizeof(char), binaryfile);
    fwrite(sum_of_entries, 1, sizeof(int), binaryfile);
    
    if(*sum_of_entries > 0)
    {
      database = searchFirstEntry(sum_of_entries, database);
      for(count_header = 0; count_header < *sum_of_entries; count_header++)
      {
        fwrite(&database->header.id, 1, sizeof(int), binaryfile);
        fwrite(&database->header.rating, 1, sizeof(double), binaryfile);
        fwrite(&database->header.content_lenght, 1, sizeof(int), binaryfile);
        database = database->next;
      }
      database = searchFirstEntry(sum_of_entries, database);
      for(count_content = 0; count_content < *sum_of_entries; count_content++)
      {
        fwrite(database->content.title, (strlen(database->content.title) + 1), 
          sizeof(char), binaryfile);
        fwrite(database->content.filename, 
          (strlen(database->content.filename) + 1), sizeof(char), binaryfile);
        fwrite(database->content.artist, (strlen(database->content.artist) + 1)
          , sizeof(char), binaryfile);
        fwrite(database->content.comment, 
          (strlen(database->content.comment) + 1),
          sizeof(char), binaryfile);
        database = database->next;
      }
    }
    if(fclose(binaryfile))
      error(-5,"");
  }
  else
    error(-3,"");
}

//------------------------------------------------------------------------------
/// This function writes the double linked list to the html-file
/// @param filename is the filename of the file
/// @param sum_of_entries is the sum of all entry in the database
/// @param database is the database which should added to the db-file
/// @return returns void
void writeHtmlFile(char *filename, MP3Database* database, int sum_of_entries)
{
  FILE *htmlfile;
  int counter;
  
  htmlfile = fopen(filename, "w");
  if(htmlfile == NULL)
    printf("error: cannot write to file %s\n", filename);
  else
  {
    fprintf(htmlfile, "<?xml version=\"1.0\" ?>\n \
    <!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"\n \
    \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n \
    <html xmlns=\"http://www.w3.org/1999/xhtml\">\n \
    <head>\n<title>Musikdatenbank</title>\n</head>\n<body>\n");
    
    counter = 0;
    if(sum_of_entries != 0)
    {
      for(; counter < sum_of_entries; counter++)
      {
        fprintf (htmlfile,
        "<div class=\"data\">\n <p class=\"title\"><strong>Title: </strong>");
        fprintf(htmlfile,"%s",database->content.title);
        fprintf(htmlfile, "</p>\n <p class=\"desc\"><strong>ID: </strong>");
        fprintf(htmlfile, "%i",database->header.id);
        fprintf(htmlfile,"</p>\n <p class=\"desc\"><strong>Artist: </strong>");
        fprintf(htmlfile,"%s",database->content.artist);
        fprintf(htmlfile,"</p>\n <p class=\"desc\"><strong>File: </strong>");
        fprintf(htmlfile,"%s",database->content.filename);
        fprintf(htmlfile,"</p>\n <p class=\"own\"><strong>Rating: </strong>");
        fprintf (htmlfile, "%f",database->header.rating);
        fprintf(htmlfile,"</p>\n <p class=\"own\"><strong>Comment: </strong>");
        fprintf(htmlfile, "%s", database->content.comment);
        fprintf(htmlfile,"</p>\n </div>\n");
        database = database->next;
      }
    }
    fprintf(htmlfile,"</body>\n</html>");
    fclose(htmlfile);
  }
}

//------------------------------------------------------------------------------
/// This  function frees the deleted entry
/// @param sum_of_entries is the sum of all entry in the database
/// @param database is the database which should added to the db-file
/// @return returns void
void freeDatabase(MP3Database *database, int sum_of_entries)
{
  int counter = 0;
  for(; counter < sum_of_entries; counter++)
  {
    freeMemory(database->content.title);
    freeMemory(database->content.filename);
    freeMemory(database->content.artist);
    freeMemory(database->content.comment);
    if(sum_of_entries > 1)
    {
      database = database->next;
      free(database->prev);
      database->prev = NULL;
    }
    else
    {
      free(database);
      database = NULL;
    }
  }
}

//------------------------------------------------------------------------------
/// freeMemory Function
/// @param mem this variable is to freed
/// @return returns void
void freeMemory(char* mem)
{
  free(mem);
  mem = NULL;
}

//------------------------------------------------------------------------------
/// This function extends the memory of the commited variable
/// @param extendvariable the variable in heap which should be realloceted
/// @param size defines the size on which extendvariable should be realloceted
/// @return returns the memory-size changed char-array
char *memoryExtension(char *extendvariable, int *size) 
{
  char *extendedvar;
  *size *= 2;
  extendedvar = (char*) realloc(extendvariable, *size);
  if(extendedvar) 
    extendedvar[*size-1] = '\0';
  else
  {
    freeMemory(extendedvar);
    error(-2,"");
  }
  return extendedvar;
}

//------------------------------------------------------------------------------
/// This function reads from the db-file
/// @param filename is the name of the db-file
/// @param filesize is the size of the db file
/// @return returns the content of the db-file
char* readFile(char* filename, int* file_size)
{
  char* content;
  
  FILE* database_file;
  database_file = fopen(filename,"r");
  if(database_file == NULL)
    error(-3, filename);  
  
  fseek (database_file, 0, SEEK_END);
  *file_size = ftell(database_file);
  if(*file_size < 8)
    error(-4, filename);
  rewind(database_file);
  
  content = (char*) malloc (sizeof(char)*(*file_size)+1);
  fread(content,1,*file_size,database_file);
  fclose(database_file);
  content[*file_size] = '\n';
  
  if((content[0] == 'E') && (content[1] == 'P') && 
     (content[2] == 'D') && (content[3] == 'B') )
  return content;
  else
    error(-4, filename);
  return NULL;
}

//------------------------------------------------------------------------------
/// This function reads a double variable of the db-file
/// @param content is the content of the db-file
/// @param start is the position where it should start to read
/// @param filename is the name of the db-file
/// @return returns the double value which should read out
double getDouble(char* content, int start, char* filename)
{
  double rating = 0;
  char to_position[start];
  
  FILE* database_file;
  database_file = fopen(filename,"r");
    
  if (database_file!=NULL)
  { 
    fread(&to_position, sizeof(char), start, database_file);
    fread(&rating, sizeof(double), 1, database_file);
    fclose(database_file);
  }
  return rating;
}

//------------------------------------------------------------------------------
/// This function reads a integer variable of the db-file
/// @param content is the content of the db-file
/// @param start is the position where it should start to read
/// @param filename is the name of the db-file
/// @return returns the unsigned integer value which should read out
unsigned int getInteger(char* content, int start, char* filename)
{
  int value = 0;
  char to_position[start];
  
  FILE* database_file;
  database_file = fopen(filename,"r");
    
  if (database_file!=NULL)
  { 
    fread(&to_position,sizeof(char),start, database_file);
    fread(&value,sizeof(int),1, database_file);
    fclose(database_file);  
  }

  return value;
}

//------------------------------------------------------------------------------
/// This function reads a String variable of the db-file
/// @param content is the content of the db-file
/// @param start is the position where it should start to read
/// @param filename is the name of the db-file
/// @return returns the string value which should read out
char* getCString(char* content, int* start)
{
  char* search_string = 0;
  char* finish_string = 0;
  int leng = 0;

  search_string  = &content[*start];
  leng = strlen(search_string);

  finish_string = (char*)malloc(sizeof(char)*(leng+1));
  strncpy(finish_string, search_string,leng);
  finish_string[leng] = 0;
  *start += (leng+1);
  
  return finish_string;
}

//------------------------------------------------------------------------------
/// This function puts all variables of the Header in the Header
/// @param content is the content of the db-file
/// @param header is the Header of the linked list
/// @param str_show is the position where it should start to read
/// @param filename is the name of the db-file
/// @return returns void
void getHeader(char* content, Header* header, int* str_show, char* filename)
{ 
  header->id = getInteger(content, *str_show, filename);
  header->rating = getDouble(content, *str_show+4, filename);
  header->content_lenght = getInteger(content, *str_show+12, filename);
  *str_show += 16;
}

//------------------------------------------------------------------------------
/// This function puts all variables of the Content in the Content
/// @param content is the content of the db-file
/// @param cont is the Content of the linked list
/// @param str_show is the position where it should start to read
/// @return returns void
void getContent(char* content, Content* cont, int* str_show)
{
  cont->title = getCString(content, str_show);
  cont->filename = getCString(content, str_show);
  cont->artist = getCString(content, str_show);
  cont->comment = getCString(content, str_show);
}

//------------------------------------------------------------------------------
/// This function reads the content of the db-file
/// @param filename is the name of the db-file
/// @param sum_of_entries is the sum of all entry in the database
/// @return returns the database
MP3Database *readDatabaseFile(char *filename, int *sum_of_entries)
{
  int file_size = 0;
  MP3Database* database = NULL;
  char* content = readFile(filename, &file_size);
  int number_data = 0;
  *sum_of_entries = number_data;
  int str_show = 4;

  number_data = getInteger(content, str_show, filename);
  str_show += 4;
  Header header[number_data];
  int counter = 0;
  for(; counter < number_data; counter++)
    getHeader(content, &header[counter], &str_show, filename);
 
  checkID(header, number_data, filename); //check 
  
  Content cont[number_data];
  for(counter = 0; counter < number_data; counter++)
  {
    if(str_show <= (file_size-16)) //check file is to short 
      getContent(content, &cont[counter], &str_show);
    if(header[counter].content_lenght != (getContentLength(&cont[counter])))
      error(-4, filename);
  }
  
  if(file_size != str_show) //check file is to long
    error(-4, filename); 
 
  for(counter = 0; counter < number_data; counter++)
    database = writeDatabase(0, 0, header[counter], cont[counter], 
                             sum_of_entries, database, 0);
  free(content);
  return database;
}

//------------------------------------------------------------------------------
/// This function checks the id of the Header
/// @param header is the Header of the linked list
/// @param number_data is the number of data in the database
/// @param filename is the name of the db-file
/// @return returns void
void checkID(Header* header, int number_data, char* filename)
{
  int counter = 0;
  for(;counter+1 < number_data; counter++)
  {
    if(header[counter].id >= header[counter+1].id)
      error(-4, filename);
  }
}

//------------------------------------------------------------------------------
/// This function returns the lenght of the content
/// @param content is the content of the db-file
/// @return returns the database
unsigned int getContentLength(Content* content)
{
 unsigned int content_length = 0;
 content_length +=   strlen(content->title)+1;
 content_length +=   strlen(content->filename)+1;
 content_length +=   strlen(content->artist)+1;
 content_length +=   strlen(content->comment)+1;
 
 return content_length;
}
