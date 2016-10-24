@ECHO OFF
IF EXIST "D:\GRAPHISOFT\ArchiCAD 18\Add-Ons\RosettaArchiCAD.apx"(del "D:\GRAPHISOFT\ArchiCAD 18\Add-Ons\RosettaArchiCAD.apx") ELSE()
copy "D:\GRAPHISOFT\API Development Kit 18.3006\Examples\Geometry_Test - Copy\x64\RosettaArchiCAD.apx" "D:\GRAPHISOFT\ArchiCAD 18\Add-Ons"