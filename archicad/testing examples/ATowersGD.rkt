#lang racket
(require rosetta/revit)
(require "main.rkt")
;(require "Messages.rkt")

#|
(parameterize ((default-slab-material "Roof Aluminium")
               (default-roof-material "Roof Aluminium"))
  (send (delete-levels) (myATSlabs-intersect (xyz 0 0 0) 0 150 50)))

(send (delete-levels) (myATSlabs-intersect (xyz 0 0 0) 0 6 2))
(send (delete-levels) (myATSlabs-intersect (xyz 0 0 0) 0 45 15))
(send (delete-levels) (myATSlabs-intersect (xyz 0 0 0) 0 150 50))

|#
;;Functions made by Sofia
;;Superellipse
;Creates a superellipse-shaped surface, as a possible variation to the shape of the slab.
(define (superellipse p a b n t)
  (+xy p (* a (expt (expt (cos t) 2) (/ 1 n)) (sgn (cos t)))
       (* b (expt (expt (sin t) 2) (/ 1 n)) (sgn (sin t)))))

(define (points-superellipse p a b n n-points)
  (map-division (lambda (t) (superellipse p a b n t))
       -pi pi n-points #f))

;;Rotation
;Allows us to rotate the coordinate system, enabling the rotation of the slab.
(define (rotate-z p alfa)
  ;(xyz-on-z-rotation (cx p) (cy p) (cz p) alfa)
  (loc-from-o-phi p alfa)
  ) 

(define (rotation-angle h)
    (/ h 50))

;List of angles of the first tower
(define (list-angles-tower1 n-floors)
  (define floors (map-division (lambda (x) 1) 0 n-floors n-floors #f))
  (define beta
  (cond ((>= (length floors) 47) (/ pi 180))
        ((= (length floors) 46) (/ pi 90))
        ((>= 45 (length floors) 31) (/ pi 60))
        ((= (length floors) 30) (/ pi 36))      ;;;!!!!!!
        ((>= 29 (length floors) 17) (/ pi 22.5))
        ((= (length floors) 16) (/ pi 36))
        ((>= 15 (length floors) 5) (/ pi 60))
        ((= (length floors) 4) (/ pi 90))
        ((>= 3 (length floors)) (/ pi 180))))
        (if (= n-floors 1)
            (list (/ pi 180))
            (cons beta 
                  (list-angles-tower1 (- n-floors 1)))))

;List of angles of the second tower
(define (list-angles-tower2 n-floors)
  (map-division (lambda (alfa) (/ pi 45)) 0 n-floors n-floors #f))  ; 45o -> 3.93

;List of accumulated angles
(define (scan f i L)
  (if (empty? L)
      (list i)
      (cons i (scan f (f i (car L)) (cdr L)))))

(define (accumulated-list L)
  (cdr (scan + 0 L)))

(define (rotation-tower list-angles)
  (accumulated-list (append (list 0) list-angles)))  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;(define ATslab1 (points-superellipse (rotate-z (+c (xyz 0 0 0) (cyl 0 0 0))(rotation-angle 0)) 20 15 1.75 50))
;(set! ATslab1 (append ATslab1 (list (car ATslab1))))
;(define ATslab2 (points-superellipse (rotate-z (+c (xyz 0 0 0) (cyl 0 0 0))(rotation-angle 0)) 17 12 1.75 50))
;(set! ATslab2 (append ATslab2 (list (car ATslab2))))

(define hs (map-division (lambda (h) h) 0 15 5 #f))

(define (myATSlabs-intersect C bottomFloor height n-floors)
  (define wallheight (/ height n-floors))
  (define hs (map-division (lambda (h) h) bottomFloor height n-floors #f))
  
  (define slab-points-aux (points-superellipse C 20 15 1.75 50))
  (define slab-points (append slab-points-aux (list (car slab-points-aux))))
  
  (define interior-slab-aux (points-superellipse C 17 12 1.75 50))
  (define interior-slab (append interior-slab-aux (list (car interior-slab-aux))))
  (define p C)
  (define d 6.3)
  (define l-corridor 2.2)
  (define d1 1.8)
  (define t 0.3)
  (define r-small 15)
  (define bottom 0)
  (define daux6 (- 6.3 l-corridor))
  (define central-hole (list (+xy p (- (+ daux6 (/ t 2))) (- (+ daux6 (/ t 2))))
                             (+xy p (+ (+ daux6 (/ t 2))) (- (+ daux6 (/ t 2))))
                             (+xy p (+ (+ daux6 (/ t 2))) (+ (+ daux6 (/ t 2))))
                             (+xy p (- (+ daux6 (/ t 2))) (+ (+ daux6 (/ t 2))))
                             (+xy p (- (+ daux6 (/ t 2))) (- (+ daux6 (/ t 2))))))
  (define roof-points-aux (points-superellipse C 20 15 1.75 50))
  (define roof-points (append roof-points-aux (list (car roof-points-aux))))
  (define rotation-list-1 (rotation-tower (list-angles-tower1 (- n-floors 1))))
  ;(create-crowd 30 30 (xy -50 -50))
  (map (lambda (level)
       (parameterize ((current-level level)
                      (default-level-to-level-height wallheight))
         (let* ((h (current-level-elevation))
                (current-rotation (car rotation-list-1))
                (slab-id (rotate-element-z (create-slab slab-points) current-rotation))
                (interior-slab-id (rotate-element-z (create-slab interior-slab) current-rotation)))
           (set! rotation-list-1 (cdr rotation-list-1))
           ;(grid-typical-floor-intersect p d l-corridor d1 t r-small interior-slab-id #t)
           (if (>= h (- height wallheight))
               (begin
                 (rotate-element-z (create-roof roof-points #:bottom-level (upper-level) #:type-of-material "Basic" #:material "Glass") current-rotation)
                 (grid-typical-floor-intersect p d l-corridor d1 t r-small interior-slab-id #f))
               (grid-typical-floor-intersect p d l-corridor d1 t r-small interior-slab-id #t))
           (create-walls-from-slab slab-id (/ wallheight 2) #:thickness 0.05)
           ;(create-columns-from-slab slab-id (/ wallheight 2) #:depth 0.15)
           ;(create-walls-from-slab interior-slab-id wallheight #:thickness 0.1)
           ;(create-columns-from-slab interior-slab-id wallheight #:depth 0.2)
           (delete-elements interior-slab-id)
           (create-hole-slab slab-id central-hole)
           )))
     (for/list ([z (range bottomFloor height wallheight)])
       ;(print "Creation ")
       ;(print z)
       ;(newline)
       (create-level #:height z)))
  )
  ;;Roof
  

;;Still uses set!
(define (grid-typical-floor-intersect p d l-corridor d1 t r-small slabID doStairs?)
  (parameterize ((default-wall-thickness t)
                 (default-wall-type-of-material "Basic"))
    (define daux1 (- d (/ t 2)))
    (define daux2 (* 1/3 d))
    (define daux3 (+ d (* t 3)))
    (define daux4 (+ d (* t 3.5)))
    (define daux5 (+ daux2 (/ t 2)))
    (define daux6 (- d l-corridor))
    (define wallList (list))
    
    ;; Middle section shape ;; 
    (define middle-section-shape-1 (create-wall (list (+xy p (- daux2) (+ daux1))   
                                               (+xy p (- daux2) (+ daux3))
                                               (+xy p (+ daux2) (+ daux3))
                                               (+xy p (+ daux2) (+ daux1)))))
    (define middle-section-shape-2 (create-wall (list (+xy p 0 (+ daux4)) (+xy p 0 (* (+ daux4) 5)))))
    
    ;; Hash section shape ;;
    (define hash-section-F-1 (create-wall (list (+xy p (+ d) (+ daux5)) (+xy p d daux1))))
    (define hash-section-F-2 (create-wall (list (+xy p d (+ daux1 1.05)) (+xy p (+ d) (+ (* 4 d))))))
    
    (define hash-section-N-1 (create-wall (list (+xy p (+ daux5) (+ d)) (+xy p (+ daux1 5.2) d))))
    (define hash-section-N-2 (create-wall (list (+xy p (+ daux1 8.8) d) (+xy p (+ (* 4 d)) (+ d)))))
    
    ;; Middle wall in Central core shape ;;
    (define central-shape (create-wall (list (+xy p (+ (- daux6 (/ t 2))) (+ d1))
                                      (+xy p (- (- daux6 (/ t 2))) (+ d1)))))
    
    (create-wall (list (+xy p (- (+ (- daux6 (/ t 2))) 1.2) 0)
                (+xy p (+ (- (- daux6 (/ t 2))) 1.2) 0))
          #:thickness 0.6)
    (create-stairs #:name "Concrete Landing 18" 
            #:orig-pos (+xy p 
                            (- (+ (- daux6 (/ t 2))) 1.6) 
                            (- (- (+ d1) (/ t 2)) (/ t 2))) 
            #:angle (* pi 3/2))
    (create-stairs #:name "Concrete Landing 18" 
            #:orig-pos(+xy p 
                           (+ (- (- daux6 (/ t 2))) 1.6) 
                           (+ (- (- (+ d1) (/ t 2))) (/ t 2))) 
            #:angle (/ pi 2))
    
    (when doStairs?
      (begin
        ;;Top stairs
        (create-stairs #:name "Stair Straight RC 18" 
                #:orig-pos (+xy p 
                                (+ (- (- daux6 (/ t 2))) 1.6) 
                                (- (+ d1) t)) 
                #:angle (* pi 3/2))
        ;;Bottom stairs
        (create-stairs #:name "Stair Straight RC 18" 
                #:orig-pos       (+xy p 
                                      (- (+ (- daux6 (/ t 2))) 1.6) 
                                      (- (- (+ d1) t))) 
                #:angle (/ pi 2))
        )
      )
     
    ;; Pillar shape ;;
    ;(define pillar-shape (create-column (+pol p r-small (* pi 3/4))))
    
    (set! wallList (append wallList (list middle-section-shape-1 middle-section-shape-2 hash-section-F-1 hash-section-F-2 hash-section-N-1 hash-section-N-2 central-shape 
                                          ;pillar-shape
                                          )))
    
    ;;; Middle section ;;;
    (set! wallList (append wallList (list (rotate-element-z middle-section-shape-1 (/ pi 2) #t)
                                          (rotate-element-z middle-section-shape-2 (/ pi 2) #t)
                                          (rotate-element-z middle-section-shape-1 pi #t)
                                          (rotate-element-z middle-section-shape-2 pi #t)
                                          (rotate-element-z middle-section-shape-1 (* pi 3/2) #t)
                                          (rotate-element-z middle-section-shape-2 (* pi 3/2) #t))))
    ;;; Hash section ;;;
    (define haux1 (mirror-element-x hash-section-N-1))
    (set! wallList (append wallList (list (mirror-element-y haux1))))
    (set! wallList (append wallList (list (mirror-element-y hash-section-N-1))))
    
    (define haux2 (mirror-element-x hash-section-N-2))
    (set! wallList (append wallList (list (mirror-element-y haux2))))
    (set! wallList (append wallList (list (mirror-element-y hash-section-N-2))))
    
    (define haux3 (mirror-element-x hash-section-F-1))
    (set! wallList (append wallList (list (mirror-element-y haux3))))
    (set! wallList (append wallList (list (mirror-element-y hash-section-F-1))))
    
    (define haux4 (mirror-element-x hash-section-F-2))
    (set! wallList (append wallList (list (mirror-element-y haux4))))
    (set! wallList (append wallList (list (mirror-element-y hash-section-F-2))))
    
    (set! wallList (append wallList (list haux1 haux2 haux3 haux4)))
    
    ;;; Central Core ;;;;
    (create-wall (list (+xy p (- daux6) (- (+ daux6 (/ t 2))))    ;;;Not the best solution... The best solution would be using a closed-line
                (+xy p (- daux6) (+ daux6))                 ;;closed-line doesn't work
                (+xy p (+ daux6) (+ daux6))
                (+xy p (+ daux6) (- (+ daux6 (/ t 2))))))
    (create-wall (list (+xy p (+ (- daux6 (/ t 2))) (- daux6))
                (+xy p (- (- daux6 (/ t 2))) (- daux6))))
    (set! wallList (append wallList (list (rotate-element-z central-shape pi #t))))
    ;;; Pillars ;;;                               
    ;(set! wallList (append wallList (list (rotate-element-z pillar-shape pi #t))))
    
    (for-each (lambda (wallID)
                (intersect-wall wallID slabID)) 
              (flatten wallList)))
  )

#|
apartamento N tem 3,6m de largura e centro em (+xy p (+ daux1 7) d) 
apartamento F tem 1,05m de largura e centro em (+xy p d (+ daux1 0,525))
|#
#;(define (apartments p d t height)
  (define daux1 (+ d (/ t 2)))
  (define wall-N (create-multi-wall-composite (list (+xy p (+ daux1 5.2) d) (+xy p (+ daux1 8.8) d)) height t))
  (define wall-F (create-multi-wall-composite (list (+xy p d daux1) (+xy p d (+ daux1 1.05))) height t))
  (define wall-list (list wall-N wall-F))
  (for-each (lambda (wallID)
              (mirror-element-y wallID)
              (mirror-element-y (mirror-element-x wallID)))
            (flatten wall-list))
)

(define (building-park)
  (park 1324 (xy -30 -30) 1 8 8)
  (park 1324 (xy -30 36) 8 1 8)
  (park 1324 (xy 36 -30) 1 9 8)
  (park 1324 (xy -30 -30) 8 1 8))

(define (random-person-number)
  (let ((random-number (random 10)))
    (cond [(eq? random-number 0) 1359]
          [(eq? random-number 1) 1360]
          [(eq? random-number 2) 1363]
          [(eq? random-number 3) 1364]
          [(eq? random-number 4) 1366]
          [(eq? random-number 5) 1367]
          [(eq? random-number 6) 1368]
          [(eq? random-number 7) 1370]
          [(eq? random-number 8) 1371]
          [(eq? random-number 9) 1372]
          [else 0])))
(define internal-crowd-step 4)
(define (create-crowd-line quantity inferior-right-corner step)
  (let ((exist? (random 4)))
    (when (not (eq? quantity 0))
      (begin
        (when (not (eq? exist? 0))
          (create-object (random-person-number) 
                         (xy (+  (cx inferior-right-corner) step) (cy inferior-right-corner))))
        (create-crowd-line (- quantity 1) inferior-right-corner (+ step internal-crowd-step))))))
(define (create-crowd rows columns inferior-right-corner)
  (when (not (eq? rows 0))
    (begin
      (create-crowd-line columns inferior-right-corner 0)
      (create-crowd (- rows 1) 
                    columns 
                    (xy (cx inferior-right-corner)
                                           (+ (cy inferior-right-corner) internal-crowd-step))))))

;; (send (intersect-wall (create-wall (xy -30.0 5.0) (xy 30.0 5.0) 0.0 3.0 0.3 0) (create-slab ATslab1) )) 
;; (send (grid-typical-floor-intersect (xyz 0 0 0) 6.3 2.2 0.3 15 0 10 (create-slab ATslab1)))


;; (send (myATSlabs-intersect (xyz 0 0 0) 0 50 10))








#|
 (define p (xyz 0 0 0))
  (define d 6.3)
  (define l-corridor 2.2)
  (define d1 1.8)
  (define t 0.3)
  (define r-small 15)
  (define bottom 0)
|#
;(send (central-core (x 0) 6.3 2.2 1.8 0.3 3))

#;(define (central-core p d l-corridor d1 t h)
  (define daux1 (- d (/ t 2)))
  (define daux2 (* 1/3 d)) 
  (define daux3 (+ d (* t 3)))
  (define daux4 (+ d (* t 3.5)))
  (define daux5 (+ daux2 (/ t 2)))
  (define daux6 (- d l-corridor))
  (define daux7 (- (- d l-corridor) (/ t 2)))
  (define central-core-id (create-multi-wall (list (+xy p (- daux6) (- daux6))   
                                                (+xy p (- daux6) (+ daux6))
                                                (+xy p (+ daux6) (+ daux6))
                                                (+xy p (+ daux6) (- daux6))
                                                (+xy p (- daux6) (- daux6)))
                                          h))
  (define stairway-door-width 1.1)
  (define stairway-door-heigth 2.1)
  (define elevator-door-width 1.0)
  (define elevator-door-heigth 2.1)
    ;;; Doors ;;;;
  (create-door (car central-core-id) (+ daux6 (- d1 (/ t 2) stairway-door-width)) stairway-door-width 0 stairway-door-heigth)
  (create-door (car (cdr central-core-id)) (+ (* (- (* daux6 2) t) 1/6) (/ t 2)) elevator-door-width 0 elevator-door-heigth)
  (create-door (car (cdr central-core-id)) (+ (* (- (* daux6 2) t) 1/2) (/ t 2)) elevator-door-width 0 elevator-door-heigth)
  (create-door (car (cdr central-core-id)) (+ (* (- (* daux6 2) t) 5/6) (/ t 2)) elevator-door-width 0 elevator-door-heigth)
  (create-door (car (cdr (cdr central-core-id))) (+ daux6 (- d1 (/ t 2) stairway-door-width)) stairway-door-width 0 stairway-door-heigth)
  (create-door (last central-core-id) (+ (* (- (* daux6 2) t) 1/6) (/ t 2)) elevator-door-width 0 elevator-door-heigth)
  (create-door (last central-core-id) (+ (* (- (* daux6 2) t) 1/2) (/ t 2)) elevator-door-width 0 elevator-door-heigth)
  (create-door (last central-core-id) (+ (* (- (* daux6 2) t) 5/6) (/ t 2)) elevator-door-width 0 elevator-door-heigth))

#|
Notes: We need to have the levels already created in order to work.
When we get the information regarding the objects, we get the information of levels
that object belongs to. However, if we delete the levels, and then try to create the objects, their levels
might not be created, so the application makes everything in the first level...
(send (define walls (get-walls))
(define levels (get-levels))
(define slabs (get-slabs))
(define objects (get-objects))
(define roofs (get-roofs))
(delete-levels)
(recreate-levels levels)
(recreate-walls walls)
(recreate-slabs slabs)
(recreate-objects objects)
(recreate-roofs roofs))


(delete-elements (wallrepeated-guid walls))
(delete-elements (slabrepeated-guid slabs))
(delete-elements (objectrepeated-guid objects))
(delete-elements (roofrepeated-guid roofs))
|#
