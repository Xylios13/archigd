#lang racket

(define (move-addon-file)
  (let ((internal-path-addon "../x64/Geometry_Test.apx")
        (internal-path-directory "D:/GRAPHISOFT/ArchiCAD 18/Add-Ons")
        (internal-path-directory-addon "D:/GRAPHISOFT/ArchiCAD 18/Add-Ons/Geometry_Test.apx"))
    (when (and (directory-exists? internal-path-directory)
               (file-exists? internal-path-addon))
      (rename-file-or-directory internal-path-addon internal-path-directory-addon #t))))
(move-addon-file)