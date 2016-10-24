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

BEST:
(send (delete-levels) (floors (u0) 120 6 11 #:n 4 #:ni 3 #:balcony-minimum-length 1))

|#
(define (floors origin
                building-length
                side-wall-width
                number-floors
                #:building-limit-width [b-l-w 36]
                #:n [n 3]
                #:ni [ni 2.0]
                #:wall-thickness [wall-thickness 0.3]
                #:n-wall [n-wall 50]
                #:balcony-minimum-length [balcony-minimum-length 3]
                #:number-holes [number-holes 3]
                #:panel-size [panel-size (default-level-to-level-height)])
  (parameterize ((default-wall-alignment "Outside")
                 (default-slab-reference "CoreTop"))
    (let* ([a b-l-w #;(+ side-wall-width (* (- number-floors 0) (default-level-to-level-height)))]
           [b (+ 5 (* (- number-floors 0) (default-level-to-level-height)))]
           [ai (- b-l-w (* side-wall-width 2))#;(* (- number-floors 0) (default-level-to-level-height))]
           [bi (* (- number-floors 0) (default-level-to-level-height))]
           (pts (for/list ([i (+ number-floors 1)])
                          (superellipse-y (u0) a b n (* i (default-level-to-level-height)))))
           (interior-pts (for/list ([i (+ number-floors 1)])
                                   (superellipse-y (u0) ai bi ni (* i (default-level-to-level-height)))))
           (front-wall-curve (limited-points-superellipse (u0) a b n 0 pi 50))
           (front-wall-inner-curve (reverse (limited-points-superellipse (u0) ai bi ni 0 pi 50)))
           (list-of-walls (division 0 building-length (floor (/ building-length 20))))
           
           (levels (for/list ([i number-floors])
                             (level (* i (default-level-to-level-height))))))
      ;(println pts)
      ;(println (superellipse-t origin a b n (cy (last pts))))
      ;(slab pts)
      ;(slab interior-pts)
      (profile "front-wall-profile" (append front-wall-curve front-wall-inner-curve))
      (for ([y (cdr list-of-walls)]
            [i (- (length list-of-walls) 2)])
           (wall (+y origin y) (+y origin (+ y 0.3))
                  #:profile-name "front-wall-profile"))
      (define panels-pts (range (cx (+x origin (- (cx (first interior-pts)))))
                                (cx (+x origin (+ (cx (first interior-pts)) panel-size)))
                                panel-size))
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
                                         (+xy pt (- (cx pte)) (- (cy pte))))
                                       (limited-points-superellipse (u0)
                                                                    a
                                                                    b
                                                                    n
                                                                    (superellipse-t origin a b n (cy pte))
                                                                    (superellipse-t origin a b n (cy ptee))
                                                                    50)))
                    (facade-inner-curve (reverse inner-wall-curve))
                    (balcony-actual-length (if (> (abs (- (cx pte)(cx ptee))) balcony-minimum-length)
                                               (abs (- (cx pte)(cx ptee)))
                                               balcony-minimum-length)))
               
               (slab (list p1
                           (+y p1 building-length)
                           (+y p2 building-length)
                           p2))
               (slab (list p3
                           (+y p3 building-length)
                           (+y p4 building-length)
                           p4))
               ;facade-wall
               
               
               (profile (string-append "outter-facade-wall-level " (~a i))
                        (append facade-curve (list (xy (- balcony-actual-length) (cy (last facade-curve)))
                                                   (x (- balcony-actual-length)))))
               
               (wall p1 (+y p1 wall-thickness)  #:profile-name (string-append "outter-facade-wall-level " (~a i)))
               (wall (+y p3 wall-thickness) p3  #:profile-name (string-append "outter-facade-wall-level " (~a i)))
               
               (wall (+y p1 building-length) (+y p1 (+ wall-thickness building-length))  #:profile-name (string-append "outter-facade-wall-level " (~a i)))
               (wall (+y p3 (+ wall-thickness building-length)) (+y p3 building-length)  #:profile-name (string-append "outter-facade-wall-level " (~a i)))
               
               
               #|
               (profile (string-append "inner-facade-wall-level " (~a i))
                        (append facade-inner-curve (list (xy (cx (last facade-inner-curve))
                                                             (cy (car facade-inner-curve))))))
               
               (wall p2 (+y p2 wall-thickness)  #:profile-name (string-append "inner-facade-wall-level " (~a i)))
               (wall (+y p4 wall-thickness) p4  #:profile-name (string-append "inner-facade-wall-level " (~a i)))
               |#
               
               (define right-wall
                 (wall p2 (+x p1 (- balcony-actual-length)))
                 #;(wall p2 p1))
               (define left-wall
                 (wall (+x p3 balcony-actual-length) p4)
                 #;(wall p3 p4))
               
               (define back-right-wall
                 (wall (+y p2 building-length) (+xy p1 (- balcony-actual-length) building-length)))
               (define back-left-wall
                 (wall (+xy p3 balcony-actual-length building-length) (+y p4 building-length)))
               
               (map (lambda (n)
                      (window right-wall
                              (xyz n 0 0.5)
                              #:width 1
                              #:height 1
                              #:type-of-window "Rectangular Window Opening 18")
                      (window back-right-wall
                              (xyz n 0 0.5)
                              #:width 1
                              #:height 1
                              #:type-of-window "Rectangular Window Opening 18"))
                    (division 1.5 (- (abs (- (cx p2) (- (cx p1) balcony-actual-length))) 1.5) number-holes))
               
               (map (lambda (n)
                      (window left-wall
                              (xyz n 0 0.5)
                              #:width 1
                              #:height 1
                              #:type-of-window "Rectangular Window Opening 18")
                      (window back-left-wall
                              (xyz n 0 0.5)
                              #:width 1
                              #:height 1
                              #:type-of-window "Rectangular Window Opening 18"))
                    (division 1.5 (- (abs (- (cx p4) (+ (cx p3) balcony-actual-length))) 1.5) number-holes))
               
               ;Center front glass wall
               #;(define center-front-wall (wall p4 p2 #:material "Glass"))
               ;Center back glass wall
               #;(define center-back-wall (wall (+y p4 building-length) (+y p2  building-length) #:material "Glass"))
               
               
               (define cw-length (abs (- (cx p4) (cx p2))))
               (define cw-initial-distance (+ (abs (- (cx (first inner-wall-curve))(cx (last inner-wall-curve)))) (default-wall-thickness)))
               ;(range 1 (abs (- (cx p4) (cx p2))) (abs (- (cx (first inner-wall-curve))(cx (last inner-wall-curve)))))
               #;(define panels-size (for/list ([i (range 1 cw-length (abs (- (cx (first inner-wall-curve))(cx (last inner-wall-curve)))) )])
                                               (abs (- (cx (first inner-wall-curve))(cx (last inner-wall-curve))))))
               
               #;(define panels-size (cons cw-initial-distance
                                           (append (for/list ([i (range 1 (- cw-length (* cw-initial-distance 2)) panel-size)])
                                                             panel-size)
                                                   (list cw-initial-distance))))
               
               
               (define aux-p4 p4)
               (define aux-p2 p2)
               
               #;(define (aux-function-panels panels-pts p4 p2)
                   (if (null? panels-pts)
                       (list)
                       (let ((pt-x (car panels-pts)))
                         (if (and (> pt-x (cx aux-p4))
                                  (< pt-x (cx aux-p2)))
                             (cons (- pt-x (cx p4)) (aux-function-panels (cdr panels-pts) (+x p4 (- pt-x(cx p4))) p2))
                             (aux-function-panels (cdr panels-pts) (+x p4 (- pt-x (cx p4))) p2)))))
               (define (aux-function-panels panels-pts start finish)
                 (if (null? panels-pts)
                     (list)
                     (let ((pt-x (car panels-pts)))
                       (if (and (> pt-x (cx start))
                                (< pt-x (cx finish)))
                           (cons (- pt-x (cx start))
                                 (aux-function-panels (cdr panels-pts) (+x start (- pt-x (cx start))) finish))
                           (aux-function-panels (cdr panels-pts) start finish)))))
               
               (define panels-size (aux-function-panels panels-pts p4 p2))
               (if (eq? lvl (first levels))
                   (let ((wall-entrance (wall (+y p4 0.1) (+y p2 0.1) #:material "Glass"))
                         ;back
                         (wall-exit (wall (+y p2 (- building-length 0.1)) (+y p4 (- building-length 0.1)) #:material "Glass"))
                         (wall-size (+ (abs (cx p4))
                                       (abs (cx p2)))))
                     (for ([i (range 1 4 1)])
                          (door wall-entrance (x (* i (/ wall-size 4))) #:type "Revolving Door 18" #:width 2.1)
                          (door wall-exit (x (* i (/ wall-size 4))) #:type "Revolving Door 18" #:width 2.1)))
                   (begin
                     ;front curtain-wall
                     (walls (list (+y p4 0.1) (+y p2 0.1)) #:material "Glass")
                     ;back curtain-wall
                     (walls (list (+y p2 (- building-length 0.1)) (+y p4 (- building-length 0.1))) #:material "Glass")))
               ;front curtain-wall
               ;(curtain-wall (list (+y p4 0.1) (+y p2 0.1)) panels-size (list 3) #:panel-material "Glass - Blue")
               ;back curtain-wall
               ;(curtain-wall (list (+y p2 (- building-length 0.1)) (+y p4 (- building-length 0.1))) panels-size (list 3) #:panel-material "Glass - Blue")
             (when (= i 0)
                 #;(door center-front-wall 
                         (abs (/ (- (cx p4) (cx p2)) 2))
                         #:type-of-door "Revolving Door 18"
                         #:width 2.1)
                 #;(door center-back-wall 
                         (abs (/ (- (cx p4) (cx p2)) 2))
                         #:type-of-door "Revolving Door 18"
                         #:width 2.1)
                 (slab (list p4 p2 (+y p2 building-length) (+y p4 building-length))))
               
               
               ;inner-walls
               (profile (string-append "inner-wall-profile-level " (~a i))
                        (append inner-wall-curve inner-wall-inner-curve)
                        )
               (define right-glass-wall (wall p2 (+y p2 building-length)
                                               #:profile-name (string-append "inner-wall-profile-level " (~a i))))
               (define left-glass-wall (wall (+y p4 building-length) p4
                                              #:profile-name (string-append "inner-wall-profile-level " (~a i))))
               (map (lambda (n)
                      (window right-glass-wall
                              (xyz n 0 0.5)
                              #:type-of-window "Rectangular Window Opening 18")
                      (window left-glass-wall
                              (xyz n 0 0.5)
                              #:type-of-window "Rectangular Window Opening 18"))
                    (division 1.5 (- (abs (- (cy p2) (cy (+y p2 building-length)))) 1.5) (* (length list-of-walls) 2)))
               
               ;balcony-walls
               (profile (string-append "balcony-wall-profile-level " (~a i))
                        (append balcony-wall-curve balcony-wall-inner-curve)
                        #:material "Glass")
               (wall p1 (+y p1 building-length)  #:profile-name (string-append "balcony-wall-profile-level " (~a i)))
               (wall (+y p3 building-length) p3  #:profile-name (string-append "balcony-wall-profile-level " (~a i)))
               
               ;structural-wall-balcony
               (wall (+x origin (+ (cx pte) (- balcony-actual-length)))
                            (+xy origin (+ (cx pte) (- balcony-actual-length)) building-length))
               (wall (+xy origin (- (+ (cx pte) (- balcony-actual-length))) building-length)
                            (+x origin (- (+ (cx pte) (- balcony-actual-length)))))
               
               
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
                                                      (+x pt (- wall-thickness)))
                                                    (reverse roof-wall-curve)))
                        (shell-outer-curve (limited-points-superellipse (u0) a b n t (- pi t) n-wall))
                        (shell-inner-curve (map (lambda (pt)
                                                  (+x pt (- wall-thickness)))
                                                (reverse shell-outer-curve)))
                        
                        (facade-curve-aux (map (lambda (pt)
                                                 (+xy pt
                                                      (- (cx (last super-e)))
                                                      (- (cy pt-aux))))
                                               super-e))
                        (facade-curve (cons (x (cx (last facade-curve-aux))) facade-curve-aux)))
                   (profile "roof-walls-profile"
                            (append roof-wall-curve roof-wall-inner-curve))
                   ;(displayln (append roof-wall-curve roof-wall-inner-curve))
                   (define roof-shell (ext-shell shell-outer-curve
                                                 (y (+ building-length wall-thickness))
                                                 #:level (upper-level)))
                   #;(wall pr1 (+y pr1 building-length)
                            #:bottom-level (upper-level)
                            #:profile-name "roof-walls-profile")
                   #;(wall (+y pr3 building-length) pr3
                            #:bottom-level (upper-level)
                            #:profile-name "roof-walls-profile")
                   
                   (define last-facade-wall (wall pr3 pr1 #:bottom-level (upper-level) #:height (- (cy (last super-e))(cy (car super-e)))))
                   (define back-last-facade-wall (wall (+y pr3 building-length) (+y pr1 building-length) #:bottom-level (upper-level) #:height (- (cy (last super-e))(cy (car super-e)))))
                   
                   (map (lambda (n)
                          (window last-facade-wall
                                  (xyz n 0 0.5)
                                  #:width 1
                                  #:height 1
                                  #:type-of-window "Rectangular Window Opening 18")
                          (window back-last-facade-wall
                                  (xyz n 0 0.5)
                                  #:width 1
                                  #:height 1
                                  #:type-of-window "Rectangular Window Opening 18"))
                        (division (+ balcony-actual-length 1.5) (- (abs (- (cx pr3)(cx pr1))) (+ balcony-actual-length 1.5)) (+ (* number-holes 2) 1)))
                   (trim-elements (list last-facade-wall roof-shell))
                   (trim-elements (list back-last-facade-wall roof-shell))
                   
                   #|
                   (profile "facade-wall-last-level" facade-curve)
                   
                   (map (lambda (n)
                          (add-hole-profile "facade-wall-last-level"
                                            (square-center-points (xy n (/ (default-level-to-level-height) 2)) 0.9 1.5 )))
                        (division (default-level-to-level-height)
                                  (- (abs (- (cx p1)(cx p2))) (* (default-level-to-level-height) 1.5))
                                  number-holes))
                   (wall origin (+y origin wall-thickness)
                         #:bottom-level (upper-level)
                         #:profile-name "facade-wall-last-level")
                   (wall (+y origin wall-thickness) origin
                         #:bottom-level (upper-level)
                         #:profile-name "facade-wall-last-level")
                   |#
                   ))))))))

