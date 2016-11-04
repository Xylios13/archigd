#lang racket
(require "main.rkt")
(require "RailingLibPart.rkt")
;(require "Messages.rkt")

#|

Torre dois: Andares: 50; Altura: 150

(send (delete-levels) (absolute-towers (u0) 6 2))
(send (delete-levels)(hide-layer (default-layer)) (absolute-towers (u0) 90 30)(show-layer (default-layer)))
(send (view-2d)(delete-levels)(hide-layer (default-layer)) (absolute-towers (u0) 90 30)(show-layer (default-layer))(view-3d))
#;Slabs
(send (absolute-towers (u0) 176 56 #:list-angles-rotation list-angles-tower1))
(send (absolute-towers (u0) 176 56 #:list-angles-rotation list-angles-tower2))
;;;;

#:Railings
(send (delete-levels)(absolute-towers (u0) 44 14 #:lod 1))
(send (delete-levels)(absolute-towers (u0) 44 14 #:lod 3))



|#
;;Functions made by Sofia
;;Superellipse
;Creates a superellipse-shaped surface, as a possible variation to the shape of the slab.
(define (superellipse p a b n t)
  (+xy p
       (* a (expt (expt (cos t) 2) (/ 1 n)) (sgn (cos t)))
       (* b (expt (expt (sin t) 2) (/ 1 n)) (sgn (sin t)))))

;;Changed to repeat first point
(define (points-superellipse p a b n n-points)
  (map-division (lambda (t) (superellipse p a b n t)) -pi pi n-points #f))

;;Rotation
;Allows us to rotate the coordinate system, enabling the rotation of the slab.
(define (rotate-z p alfa)
  ;(xyz-on-z-rotation (cx p) (cy p) (cz p) alfa)
  (loc-from-o-phi p alfa)
  ) 

(define (rotation-angle h)
  (/ h 50))

;List of angles of the first tower
(define (list-angles-tower1 n-floors [rotation 0])
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

;(/ pi 
;List of angles of the second tower
(define (list-angles-tower2 n-floors [rotation (/ pi 45)])
  (map-division (lambda (alfa) rotation)
                0 n-floors n-floors #f))  ; 45o -> 3.93

;List of accumulated angles
(define (scan f i L)
  (if (empty? L)
      (list i)
      (cons i (scan f (f i (car L)) (cdr L)))))

(define (accumulated-list L)
  (cdr (scan + 0 L)))

(define (rotation-tower list-angles)
  (accumulated-list (append (list 0) list-angles)))

(define (rotate-points lst angle)
  (for/list ((pt lst))
            (pol (pol-rho pt) (+ (pol-phi pt) angle))))
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (grid-typical-floor-intersect p d l-corridor d1 r-small t slabID)
  (parameterize ((default-wall-thickness t)
                 (default-wall-type-of-material "Basic")
                 (default-wall-material "Plaster - Gypsum"))
    (define daux1 (- d (/ t 2)))
    (define daux2 (* 1/3 d))
    (define daux3 (+ d (* t 3)))
    (define daux4 (+ d (* t 3.5)))
    (define daux5 (+ daux2 (/ t 2)))
    (define daux6 (- d l-corridor))
    (define daux7 (- (- d l-corridor) (/ t 2)))
    (define wall-list
      (list (walls (list (+xy p (- daux2) (+ daux1))   
                               (+xy p (- daux2) (+ daux3))
                               (+xy p (+ daux2) (+ daux3))
                               (+xy p (+ daux2) (+ daux1))))
            (walls (list (+xy p (- daux2) (- daux1))
                               (+xy p (- daux2) (- daux3))
                               (+xy p (+ daux2) (- daux3))
                               (+xy p (+ daux2) (- daux1))))
            (walls (list (+xy p (- daux1) (+ daux2))
                               (+xy p (- daux3) (+ daux2))
                               (+xy p (- daux3) (- daux2))
                               (+xy p (- daux1) (- daux2))))
            (walls (list (+xy p (+ daux1) (+ daux2))
                               (+xy p (+ daux3) (+ daux2))
                               (+xy p (+ daux3) (- daux2))
                               (+xy p (+ daux1) (- daux2))))
            (walls (list (+xy p 0 (- daux4)) (+xy p 0 (* (- daux4) 5)))) 
            (walls (list (+xy p 0 (+ daux4)) (+xy p 0 (* (+ daux4) 5))))
            (walls (list (+xy p (- daux4) 0) (+xy p (* (- daux4) 5) 0)))
            (walls (list (+xy p (+ daux4) 0) (+xy p (* (+ daux4) 5) 0)))
            ;;; Hashtag section ;;;    
            (walls (list (+xy p (- (* 4 d)) (- d)) (+xy p (- daux5) (- d)))) 
            (walls (list (+xy p (+ daux5) (- d)) (+xy p (+ (* 4 d)) (- d)))) 
            (walls (list (+xy p (- (* 4 d)) (+ d)) (+xy p (- daux5) (+ d))))
            (walls (list (+xy p (+ daux5) (+ d)) (+xy p (+ (* 4 d)) (+ d))))
            (walls (list (+xy p (+ d) (- (* 4 d))) (+xy p (+ d) (- daux5)))) 
            (walls (list (+xy p (+ d) (+ daux5)) (+xy p (+ d) (+ (* 4 d))))) 
            (walls (list (+xy p (- d) (- (* 4 d))) (+xy p (- d) (- daux5))))
            (walls (list (+xy p (- d) (+ daux5)) (+xy p (- d) (+ (* 4 d)))))
            ;;; Central Core ;;;;
            (walls (list (+xy p (- daux6) (- daux6))   
                               (+xy p (+ daux6) (- daux6))
                               (+xy p (+ daux6) (+ daux6))
                               (+xy p (- daux6) (+ daux6))
                               (+xy p (- daux6) (- daux6))))
            (walls (list (+xy p (+ (- daux6 (/ t 2))) (- d1))
                               (+xy p (- (- daux6 (/ t 2))) (- d1))))
            (walls (list (+xy p (+ (- daux6 (/ t 2))) (+ d1))
                               (+xy p (- (- daux6 (/ t 2))) (+ d1)))) 
            ;;; Walls 45 ;;;
            (walls (list (+pol p r-small pi/4) (+pol p (* r-small 2) pi/4)))      
            (walls (list (+pol p r-small (* 2pi 5/8)) (+pol p (* r-small 2) (* 2pi 5/8))))
            ;;; Wall for stair division ;;;
            (walls (list (+xy p (- (+ (- daux6 (/ t 2))) 1.2) 0)
                               (+xy p (+ (- (- daux6 (/ t 2))) 1.2) 0))
                         #:thickness 0.6)))
    
    ;;; Pillars ;;;
    ;(column (+pol p r-small (* pi 3/4)) #:depth t #:circle-based? #t)                  ;pilar não pode cortar; quando corta, não existe pilar                          
    ;(column (+pol p r-small (* 2pi 7/8)) #:depth t #:circle-based? #t)
    
    (for-each (lambda (wallID)
                (list)
                (intersect-wall wallID slabID)) 
              (flatten wall-list))))

(define (floor-stairs p d l-corridor d1 t)
  (define daux6 (- d l-corridor))
  ;;Top stairs
  (stairs "Stair Straight RC 18" 
          (+xy p 
               (+ (- (- daux6 (/ t 2))) 1.6) 
               (- (+ d1) t)) 
          #:angle (* pi 3/2))
  ;;Bottom stairs
  (stairs "Stair Straight RC 18" 
          (+xy p 
               (- (+ (- daux6 (/ t 2))) 1.6) 
               (- (- (+ d1) t))) 
          #:angle (/ pi 2)))

(define (floor-landings p d l-corridor d1 t)
  (define daux6 (- d l-corridor))
  (stairs  "Concrete Landing 18" 
                  (+xy p 
                       (- (+ (- daux6 (/ t 2))) 1.6) 
                       (- (- (+ d1) (/ t 2)) (/ t 2))) 
                  #:angle (* pi 3/2))
  (stairs  "Concrete Landing 18" 
                  (+xy p 
                       (+ (- (- daux6 (/ t 2))) 1.6) 
                       (+ (- (- (+ d1) (/ t 2))) (/ t 2))) 
                  #:angle (/ pi 2)))

(define (railing pts height [lod 0])
  (if (= lod 0)
      ; (0) used for walls with columns and beams
      (walls pts #:thickness 0.05 #:material "Glass Mirror" #:height height)
      (for ((p0 pts)
            (p1 (cdr pts))
            (i (length pts)))
           (let ([properties-list-1 (list "lra" (sqrt (+ (expt (- (cx p1)(cx p0)) 2)(expt (- (cy p1)(cy p0)) 2)))
                                          "fsw" 0.05
                                          "ftw" 0.01
                                          "fbw" 0.01
                                          "fdf" 0
                                          "ds" 0.05
                                          "fmat" "Metal - Aluminium")])
             (cond
               [(= lod 1)
                (object "Rail Solid Frame Filled 18"
                        p0
                        #:angle (+ (sph-phi (p-p p1 p0)) 0)
                        #:properties properties-list-1)]
               [(= lod 2)
                (object "Railing Vertical 18"
                        p0
                        #:angle (+ (sph-phi (p-p p1 p0)) 0)
                        #:properties properties-list-1)]
               [(= lod 3)
                (let ((rail-length (sqrt (+ (expt (- (cx p1)(cx p0)) 2)(expt (- (cy p1)(cy p0)) 2))))
                      (angle (sph-phi (p-p p1 p0))))
                  (object "BALUSTRADE_SENTREL_BO"
                          #;"railing-test"
                          (if (even? i)
                              p0
                              (+xy p0 (* (cos angle) rail-length)(* (sin angle) rail-length)))
                          #:angle (if (even? i)
                                      (- angle pi/2)
                                      (+ angle pi/2))
                          #:height -0.1
                          #:properties (list "lLen" (sqrt (+ (expt (- (cx p1)(cx p0)) 2)(expt (- (cy p1)(cy p0)) 2)))
                                             "lSlo" (/ pi 18)
                                             "mHandrail" "Paint - Anthracite"
                                             "mRail" "Metal - Stainless Steel")))]
               [(= lod 4)
                (let ((rail-length (sqrt (+ (expt (- (cx p1)(cx p0)) 2)(expt (- (cy p1)(cy p0)) 2))))
                      (angle (sph-phi (p-p p1 p0))))
                  (object "railing-test"
                          (if (even? i)
                              p0
                              (+xy p0 (* (cos angle) rail-length)(* (sin angle) rail-length)))
                          #:angle (if (even? i)
                                      (- angle pi/2)
                                      (+ angle pi/2))
                          #:height -0.1
                          #:properties (list "lLen" (sqrt (+ (expt (- (cx p1)(cx p0)) 2)(expt (- (cy p1)(cy p0)) 2)))
                                             "lSlo" (/ pi 18)
                                             "mHandrail" "Paint - Anthracite"
                                             "mRail" "Metal - Stainless Steel")))]
               )))))


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
                  #:number-of-floors n-floors
                  #:stairs stairs
                  #:landings landings
                  #:lod lod)
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
                                (level z)))
  (define (points-polygon p r n)
    (map-division (lambda (alfa) (+pol p r alfa)) 0 2pi n #f))
  ;;Floors of the building - slabs, walls, columns and stairs
  (map (lambda (level alfa name-i)
         (parameterize ((current-level level)
                        (default-level-to-level-height wallheight))
           (let* ((h (current-level-elevation))
                  (slab-id (slab (slab-shape alfa) #:type-of-material "Composite" #:material "Generic Wall/Shell"))
                  #;(interior-slab-id (slab #;(points-polygon (u0) 13.6 100) (interior-shape alfa)))
                  #;(interior-slab-id (slab #;(points-polygon (u0) 13.6 100) (interior-shape alfa) #:type-of-material "Basic" #:material "GENERIC - STRUCTURAL" #:thickness wallheight #:bottom wallheight ))
                  (interior-slab-id (slab #;(points-polygon (u0) 13.6 100) (interior-shape alfa) #:type-of-material "Basic" #:material "GENERIC - STRUCTURAL")))
             (walls-grid  interior-slab-id)
             (delete-elements interior-slab-id)
             (unless (eq? level (first levels-list))
               (railing (append (slab-shape alfa) (list (car (slab-shape alfa)))) (/ wallheight 2) lod)
               (hole-slab slab-id central-hole)
               (landings))
             (unless (eq? level (last levels-list))
               (stairs))
             )))
       levels-list
       rots
       (range 0 (length levels-list) 1))
  (roof #;(points-polygon (u0) 13.6 100)
        (interior-shape (last rots))
        #:bottom-level (upper-level (last levels-list) wallheight)
        #:type-of-material "Basic"
        #:material "GENERIC - STRUCTURAL"))

(define (absolute-towers C [building-height 176] [number-of-floors 56] [ellipse-n 1.75] #:rotation [rotation (/ pi 40)] #:scale [scale 1] #:list-angles-rotation [list-angles-rotation list-angles-tower1] #:lod [lod 0])
  (lib-part-railing)
  (building #:central-point C
            #:slab (lambda (alfa)
                     (rotate-points (points-superellipse C 20 15 ellipse-n 50) alfa)
                     #;(rotate-points (points-superellipse C 25 20 ellipse-n 4) alfa))          ;chosen shape defines the shape of the slab / changes to the radius allows changes of scale
            #:guardrail (lambda (h alfa) 
                          (define p (rotate-z (+c C (cyl 0 0 h))   
                                              alfa))
                          (points-superellipse p 20 15 ellipse-n 50)
                          #;(points-superellipse p 25 20 ellipse-n 4))
            #:railing (lambda (pt1 pt2) '())     
            #:guardrail-post (lambda (p) '())
            #:handrail '()
            #:interior (lambda (alfa)                  
                         (rotate-points (points-superellipse C 17 12 ellipse-n 50) alfa)
                         #;(rotate-points (points-superellipse C 23 17 ellipse-n 4) alfa))
            #:interior-points (lambda (alfa)                   
                                (rotate-points (points-superellipse C 17 12 ellipse-n 50) alfa)
                                #;(rotate-points (points-superellipse C 23 17 ellipse-n 4) alfa))
            #:walls (lambda (slab-to-intersect) (grid-typical-floor-intersect C 6.3 2.2 1.8 12 0.3 slab-to-intersect))
            #:list-angles-rotation (lambda (n)
                                     (list-angles-rotation n rotation))
            #;(lambda (n)
                (list-angles-tower1 n)
                #;(list-angles-tower2 n))  
            #:building-height building-height
            #:number-of-floors number-of-floors
            #:stairs (lambda () (floor-stairs C 6.3 2.2 1.8 0.3))
            #:landings (lambda () (floor-landings C 6.3 2.2 1.8 0.3))
            #:lod lod))

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
          (object (random-person-number) 
                         (xy (+ (cx inferior-right-corner) step) (cy inferior-right-corner))))
        (create-crowd-line (- quantity 1) inferior-right-corner (+ step internal-crowd-step))))))
(define (create-crowd rows columns inferior-right-corner)
  (when (not (eq? rows 0))
    (begin
      (create-crowd-line columns inferior-right-corner 0)
      (create-crowd (- rows 1) 
                    columns 
                    (xy (cx inferior-right-corner)
                        (+ (cy inferior-right-corner) internal-crowd-step))))))

(define number-frames 10)
(define min 0)
(define max 10)
(define variation-list (division min max number-frames))

(define (renders-loop)
  ;(connect)
  (for ([i variation-list]
        [name-i (length variation-list)])
       (let ((before-sec (current-seconds)))
         ;(hide-layer (default-layer))
         (delete-all-elements)
         (view-2d)
         ;(absolute-towers (u0) 6 2 i)
         (absolute-towers (u0) (+ 120 (* i 3)) (+ 40 i) #:list-angles-rotation list-angles-tower2)
         (render (string-append (string-append "C:\\Renders\\FloorsRender" (~a name-i)) ".png"))
         ;(show-layer (default-layer))
         (view-3d)
         (displayln (- (current-seconds) before-sec))))
  (disconnect))

#;(renders-loop)







