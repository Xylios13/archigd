#lang racket
(require "main.rkt")

;CHANGABLE VALUES_______________________________________________________

(define n-floors 160)
(define floor-height 4)
(define slab-thickness 0.5)

(define tower-max-radius 40)
(define tower-min-radius 16)
(define center-radius 10)
(define turbine-radius 6)

(define rotations-of-skin 3)
(define bar-radius 0.25)
(define middle-bar-radius 0.05)
(define small-bar-radius 0.03)

;;4 SKIN BASIS____________________________________________________________

;definig values

(define top% 0.18)
(define tower-height (* floor-height n-floors (- 1 top%)))
(define tower-top-height (* floor-height n-floors))

(define lineA-radius (+ tower-max-radius turbine-radius))
(define lineB-radius tower-max-radius)
(define lineC-radius tower-min-radius)
(define lineD-radius (/ (+ tower-max-radius tower-min-radius) 2))
(define lineE-radius (/ (+ tower-max-radius tower-min-radius) 1.5))

(define lineA-fi0 0)
(define lineB-fi0 0)
(define lineC-fi0 pi)
(define lineD-fi0 (* 3 pi/2))
(define lineE-fi0 (/ -pi 4))

;definig functions

(define (net-points p r0 r fi0 rotations-of-skin n)
  (let ((dfi (/ (* rotations-of-skin 2pi) n))
        (dz (/ tower-height n))
        (dr (/ r0 (* n top%))))
    (cond ((< (cz p) tower-height)        
           (cons (+pol p r fi0)
                 (net-points (+z p dz) r0 r (- fi0 dfi) rotations-of-skin n)))
          ((>= r 0)
           (cons (+pol p r fi0)
                 (net-points (+z p dz) r0 (- r dr) (- fi0 dfi) rotations-of-skin n)))
          (else (list)))))

