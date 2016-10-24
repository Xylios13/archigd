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
  ;TODO not using closed line for slab!!!
  (map-division (lambda (t) (superellipse p a b n t)) -pi pi n-points #f))

(define (limited-points-superellipse p a b n start finish n-points)
  (map-division (lambda (t) (superellipse p a b n t)) start finish n-points #t))

(define (square-center-points pt width height)
  (list (+xy pt (- (/ width 2))(- (/ height 2)))
        (+xy pt (+ (/ width 2))(- (/ height 2)))
        (+xy pt (+ (/ width 2))(+ (/ height 2)))
        (+xy pt (- (/ width 2))(+ (/ height 2)))))

#|
(send (delete-levels) (floors (u0) 40 15 12 #:n 4 #:ni 1.2))
(send (delete-levels) (floors (u0) 40 15 12 #:n 2 #:ni 1.8))
(send (delete-levels) (floors (u0) 140 15 12 #:n 4 #:ni 2))

|#
(define (floors origin
                building-length
                width
                number-floors
                #:n [n 3]
                #:ni [ni 2.0]
                #:wall-thickness [wall-thickness 0.3]
                #:n-wall [n-wall 50]
                #:balcony-minimum-length [balcony-minimum-length 3]
                #:number-holes [number-holes 3])
  (let* ([a (+ width (* (- number-floors 0) (default-level-to-level-height)))]
         [b (+ 5 (* (- number-floors 0) (default-level-to-level-height)))]
         [ai (* (- number-floors 0) (default-level-to-level-height))]
         [bi (* (- number-floors 0) (default-level-to-level-height))]
         (pts (for/list ([i (+ number-floors 1)])
                        (superellipse-y (u0) a b n (* i (default-level-to-level-height)))))
         (interior-pts (for/list ([i (+ number-floors 1)])
                                 (superellipse-y (u0) ai bi ni (* i (default-level-to-level-height)))))
         (front-wall-curve (limited-points-superellipse (u0) a b n 0 pi 50))
         (front-wall-inner-curve (reverse (limited-points-superellipse (u0) ai bi ni 0 pi 50)))
         (list-of-walls (division 0 building-length (floor (/ building-length 20))))
         
         (levels (for/list ([i number-floors])
                           (create-level #:height (* i (default-level-to-level-height))))))
    ;(println pts)
    ;(println (superellipse-t origin a b n (cy (last pts))))
    ;(slab pts)
    ;(slab interior-pts)
    (profile "front-wall-profile" (append front-wall-curve front-wall-inner-curve))
    (for ([y (cdr list-of-walls)]
          [i (- (length list-of-walls) 1)])
         (wall (list (+y origin y) (+y origin (+ y 0.3)))
               #:profile-name "front-wall-profile"))
    (for ([lvl levels]
          [i (length levels)]
          [pte pts]
          [pti interior-pts])
         (parameterize ((current-level lvl))
           (let* ((p1 (+x origin (cx pte)))
                  (p2 (+x origin (cx pti)))
                  (p3 (+x origin (- (cx pte))))
                  (p4 (+x origin (- (cx pti))))
                  (ptee (list-ref pts (+ i 1)))
                  (ptii (list-ref interior-pts (+ i 1)))
                  (inner-wall-curve (map (lambda (pt)
                                           (+xy pt (- (cx pti)) (- (cy pti))))
                                         (limited-points-superellipse (u0)
                                                                      ai
                                                                      bi
                                                                      ni
                                                                      (superellipse-t origin ai bi ni (cy pti))
                                                                      (superellipse-t origin ai bi ni (cy ptii))
                                                                      50)))
                  (inner-wall-inner-curve (reverse (map (lambda (pt)
                                                          (+x pt (- wall-thickness)))
                                                        inner-wall-curve)))
                  (balcony-wall-curve (map (lambda (pt)
                                             (+xy pt (- (cx pte)) (- (cy pte))))
                                           (limited-points-superellipse (u0)
                                                                        a
                                                                        b
                                                                        n
                                                                        (superellipse-t origin a b n (cy pte))
                                                                        (superellipse-t origin a b n (+ (cy pte)
                                                                                                        (/ (abs (- (cy pte)(cy ptee))) 2)))
                                                                        50)))
                  (balcony-wall-inner-curve (reverse (map (lambda (pt)
                                                            (+x pt 0.1))
                                                          balcony-wall-curve)))
                  (facade-curve (map (lambda (pt)
                                       (+xy pt (- (cx pti)) (- (cy pti))))
                                     (limited-points-superellipse (u0)
                                                                  a
                                                                  b
                                                                  n
                                                                  (superellipse-t origin a b n (cy pte))
                                                                  (superellipse-t origin a b n (cy ptee))
                                                                  50)))
                  (facade-inner-curve (reverse inner-wall-curve)))
             (slab (list p1
                         (+y p1 building-length)
                         (+y p2 building-length)
                         p2))
             (slab (list p3
                         (+y p3 building-length)
                         (+y p4 building-length)
                         p4))
             ;facade-wall
             (profile (string-append "facade-wall-level " (~a i))
                      (append facade-curve facade-inner-curve))
             (map (lambda (n)
                    (add-hole-profile (string-append "facade-wall-level " (~a i))
                                      (square-center-points (xy n (/ (default-level-to-level-height) 2)) (/ (default-level-to-level-height) 2) (/ (default-level-to-level-height) 2) )))
                  (division (default-level-to-level-height)
                            (- (abs (- (cx p1)(cx p2))) (* (default-level-to-level-height) 1.5))
                            number-holes))
             (wall (list p2 (+y p2 wall-thickness))  #:profile-name (string-append "facade-wall-level " (~a i)))
             (wall (list (+y p4 wall-thickness) p4)  #:profile-name (string-append "facade-wall-level " (~a i)))
             
             ;inner-walls
             (profile (string-append "inner-wall-profile-level " (~a i))
                      (append inner-wall-curve inner-wall-inner-curve)
                      #:material "Glass")
             (wall (list p2 (+y p2 building-length))  #:profile-name (string-append "inner-wall-profile-level " (~a i)))
             (wall (list (+y p4 building-length) p4)  #:profile-name (string-append "inner-wall-profile-level " (~a i)))
             
             ;balcony-walls
             (profile (string-append "balcony-wall-profile-level " (~a i))
                      (append balcony-wall-curve balcony-wall-inner-curve)
                      #:material "Glass")
             (wall (list p1 (+y p1 building-length))  #:profile-name (string-append "balcony-wall-profile-level " (~a i)))
             (wall (list (+y p3 building-length) p3)  #:profile-name (string-append "balcony-wall-profile-level " (~a i)))
             
             ;structural-wall
             (if (> (abs (- (cx pte)(cx ptee))) balcony-minimum-length)
                 (begin
                   (wall (list (+x origin (cx ptee)) (+xy origin (cx ptee) building-length)))
                   (wall (list (+x origin (- (cx ptee))) (+xy origin (- (cx ptee)) building-length))))
                 (begin
                   (wall (list (+x origin (+ (cx pte) (- balcony-minimum-length))) (+xy origin (+ (cx pte) (- balcony-minimum-length)) building-length)))
                   (wall (list (+x origin (- (+ (cx pte) (- balcony-minimum-length)))) (+xy origin (- (+ (cx pte) (- balcony-minimum-length))) building-length)))))
             
             ;when last floor
             ;do a roof from walls
             (when (= i (- number-floors 1))
               (let* ((t (superellipse-t origin a b n (+ (cy pte) 3)))
                      (pr1 (+x origin (cx ptee)))
                      (pr3 (+x origin (- (cx ptee))))
                      (super-e (limited-points-superellipse (u0) a b n t pi/2 n-wall))
                      (pt-aux (car super-e))
                      (roof-wall-curve (for/list ([pt super-e])
                                                 (+xy pt
                                                      (- (cx pt-aux))
                                                      (- (cy pt-aux)))))
                      (roof-wall-inner-curve (map (lambda (pt)
                                                    (+x pt wall-thickness))
                                                  (reverse roof-wall-curve)))
                      (facade-curve-aux (map (lambda (pt)
                                               (+xy pt
                                                    (- (cx (last super-e)))
                                                    (- (cy pt-aux))))
                                             super-e))
                      (facade-curve (cons (x (cx (last facade-curve-aux))) facade-curve-aux)))
                 (profile "roof-walls-profile"
                          (append roof-wall-curve roof-wall-inner-curve))
                 (wall (list pr1 (+y pr1 building-length))
                       #:bottom-level (upper-level)
                       #:profile-name "roof-walls-profile")
                 (wall (list (+y pr3 building-length) pr3)
                       #:bottom-level (upper-level)
                       #:profile-name "roof-walls-profile")
                 
                 (profile "facade-wall-last-level" facade-curve)
                 
                 (map (lambda (n)
                    (add-hole-profile "facade-wall-last-level"
                                      (square-center-points (xy n (/ (default-level-to-level-height) 2)) (/ (default-level-to-level-height) 2) (/ (default-level-to-level-height) 2) )))
                  (division (default-level-to-level-height)
                            (- (abs (- (cx p1)(cx p2))) (* (default-level-to-level-height) 1.5))
                            number-holes))
                 (wall (list origin (+y origin wall-thickness))
                       #:bottom-level (upper-level)
                       #:profile-name "facade-wall-last-level")
                 (wall (list (+y origin wall-thickness) origin)
                       #:bottom-level (upper-level)
                       #:profile-name "facade-wall-last-level")
                 
                 )))))))
