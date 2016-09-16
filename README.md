# mysql
CoD4X MySQL support for GSC.

# Contributors
- [Sharpienero](https://github.com/Sharpienero/)
- [Michael Hillcox](https://github.com/MichaelHillcox/)
- [T-Maxxx](https://github.com/T-Max/)

# Script Docs 
### MySQL Related Functions

#### `mysql_real_connect()`

Connects to a database

Usage example `db = mysql_real_connect(host, user, pass, db, port)`

###### Use 127.0.0.1 on unix, and the default port for mysql is 3306.

#### `mysql_close()`

Close a MySQL Connection

Usage example `mysql_close()`

#### `mysql_errno()`

Returns the MySQL Error number.
Usage example: `error = mysql_errno()`

#### `mysql_error()`

Returns the MySQL Error number.
Usage example: `error = mysql_error()`

#### `mysql_query()`

Send a query to a database and returns the result for use in the following functions:

Usage example:
```cs
query = mysql_query(<string query>)
```

#### `mysql_rowcount()`
Returns the amount of rows

Usage example: `count = mysql_rowcount()`

#### `mysql_affected_rows()`
Returns the amount of affected rows

Usage example: `count = mysql_affected_rows()`

#### `mysql_num_field()`

Returns number of fields

Usage example = `count = mysql_num_field()`


#### `mysql_fetch_rows()`

Fetches and handles all rows from the `mysql_query()`

Usage example = `data = mysql_fetch_rows()`

Returns: This function will return two different data types depending on the field count. If the field count is one, `if( mysql_num_field(mysql) == 1 )`, then the function will return a key based array `data['field_name']`. If the `mysql_num_field()` is larger than one then it will return a 2D array. The first array being a numeric array and the second being a key based array `data[0]['field_name']`.