(define (bar spl r)
  (for ([p0 spl]
        [p1 (cdr spl)])
       (column-two-points p0 p1 #:depth r #:width r #:circle-based? #t)))

(define (skin-line pts rb)
  ;TODO SPLINE (bar (spline pts) rb)
  (bar pts rb))

(define (tower-main-skin p)
  (let ((rb bar-radius)
        (n n-floors))
  (skin-line (net-points p lineA-radius lineA-radius lineA-fi0 rotations-of-skin n) rb)
  (skin-line (net-points p lineB-radius lineB-radius lineB-fi0 rotations-of-skin n) rb)
  (skin-line (net-points p lineC-radius lineC-radius lineC-fi0 rotations-of-skin n) rb)
  (skin-line (net-points p lineD-radius lineD-radius lineD-fi0 rotations-of-skin n) rb)
  (skin-line (net-points p lineE-radius lineE-radius lineE-fi0 rotations-of-skin n) rb)))

;;MIDDLE SKIN NORMAL ROTATION__________________________________________________

;definig values

(define n-lines-BC 4)
(define n-lines-CD 3)
(define n-lines-EB 2)

(define n-points-BC (/ n-floors 3))
(define n-points-CD (/ n-floors 1.5))
(define n-points-DE (/ n-floors 3))
(define n-points-EB (/ n-floors 1.5))

;matrix of middle-skin points

(define (skin-matrix p
                     lineX-radius lineY-radius
                     lineX-fi0 lineY-fi0
                     n-lines-XY n-lines-XY0 n-points-XY)
  (let ((dfi (/ (- lineY-fi0 lineX-fi0) (+ 1 n-lines-XY0)))
        (dr (/ (- lineY-radius lineX-radius) (+ 1 n-lines-XY0))))
    (if (<= n-lines-XY 0)
        (list)
        (cons
         (net-points p
                     (+ lineX-radius (* n-lines-XY dr))
                     (+ lineX-radius (* n-lines-XY dr))
                     (+ lineX-fi0 (* n-lines-XY dfi))
                     rotations-of-skin
                     n-points-XY)
         (skin-matrix p
                     lineX-radius lineY-radius
                     lineX-fi0 lineY-fi0
                     (- n-lines-XY 1) n-lines-XY0 n-points-XY)))))

(define (skin-lines mtrx rb)
  (if (null? mtrx)
      #t
      (begin (skin-line (car mtrx) rb)
             (skin-lines (cdr mtrx) rb))))

(define (skin-matrix-BC p)
  (skin-matrix p
               lineB-radius lineC-radius
               lineB-fi0 lineC-fi0
               n-lines-BC n-lines-BC n-points-BC))
(define (skin-matrix-CD p)
  (skin-matrix p
               lineC-radius lineD-radius
               lineC-fi0 lineD-fi0
               n-lines-CD n-lines-CD n-points-CD))
(define (skin-matrix-EB p)
  (skin-matrix p
               lineE-radius lineB-radius
               lineE-fi0 lineB-fi0
               n-lines-EB n-lines-EB n-points-EB))

(define (horizontal-middle-skin p)
  (skin-lines (skin-matrix-BC p) middle-bar-radius)
  (skin-lines (skin-matrix-CD p) middle-bar-radius)
  (skin-lines (skin-matrix-EB p) middle-bar-radius))

;;MIDDLE SKIN OPOSITE ROTATION__________________________________________________

;sub-list and invert-list funtions

(define (sublist l begining)
  (if (= begining 0)
      l
      (sublist (cdr l) (- begining 1))))        

(define (pyramidal-pt-subtraction mtrx levels)
  (if (= levels 0)
      (cdr mtrx)
      (cons (sublist (car mtrx) levels)
            (pyramidal-pt-subtraction (cdr mtrx) (- levels 1)))))

(define (transpose-matrix mtrx)
  (if (null? (car mtrx))
      (list)
      (cons (map car mtrx)
            (transpose-matrix (map cdr mtrx)))))

(define (invertlist l)
  (if (null? l)
      (list)
      (list (flatten (cdr l))
            (car l))))
;matrix of total-skin points

(define (vertical-skin p
                       skin-mtrx
                       lineX-radius lineY-radius
                       lineX-fi0 lineY-fi0
                       n-points-XY)
  (define (list-cons skin-mtrx)
    (if (null? skin-mtrx)
        (list (net-points p lineX-radius lineX-radius lineX-fi0 rotations-of-skin n-points-XY))
        (cons (car skin-mtrx)
              (list-cons (cdr skin-mtrx)))))
  (cons (net-points p lineY-radius lineY-radius lineY-fi0 rotations-of-skin n-points-XY)
        (list-cons skin-mtrx)))

(define (tot-skin-mtrx-BC p)
  (transpose-matrix
   (pyramidal-pt-subtraction
    (vertical-skin p
                   (skin-matrix-BC p)
                   lineB-radius lineC-radius
                   lineB-fi0 lineC-fi0
                   n-points-BC)
    (/ n-lines-BC 2))))

(define (tot-skin-mtrx-CD p)
  (transpose-matrix
   (pyramidal-pt-subtraction
    (vertical-skin p
                   (skin-matrix-CD p)
                   lineC-radius lineD-radius
                   lineC-fi0 lineD-fi0
                   n-points-CD)
    (+ n-lines-CD 1))))

(define (tot-skin-mtrx-EB p)
  (transpose-matrix
   (pyramidal-pt-subtraction
    (vertical-skin p
                   (skin-matrix-EB p)
                   lineE-radius lineB-radius
                   lineE-fi0 lineB-fi0
                   n-points-EB)
    (+ n-lines-EB 1))))
    
(define (tot-skin-mtrx-DE1 p)
(transpose-matrix
 (list (cdr (net-points p lineE-radius lineE-radius lineE-fi0 rotations-of-skin n-points-DE))
       (net-points p lineD-radius lineD-radius lineD-fi0 rotations-of-skin n-points-DE))))
(define (tot-skin-mtrx-DE2 p)
(transpose-matrix
 (list (cdr (net-points p lineD-radius lineD-radius lineD-fi0 rotations-of-skin n-points-DE))
       (cdr (net-points p lineE-radius lineE-radius lineE-fi0 rotations-of-skin n-points-DE)))))

(define (vertical-middle-skin p)
  (skin-lines (tot-skin-mtrx-BC p) middle-bar-radius)
  (skin-lines (tot-skin-mtrx-CD p) small-bar-radius)
  (skin-lines (tot-skin-mtrx-DE1 p) middle-bar-radius)
  (skin-lines (tot-skin-mtrx-DE2 p) middle-bar-radius)
  (skin-lines (tot-skin-mtrx-EB p) small-bar-radius))


;;FLOORS__________________________________________________________________________

(define (floor-middle-points A B)
  (if (or (null? A) (null? B))
      (list)
      (cons (xyz (/ (+ (cx (car A)) (cx (car B))) 2)
                 (/ (+ (cy (car A)) (cy (car B))) 2)
                 (cz (car A)))
            (floor-middle-points (cdr A) (cdr B)))))

(define (middle-points-list p)
  (floor-middle-points (net-points p lineB-radius lineB-radius lineB-fi0 rotations-of-skin n-floors)
                       (net-points p lineC-radius lineC-radius lineC-fi0 rotations-of-skin n-floors)))

(define (slabs pts-1 pts-2)
      (if (>= (cz (car pts-1)) cylinder-height)
          #t
          (parameterize ((current-level (create-level #:height (cz (car pts-1)))))
            (slab (list (car pts-1)(car pts-2))
                  #:parcs (list -pi)
                  #:thickness slab-thickness)
            ;(wall (list (car pts-1)(car pts-2)) #:material "Glass" #:top-level (upper-level #:height 3.2800))
            ;(wall (list (car pts-1)(car pts-2)) #:arcs (list -pi) #:material "Glass" #:top-level (upper-level #:height 3.2800))
            (slabs (cdr pts-1) (cdr pts-2)))))

(define (floors p)
  (slabs (net-points p lineB-radius lineB-radius lineB-fi0 rotations-of-skin n-floors)
         (net-points p lineC-radius lineC-radius lineC-fi0 rotations-of-skin n-floors)))

       
;;CENTER CYLINDER AND TOP_________________________________________________________

(define cylinder-height (+ tower-height (* tower-top-height (/ top% 3))))

(define (center-cylinder p)
  (column p #:depth center-radius #:width center-radius #:circle-based? #t #:top-level (create-level #:height cylinder-height)))

(define (top-cylinder p)
  (let ((p1 (+z p cylinder-height))
        (p2 (+xz p 0 (+ cylinder-height (* tower-top-height (/ top% 3.5)))))
        #;(p2 (+xz p (- center-radius (/ center-radius 2))
                   (+ cylinder-height (* tower-top-height (/ top% 3.5))))))
    (pyramid-by-center p1 p2 (/ center-radius 2) #:level (create-level #:height cylinder-height))
    #;(column-two-points p1 p2 #:depth center-radius #:width center-radius #:bottom-level (create-level #:height cylinder-height))
    #;(rotate (union
             (loft (list (circle p1 center-radius)
                         (circle p2 (/ center-radius 4))))
             (sphere p2 (/ center-radius 4)))
            (+ (/ (+ lineC-fi0 lineD-fi0) 2) (* rotations-of-skin 2pi)))))

;;GLASS__________________________________________________________________________

(define (glass-line A B)
  (if (> (cz (car A)) cylinder-height)
      null
      (begin (surface-polygon (list (car A) (car B) (cadr A)) "Glass")
             (surface-polygon (list (car B) (cadr B) (cadr A)) "Glass")
             (glass-line (cdr A) (cdr B)))))

(define (glass-wall glass-matrix)
  (if (null? (cdr glass-matrix))
      null
      (begin (glass-line (car glass-matrix) (cadr glass-matrix))
             (glass-wall (cdr glass-matrix)))))

(define (glass-matrix-CD p)
  (vertical-skin p
                 (skin-matrix p
                              lineC-radius lineD-radius
                              lineC-fi0 lineD-fi0
                              (+ (* 2 n-lines-CD) 1) (+ (* 2 n-lines-CD) 1)
                              (+ (* 2 n-floors) 1))
                 lineC-radius lineD-radius
                 lineC-fi0 lineD-fi0
                 (+ (* 2 n-floors) 1)))

(define (glass-matrix-EB p)
  (vertical-skin p
                 (skin-matrix p
                              lineE-radius lineB-radius
                              lineE-fi0 lineB-fi0
                              (+ (* 2 n-lines-EB) 1) (+ (* 2 n-lines-EB) 1)
                              (+ (* 2 n-floors) 1))  
                 lineE-radius lineB-radius
                 lineE-fi0 lineB-fi0
                 (+ (* 2 n-floors) 1)))

(define (glass p)
  (glass-wall (glass-matrix-CD p))
  (glass-wall (glass-matrix-EB p)))

;;TURBINES__________________________________________________________________________

#|

(define (fan-blade p1 p2)
  (surface-polygon p1 p2
                   (+z (/c (+c p1 p2) 2)
                       (* 2 small-bar-radius))))

(define (fan p)
  (let ((r (+ (/ turbine-radius 2) small-bar-radius))
        (fi (/ pi 3)))
    (union
     (sweep (circle p r) (circle (u0) small-bar-radius))
     (sphere p (* 2 small-bar-radius))
     (fan-blade p (+pol p r 0))
     (fan-blade p (+pol p r fi))
     (fan-blade p (+pol p r (* 2 fi)))
     (fan-blade p (+pol p r (* 3 fi)))
     (fan-blade p (+pol p r (* 4 fi)))
     (fan-blade p (+pol p r (* 5 fi))))))

(define (place-turbines p0 fi)
  (define r (+ tower-max-radius (/ turbine-radius 2)))
  (define n (* rotations-of-skin 16))
  (define dfi (/ (* rotations-of-skin 2pi) n))
  (define dz (/ tower-height n))
  (let ((p (+pol p0 r fi)))
    (cond ((>= fi -pi)
           (place-turbines (+z p0 dz) (- fi dfi)))
          ((<= (cz p0) tower-height)
           (union
            (rotate (rotate (fan p) pi/2 p (+x p 1))
                    fi p)
            (place-turbines (+z p0 dz) (- fi dfi))))
          (else (empty-shape)))))
    
(define (turbines p)
  (place-turbines p 0))

|#
;;CONSTRUCT_______________________________________________________________
#|
(define layer-bars (create-layer "bars" (rgb 255 255 255)))
(define layer-glass (create-layer "glass" (rgb 152 222 255)))
(define layer-slabs (create-layer "slabs" (rgb 33 246 108)))
|#

(define p (x 0))
#|
There is a bug in the levels creation,
the order in which they are created is important.
If the last level is created before the others, i.e.,
do the center column first it will result in a bug with the next level height
|#

(connect)
(delete-levels)
(floors p)
(center-cylinder p)
(top-cylinder p)
(tower-main-skin p)
(horizontal-middle-skin p)
(glass p)
(disconnect)

#|
(with-current-layer layer-bars
                    (center-cylinder p)
                    (tower-main-skin p)
                    ;(horizontal-middle-skin p)
                    ;(vertical-middle-skin p)
                    (turbines p)
                    )

(with-current-layer layer-glass
                    (glass p)
                    (top-cylinder p)
                    )

(with-current-layer layer-slabs
                    (floors p)
                    )
|#





