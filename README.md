# CSViewer
Console app for viewing CSV files.

### Platforms
Windows, Linux

### Features  
* supports main features of CSV format (https://en.wikipedia.org/wiki/Comma-separated_values)
  * different encodings and Unicode (using libiconv library)
  * quoting
  * filtering bad lines
  * columns header
* simple shell with commands
* viewing various selections of data by rows and/or columns
* pretty rendering of tabular data
  * fitting view to columns content and console size
  * wrap mode

### Command line options
**csviewer \<file\> [options]**

  -H - Print help  
  -E\<str\> - Set file encoding (cp1251, ansi, utf8, utf16), utf8 by default  
  -D\<char\> - Set delimiter (comma ',' by default). Can be specified as char code \x, for example \56.  
  -Q\<char\> - Set quoting char to enclose strings. " by default.  
  -C - Read column names from the 1st line of file.  
  -B\<raise, warn, skip\> - How to process bad lines in file.    
  -S\<num\> - Skip first N lines of file.  

### Shell commands
General command syntax:  
**command [keyword1] [arg1_1, arg1_2, ...] | [keyword2] [arg2_1, arg2_2, ...]**

Arguments types:
* string (Query, "User query", "666")
* integer (1, 777)
* strings range (A:Z, "Dummy 1":"Dummy 5")
* integers range (1:5)
* key-value pair (A=1, 1=A, "column":"data")

#### Available commands:  

**h[ead]  [N]** - print first N rows  
**t[ail]  [N]** - print last N rows  
**p[rint]** - print all rows chunk by chunk  
**p[rint]  [row] R1, <M1:N1>, <M2:N2>, R2, ...** - print selected rows  
**p[rint]  col С1, <A1:B1>, <A2:B2>, C2, ...** - print selected columns  
**p[rint]  [row] R1, <M1:N1>, <M2:N2>, R2, ... | col С1, <A1:B1>, <A2:B2>, C2, ...** - print selected rows and columns  
**v[iew]  [param1=val1, param2=val2 ...]** - get or set view options  
**c[ols]  [\<X1\>=\<NAME1\>, ..., \<XN\>=\<NAMEN\>]** - get or set column names  
**s[hape]** - get shape of all data  
**help** - print help on commands  
**q[uit]** - quit app  
