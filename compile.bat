@ECHO OFF
protoc -I=. --racket_out=./archigd/archicad --cpp_out=./Src messages.proto