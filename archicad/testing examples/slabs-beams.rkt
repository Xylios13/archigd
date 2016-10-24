#lang racket
(require "main.rkt")

(define (rad->deg r)
  (/ (* r 180) pi)) 

(define (mid-point p0 p1)
  (xy (/ (+ (cx p0)(cx p1)) 2) (/ (+ (cy p0)(cy p1)) 2)))
(define (frac-point p0 p1 k)
  (xy (+ (cx p0) (* k (- (cx p1)(cx p0))))
      (+ (cy p0) (* k (- (cy p1)(cy p0))))))

(define (div-3 p0 p1)
  (list p0 (frac-point p0 p1 1/3)(frac-point p0 p1 2/3)))

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
  ;TODO not using closed line for slab!!!
  (map-division (lambda (t) (superellipse p a b n t)) -pi pi n-points #f))

(define (limited-points-superellipse p a b n start finish n-points)
  (map-division (lambda (t) (superellipse p a b n t)) start finish n-points #t))

(define (square-center-points pt width height)
  (list (+xy pt (- (/ width 2))(- (/ height 2)))
        (+xy pt (+ (/ width 2))(- (/ height 2)))
        (+xy pt (+ (/ width 2))(+ (/ height 2)))
        (+xy pt (- (/ width 2))(+ (/ height 2)))))

(define (tower-slabs points number-floors da)
  (let ((levels (for/list ([i number-floors])
                          (level (* i (default-level-to-level-height))))))
    (for ([lvl levels]
          [i (length levels)])
         (slab (map (lambda (p)
                      (pol (sqrt (+ (* (cx p)(cx p))(* (cy p)(cy p))))
                           (+ (pol-phi p)(* da i))))
                    points)
               #:bottom-level lvl))))

(define (tower-beams points number-floors da)
  (for ([i (- number-floors 1)])
       (define alt-points (append (flatten (for/list ([p0 points]
                                              [p1 (cdr points)])
                                             (div-3 p0 p1)))
                                  (div-3 (last points)(first points))))
       (for ([p alt-points])
            (let ((p1 (pol (sqrt (+ (* (cx p)(cx p))(* (cy p)(cy p))))
                               (+ (pol-phi p)(* da i))))
                  (p2 (+z (pol (sqrt (+ (* (cx p)(cx p))(* (cy p)(cy p))))
                               (+ (pol-phi p)(* da (+ i 1))))
                          (default-level-to-level-height))))
              #;(column p1 #:slant-angle  (abs (- pi/2 (sph-psi (p-p p1 p2)))) #:slant-direction (+ (sph-phi (p-p p1 p2)) pi/2) #:bottom-level (create-level #:height (* i (default-level-to-level-height))))
              (column-two-points p1 p2 #:bottom-level (level (* i (default-level-to-level-height))) #:circle-based? #t)
              #;(beam p1 p2 #:bottom-level (level (* i (default-level-to-level-height))))))))

(define (tower points number-floors da)
  (tower-slabs points number-floors da)
  (tower-beams points number-floors da))

#|
(send (delete-levels) (tower (polygon-points (u0) 8 5) 5 pi/3))
(send (delete-levels) (tower (polygon-points (u0) 8 5) 5 0))
(send (delete-levels) (tower (polygon-points (u0) 4 5) 5 pi/4))

(send (delete-levels) (tower (polygon-points (x 5) 8 5) 5 pi/3))


(send (delete-levels) (tower (polygon-points (u0) 6 10) 15 (/ pi 14)))

|#