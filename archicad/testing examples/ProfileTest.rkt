#lang racket
(require "main.rkt")

;;Superellipse
;Creates a superellipse-shaped surface, as a possible variation to the shape of the slab.
(define (superellipse p a b n t)
  (+xy p
       (* a (expt (expt (cos t) 2) (/ 1 n)) (sgn (cos t)))
       (* b (expt (expt (sin t) 2) (/ 1 n)) (sgn (sin t)))))

(define (points-superellipse p a b n n-points)
  ;TODO not using closed line for slab!!!
  (map-division (lambda (t) (superellipse p a b n t)) -pi pi n-points #f))

(define (limited-points-superellipse p a b n start finish n-points)
  (map-division (lambda (t) (superellipse p a b n t)) start finish n-points #f))

#|
(send (test-wall (list (x 0)(x 10)) 2 "e1" (u0) 20 15 1.75 0 pi 50)
        (test-wall (list (x 0)(x 10)) 2 "e2" (u0) 10 8 1.75 0 pi 50))
(send (test-wall (list (x 0)(x 2)) 8 "e1" (u0) 20 17 1.75 0 pi 50))
|#
(define (test-wall guide thickness profile-name p a b n start finish n-points)
  (let* ((curve (limited-points-superellipse p a b n start finish n-points))
         (back-curve (reverse (map (lambda (p) (+xy p
                                                     (* (/ (cx p) (sqrt (+ (expt (cx p) 2)
                                                                           (expt (cy p) 2))))
                                                        (- thickness))
                                                     (* (/ (cy p) (sqrt (+ (expt (cx p) 2)
                                                                           (expt (cy p) 2))))
                                                        (- thickness)))) curve)))
         (profile-points (append curve back-curve)))
    (profile profile-name profile-points)
    (wall guide #:profile-name profile-name)
    (column (x 0) #:profile-name profile-name)))