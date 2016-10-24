#lang racket
(require "main.rkt")

(define (+pol-lst lst k psi)
  (for/list ([p lst])
            (+pol p k psi)))

(define (tower center floors sides start-radius rotation-step #:shift [shift (/ 2pi floors)] #:translate [translate 3])
  (let ((radius-step (/ start-radius floors)))
    (for ([i floors])
         (parameterize ((current-level (create-level #:height (* i (default-level-to-level-height)))))
           (define p center #;(+pol center translate (* i shift)))
           (define slab-points (+pol-lst (polygon-points (u0) sides (- start-radius (* i radius-step))(* i rotation-step)) translate (* i shift)))
           (define upper-slab-points (+pol-lst (polygon-points (u0) sides (- start-radius (* (+ i 1) radius-step))(* (+ i 1) rotation-step)) translate (* (+ i 1) shift)))
           (slab slab-points #:type-of-material "Basic")
           (if (= (- start-radius (* (+ i 1) radius-step)) 0)
               (for ([pt1 (append slab-points (list (car slab-points)))]
                     [pt2 (append (cdr slab-points) (list (car slab-points)))])
                    (surface-polygon (list (+z pt1 (* i (default-level-to-level-height)))
                                           (+z pt2 (* i (default-level-to-level-height)))
                                           (+z (+pol p translate (* (+ i 1) shift)) (* (+ i 1) (default-level-to-level-height)))) "Glass")
                    (column-two-points pt1
                                       (+z (+pol p translate (* (+ i 1) shift)) (default-level-to-level-height))
                                       #:width 0.05
                                       #:depth 0.05))
               (for ([pt1 (append slab-points (list (car slab-points)))]
                     [pt2 (append (cdr slab-points) (list (car slab-points)))]
                     [pt3 (append upper-slab-points (list (car upper-slab-points)))]
                     [pt4 (append (cdr upper-slab-points) (list (car upper-slab-points)))])
                    (surface-polygon (list (+z pt1 (* i (default-level-to-level-height)))
                                           (+z pt2 (* i (default-level-to-level-height)))
                                           (+z pt4 (* (+ i 1) (default-level-to-level-height)))) "Glass")
                    (surface-polygon (list (+z pt1 (* i (default-level-to-level-height)))
                                           (+z pt4 (* (+ i 1) (default-level-to-level-height)))
                                           (+z pt3 (* (+ i 1) (default-level-to-level-height)))) "Glass")
                    (column-two-points pt1
                                       (+z pt3 (default-level-to-level-height))
                                       #:width 0.05
                                       #:depth 0.05)
                ))))))

#|

(send (delete-all-elements)(tower (u0) 50 10 10 (/ pi 20)))
|#