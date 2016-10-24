#lang racket
(require "main.rkt")

(define (rad->deg r)
  (/ (* r 180) pi)) 

;;Superellipse
;Creates a superellipse-shaped surface, as a possible variation to the shape of the slab.
(define (superellipse p a b n t)
  (+xy p
       (* a (expt (expt (cos t) 2) (/ 1 n)) (sgn (cos t)))
       (* b (expt (expt (sin t) 2) (/ 1 n)) (sgn (sin t)))))

(define (superellipse-t p a b n y)
  (if (and (<= y b) (>= y (- b)))
      (asin (expt (expt (/ y b) (/ 1 2)) n))
      (begin (disconnect)
             (print "y: ")
             (println y)
             (print "b: ")
             (println b)
             (error "y must be between -b and b"))))

;Given y, calculates t and then given the point with same y
(define (superellipse-y p a b n y)
  (let* ((t (superellipse-t p a b n y)))
    #;((aux (expt (expt (/ y b) (/ 1 2)) n))
       (bounded-value (cond [(> aux 1) (- aux (truncate aux))]
                            [(< aux -1) (+ aux (truncate aux))]
                            [else aux]))
       (t (asin bounded-value)))
    (+xy p
         (* a (expt (expt (cos t) 2) (/ 1 n)) (sgn (cos t)))
         y)))

(define (points-superellipse p a b n n-points )
  (map-division (lambda (t) (superellipse p a b n t)) -pi pi n-points))

(define (limited-points-superellipse p a b n start finish n-points)
  (map-division (lambda (t) (superellipse p a b n t)) start finish n-points #t))

(define (test)
  (send (sweep (list (xy 0 0)(xy 5 0)(xy 5 5)(xy 0 5))
               (for/list ([p (points-superellipse (u0) 1 1 1 4)])
                      (xyz (* 10 (cx p)) 0 (* 10 (cy p)))))))