# PoSh Microshell
PoSh, short for **Po**la**Sh**ell -pronounced /pɒʃ/  similar to the BBC pronunciation of “posh"- is a Micro Shell implemented using Lex and Yacc. <br>
This Project was done as part of Alex University's Operating Systems Course: CSE 366 

Supported Grammer:<br>
cmd [arg]* [ | cmd [arg]* ]* [ [> filename] 
 [< filename]
[>> filename] ]* [&]<br>
> Example: ls -al | grep command | grep command.o > out

Todo:
- Ctrl+c override
- cd [dir] cmd
- support other non-Alphanumeric args
- log process deletion
- Wild Carding

