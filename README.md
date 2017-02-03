# mysql
CoD4X MySQL support for GSC. Continious connection up to 4 databases supported.

# Contributors
- [Sharpienero](https://github.com/Sharpienero/)
- [Michael Hillcox](https://github.com/MichaelHillcox/)
- [T-Maxxx](https://github.com/T-Max/)

# Script Docs 
### MySQL Related Functions

#### `mysql_real_connect(<str host>, <str user>, <str passwd>, <str db>, [int port=3306])`

Connects to a database, returns handle to database connection. If you want to connect to multiple databases, you can call this function up to 4 times. Do not forget to close connection with [mysql_close()](https://github.com/callofduty4x/mysql/blob/master/README.md#mysql_closehandle). TODO: link here!

If "localhost" passed as `host`, for *NIX OS will be changed to "127.0.0.1".

Usage example 1: `handle = mysql_real_connect("localhost", "cod4server", "12345678", "players");`

Usage example 2: `handle = mysql_real_connect("255.125.255.125", "usr1337", "87654321", "stats", 1337);`


#### `mysql_close(<handle>)`

Close a MySQL connection. Handle must be valid.

Usage example: `mysql_close(handle);`

#### `mysql_query(<handle>, <str query>)`

Send a query to a database and saves the result for use in following functions. Must be called after [mysql_real_connect()](https://github.com/callofduty4x/mysql/blob/master/README.md#mysql_real_connectstr-host-str-user-str-passwd-str-db-int-port3306).

Usage example: `mysql_query(handle, "SELECT * from users");`

#### `mysql_num_rows(<handle>)`

Returns the amount of rows received after latest query. Must be called after [mysql_real_connect()](https://github.com/callofduty4x/mysql/blob/master/README.md#mysql_real_connectstr-host-str-user-str-passwd-str-db-int-port3306) and [mysql_query()](https://github.com/callofduty4x/mysql/blob/master/README.md#mysql_queryhandle-str-query).

Usage example: `rowsCount = mysql_num_rows(handle);`

#### `mysql_affected_rows(<handle>)`

Returns the amount of affected rows after latest query. Must be called after [mysql_real_connect()](https://github.com/callofduty4x/mysql/blob/master/README.md#mysql_real_connectstr-host-str-user-str-passwd-str-db-int-port3306) and [mysql_query()](https://github.com/callofduty4x/mysql/blob/master/README.md#mysql_queryhandle-str-query).

Usage example: `rowsCount = mysql_affected_rows(handle);`

#### `mysql_num_fields(<handle>)`

Returns number of fields in latest query result. Must be called after [mysql_real_connect()](https://github.com/callofduty4x/mysql/blob/master/README.md#mysql_real_connectstr-host-str-user-str-passwd-str-db-int-port3306) and [mysql_query()](https://github.com/callofduty4x/mysql/blob/master/README.md#mysql_queryhandle-str-query).

Usage example: `fieldsCount = mysql_num_field(handle);`

#### `mysql_fetch_row(<handle>)`

Returns next row from latest query result as array. Array's keys are equal to column names and can be found with `getArrayKeys(<array>)`. An empty array returned if there's no more rows left. Must be called after [mysql_real_connect()](https://github.com/callofduty4x/mysql/blob/master/README.md#mysql_real_connectstr-host-str-user-str-passwd-str-db-int-port3306) and [mysql_query()](https://github.com/callofduty4x/mysql/blob/master/README.md#mysql_queryhandle-str-query).

Usage example: 
```
arr = mysql_fetch_row(handle);
keys = getArrayKeys(arr);
for(j = 0; j < keys.size; j++)
{
  iprintln("Key=" + keys[j] + ", value=" + arr[keys[j]]);
}
```

### Non-MySQL Related Functions
#### `mysql_fetch_rows(<handle>)`

Returns rest rows from latest query result as 2-dimensional array. Array's keys are equal to column names and can be found with `getArrayKeys(<array>)`. An empty array returned if there's no more rows left. Must be called after [mysql_real_connect()](https://github.com/callofduty4x/mysql/blob/master/README.md#mysql_real_connectstr-host-str-user-str-passwd-str-db-int-port3306) and [mysql_query()](https://github.com/callofduty4x/mysql/blob/master/README.md#mysql_queryhandle-str-query).

Usage example: 
```
query = mysql_fetch_rows(handle);
for(i = 0; i < query.size; i++)
{
  keys = getArrayKeys(query[i]);
  for(j = 0; j < keys.size; j++)
  {
    iprintln("Row #" + i + ", Key=" + keys[j] + ", value=" + query[i][keys[j]] + "\n");
  }
}
```
