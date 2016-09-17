#ifndef SCRIPT_FUNCTIONS_H
#define SCRIPT_FUNCTIONS_H

#define MYSQL_CONNECTION_COUNT (4)

/* MySQL-documented */
void Scr_MySQL_Real_Connect_f();
void Scr_MySQL_Close_f();
void Scr_MySQL_Affected_Rows_f();
void Scr_MySQL_Query_f();
void Scr_MySQL_Num_Rows_f();
void Scr_MySQL_Num_Fields_f();
void Scr_MySQL_Fetch_Row_f();

/* MySQL-custom */
void Scr_MySQL_Fetch_Rows_f();

#endif /* SCRIPT_FUNCTIONS_H */
