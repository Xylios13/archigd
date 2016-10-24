#lang racket
;(require "main.rkt")
(require rosetta/archicad)
(delete-levels)
(define height 15)
(define wall-height 3)
#;(define levels-list (for/list ([z (range 0 height wall-height)])
                                (level z)))
(define levels-list (map level (range 0 height wall-height)))

(for ([lvl levels-list])
  (parameterize ((current-level lvl))
    (slab (list (xy 0 0) (xy 5 0) (xy 5 5) (xy 0 5)))
    (walls (list (xy 0 0) (xy 5 0) (xy 5 5) (xy 0 5)))))

(roof (list (xy 0 0) (xy 5 0) (xy 5 5) (xy 0 5))
      (upper-level (last levels-list) wall-height))
(disconnect)
