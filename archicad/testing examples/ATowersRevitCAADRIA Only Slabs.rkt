#lang racket
(require rosetta/revit)
(require "main.rkt")

(define (rotate-z p alfa)
  (loc-from-o-phi p alfa))    ;;;Deixou de ser necessária?

(define (division a b n end?)
  (map-division (lambda (x) x) a b n end?))   ;;;Até o professor arranjar o bug do division

;;;Auxiliary Functions

(define (rotate-points lst angle)
  (for/list ((pt lst))
    (pol (pol-rho pt) (+ (pol-phi pt) angle))))

;;Superellipse
;Creates a superellipse-shaped surface, as a possible variation to the shape of the slab.
(define (superellipse p a b n t)
  (+xy p
       (* a (expt (expt (cos t) 2) (/ 1 n)) (sgn (cos t)))
       (* b (expt (expt (sin t) 2) (/ 1 n)) (sgn (sin t)))))

(define (points-superellipse p a b n n-points)
  ;TODO not using closed line for slab!!!
  (map-division (lambda (t) (superellipse p a b n t)) -pi pi n-points #f))
 
;;List of angles of the first tower
(define (list-angles-tower1 n-floors)
  (define floors (map-division (lambda (x) 1) 0 n-floors n-floors #f))
  (define beta
  (cond ((>= (length floors) 47) (/ pi 180))
        ((= (length floors) 46) (/ pi 90))
        ((>= 45 (length floors) 31) (/ pi 60))
        ((= (length floors) 30) (/ pi 36))      
        ((>= 29 (length floors) 17) (/ pi 22.5))
        ((= (length floors) 16) (/ pi 36))
        ((>= 15 (length floors) 5) (/ pi 60))
        ((= (length floors) 4) (/ pi 90))
        ((>= 3 (length floors)) (/ pi 180))))
        (if (= n-floors 1)
            (list (/ pi 180))
            (cons beta 
                  (list-angles-tower1 (- n-floors 1)))))

;;List of angles of the second tower
(define (list-angles-tower2 n-floors)
  (map-division (lambda (alfa) (/ pi 25)) 0 n-floors n-floors #f))

;;List of accumulated angles
(define (scan f i L)
  (if (empty? L)
      (list i)
      (cons i (scan f (f i (car L)) (cdr L)))))

(define (accumulated-list L)
  (cdr (scan + 0 L)))

(define (rotation-tower list-angles)
  (accumulated-list (append (list 0) list-angles)))   

;;;;;;Building Geometry;;;;;;;
;;;Walls
;Section walls is the result of a sweep through the walls axis
#;(define (section-wall curve t h)
  (sweep curve (surface-rectangle (x (/ t -2)) t h)))                              

;Grid typical Floor
(define (grid-typical-floor-intersect p d l-corridor d1 r-small t slabID)
  (define daux1 (- d (/ t 2)))
  (define daux2 (* 1/3 d))
  (define daux3 (+ d (* t 3)))
  (define daux4 (+ d (* t 3.5)))
  (define daux5 (+ daux2 (/ t 2)))
  (define daux6 (- d l-corridor))
  (define daux7 (- (- d l-corridor) (/ t 2)))
  ;;; Middle section ;;;
  (define wall-list
    (list
     (create-wall (list (+xy p (- daux2) (+ daux1))   
                        (+xy p (- daux2) (+ daux3))
                        (+xy p (+ daux2) (+ daux3))
                        (+xy p (+ daux2) (+ daux1))))
     (create-wall (list (+xy p (- daux2) (- daux1))
                        (+xy p (- daux2) (- daux3))
                        (+xy p (+ daux2) (- daux3))
                        (+xy p (+ daux2) (- daux1)))) 
     (create-wall (list (+xy p (- daux1) (+ daux2))
                        (+xy p (- daux3) (+ daux2))
                        (+xy p (- daux3) (- daux2))
                        (+xy p (- daux1) (- daux2))))
     (create-wall (list (+xy p (+ daux1) (+ daux2))
                        (+xy p (+ daux3) (+ daux2))
                        (+xy p (+ daux3) (- daux2))
                        (+xy p (+ daux1) (- daux2))))
     (create-wall (list (+xy p 0 (- daux4)) (+xy p 0 (* (- daux4) 5)))) 
     (create-wall (list (+xy p 0 (+ daux4)) (+xy p 0 (* (+ daux4) 5))))
     (create-wall (list (+xy p (- daux4) 0) (+xy p (* (- daux4) 5) 0)))
     (create-wall (list (+xy p (+ daux4) 0) (+xy p (* (+ daux4) 5) 0)))
     ;;; Hashtag section ;;;    
     (create-wall (list (+xy p (- (* 4 d)) (- d)) (+xy p (- daux5) (- d)))) 
     (create-wall (list (+xy p (+ daux5) (- d)) (+xy p (+ (* 4 d)) (- d)))) 
     (create-wall (list (+xy p (- (* 4 d)) (+ d)) (+xy p (- daux5) (+ d))))
     (create-wall (list (+xy p (+ daux5) (+ d)) (+xy p (+ (* 4 d)) (+ d))))
     (create-wall (list (+xy p (+ d) (- (* 4 d))) (+xy p (+ d) (- daux5)))) 
     (create-wall (list (+xy p (+ d) (+ daux5)) (+xy p (+ d) (+ (* 4 d))))) 
     (create-wall (list (+xy p (- d) (- (* 4 d))) (+xy p (- d) (- daux5))))
     (create-wall (list (+xy p (- d) (+ daux5)) (+xy p (- d) (+ (* 4 d)))))
     ;;; Central Core ;;;;
     (create-wall (list (+xy p (- daux6) (- daux6))   
                        (+xy p (+ daux6) (- daux6))
                        (+xy p (+ daux6) (+ daux6))
                        (+xy p (- daux6) (+ daux6))
                        (+xy p (- daux6) (- daux6))))
     (create-wall (list (+xy p (+ (- daux6 (/ t 2))) (- d1))
                        (+xy p (- (- daux6 (/ t 2))) (- d1))))
     (create-wall (list (+xy p (+ (- daux6 (/ t 2))) (+ d1))
                        (+xy p (- (- daux6 (/ t 2))) (+ d1))))
     ;;; Walls 45º ;;;
     (create-wall (list (+pol p r-small pi/4) (+pol p (* r-small 2) pi/4)))      
     (create-wall (list (+pol p r-small (* 2pi 5/8)) (+pol p (* r-small 2) (* 2pi 5/8))))
     ;;; Wall for stair division
     (create-wall (list (+xy p (- (+ (- daux6 (/ t 2))) 1.2) 0)
                        (+xy p (+ (- (- daux6 (/ t 2))) 1.2) 0)))))
  
  ;;; Pillars ;;;
  (create-column (+pol p r-small (* pi 3/4)))                  ;pilar não pode cortar; quando corta, não existe pilar                          
  (create-column (+pol p r-small (* 2pi 7/8)))
  
  (for-each (lambda (wallID)
              (intersect-wall wallID slabID)) 
            (flatten wall-list)))

;;;Building
(define (building #:central-point C
                    ;;Shapes
                  #:slab slab-shape
                  #:guardrail guardrail-shape
                  #:railing rail-shape
                  #:guardrail-post guardrail-post
                  #:handrail handrail-shape
                  #:interior interior-shape        
                  #:interior-points interior-points       
                  #:walls walls-grid
                    ;;Rotation
                  #:list-angles-rotation list-angles
                    ;;Distances 
                  #:distance-d [d 6.3]
                  #:corridor-width [w-corridor 2.0]            
                  #:distance-d1 [d1 1.8]                     
                    ;;Thicknesses
                  #:slab-thickness [t-slab 0.3]
                  #:roof-slab-thickness [t-roof-slab 0.5]
                  #:wall-thickness [t-walls 0.3]
                  #:stair-thickness [t-stair 0.2]
                  #:parapet-thickness [t-parapet (* t-walls 2/3)]
                    ;;Stairs Restrictions 
                  #:minimum-riser-value [r-min 0.14]
                  #:maximum-riser-value [r-max 0.18]
                  #:minimum-tread-value [t-min 0.28]
                  #:minimum-blondel-value [b-min 0.62]
                  #:maximum-blondel-value [b-max 0.64]
                    ;;Building Height and number of floors
                  #:building-height height 
                  #:number-of-floors n-floors)
                  ;;#:stairs stairs
                  ;;#:landings landings)
  (define hs (map-division (lambda (h) h) 0 height n-floors #f))
  (define rots (rotation-tower (list-angles (- n-floors 1))))
  (define wallheight (/ height n-floors))
  (define l-corridor 2.2)
  (define daux6 (- 6.3 l-corridor))
  (define t 0.3)
  (define central-hole (list (+xy C (- (+ daux6 (/ t 2))) (- (+ daux6 (/ t 2))))
                             (+xy C (+ (+ daux6 (/ t 2))) (- (+ daux6 (/ t 2))))
                             (+xy C (+ (+ daux6 (/ t 2))) (+ (+ daux6 (/ t 2))))
                             (+xy C (- (+ daux6 (/ t 2))) (+ (+ daux6 (/ t 2))))
                             #;(+xy C (- (+ daux6 (/ t 2))) (- (+ daux6 (/ t 2))))))
  (define levels-list (for/list ([z (range 0 height wallheight)])
                 (create-level #:height z)))
  ;;Floors of the building - slabs, walls, columns and stairs
  (map (lambda (level alfa)
         (parameterize ((current-level level)
                        (default-level-to-level-height wallheight))
           (let* ((h (current-level-elevation))
                  (slab-id (create-slab (slab-shape alfa)))
                  (interior-slab-id (create-slab (interior-shape alfa))))
             ;(walls-grid  interior-slab-id)
             ;(create-walls-from-slab slab-id (/ wallheight 2))
             ;(create-columns-from-slab slab-id (/ wallheight 2) #:depth 0.15)
             ;(create-walls-from-slab interior-slab-id wallheight #:thickness 0.1)
             ;(create-columns-from-slab interior-slab-id wallheight #:depth 0.2)
             ;TODO Name difference, ArchiCAD: delete-elements, Revit: delete-element
             (delete-elements interior-slab-id)
             ;(create-hole-slab slab-id central-hole)
             ;;(unless (eq? level (last levels-list))
               ;;(stairs))
             ;;(landings)
             )))
       levels-list
       rots)
  #;(create-roof (interior-shape (last rots))
               #:bottom-level (upper-level #:level (last levels-list) #:height wallheight)))


;;;Absolute Towers Final
;In relation to a central point C
(define (absolute-towers C [building-height 176] [number-of-floors 56])
    (building #:central-point C
              #:slab (lambda (alfa)                  
                        (rotate-points (points-superellipse C 21 15 1.75 50) alfa))          ;chosen shape defines the shape of the slab / changes to the radius allows changes of scale
              #:guardrail (lambda (h alfa) 
                           (define p (rotate-z (+c C (cyl 0 0 h))   
                                               alfa))
                           (points-superellipse p 21 15 1.75 50))
              #:railing (lambda (pt1 pt2) '())     
              #:guardrail-post (lambda (p) '())
              #:handrail '()
              #:interior (lambda (alfa)                 
                           (rotate-points (points-superellipse C 18.5 13.6 1.75 50) alfa))
              #:interior-points (lambda (alfa)                
                           (rotate-points (points-superellipse C 18 13.6 1.75 50) alfa))
              #:walls (lambda (slab-to-intersect) (grid-typical-floor-intersect C 6.3 2.2 1.8 12 0.3 slab-to-intersect))
              #:list-angles-rotation (lambda (n) (list-angles-tower2 n))  
              #:building-height building-height
              #:number-of-floors number-of-floors
              ;;#:stairs (lambda () (floor-stairs C 6.3 2.2 1.8 0.3))
              ;;#:landings (lambda () (floor-landings C 6.3 2.2 1.8 0.3))))
              ))
;This code should be in the program? Is it not better to just write on the REPL?
;(connect-to-revit)
;(absolute-towers (xyz 0 0 0))