#lang racket
(require "main.rkt")

(connect)
#|
(define levels-i (get-levels))
(define walls-i (get-walls))
(define slabs-i (get-slabs))
(define columns-i (get-columns))
(define objects-i (get-objects))
(define roofs-i (get-roofs))

(delete-levels)

(recreate-levels levels-i)
(recreate-walls walls-i)
(recreate-slabs slabs-i)
(recreate-columns columns-i)
(recreate-objects objects-i)
(recreate-roofs roofs-i)
|#

;(recreate-levels)

(recreate-walls-material-2)

#;(recreate-slabs)
#;(recreate-columns)
#;(recreate-objects)
#;(recreate-roofs)

(disconnect)