#lang racket
(require "main.rkt")

(define (scale-lst k lst)
  (for/list ([pt lst])
            (xyz (* (cx pt) k)
                 (* (cy pt) k)
                 (* (cz pt) k))))

(define (scale-lsst k lsst)
  (for/list ([lst lsst])
            (scale-lst k lst)))

(define (shift-point off-x off-y off-z pt)
  (+xyz pt off-x off-y off-z))

(define (shift-lst off-x off-y off-z lst)
  (for/list ([el lst])
       (shift-point off-x off-y off-z el)))

(define (shift-lsst off-x off-y off-z lsst)
  (for/list ([lst lsst])
       (shift-lst off-x off-y off-z lst)))

(define (rotate-lst-z lst rad)
  (for/list ([pt lst])
            (rotate-p-z pt rad)))
(define (rotate-lsst-z lsst rad)
  (for/list ([lst lsst])
            (rotate-lst-z lst rad)))

(define simple-beam-profile-points
  (scale-lst 0.001 (shift-lst -120 -37.5 0 (list (xy 0 34.75)
                                                 (xy 180 0)
                                                 (xy 240 0)
                                                 (xy 240 75)
                                                 (xy 0 75)
                                                 (xy 0 34.75)))))
(define simple-slim-beam-profile-points
  (scale-lst (/ 0.001 2) (shift-lst -240 -37.5 0 (list (xy 0 34.75)
                                                 (xy 180 0)
                                                 (xy 300 0)
                                                 (xy 480 34.75)
                                                 (xy 480 75)
                                                 (xy 0 75)
                                                 (xy 0 34.75)))))

(define simple-wide-beam-profile-points
  (scale-lst 0.001 (shift-lst -240 -37.5 0 (list (xy 0 34.75)
                                                 (xy 180 0)
                                                 (xy 300 0)
                                                 (xy 480 34.75)
                                                 (xy 480 75)
                                                 (xy 0 75)
                                                 (xy 0 34.75)))))

(define complex-beam-profile-points
  (scale-lst 0.001 (shift-lst -120 -37.5 0 (append (list (xy 0 34.75)
                                                         (xy 180 0)
                                                         (xy 240 0)
                                                         (xy 240 2.75)
                                                         (xy 238.625 4.125)
                                                         (xy 238.625 5.5)
                                                         (xy 235.875 5.5)
                                                         (xy 235.875 4.125)
                                                         (xy 238.625 2.75)
                                                         (xy 233.125 2.75)
                                                         (xy 233.125 11)
                                                         (xy 230.125 11)
                                                         (xy 230.125 8.25)
                                                         (xy 231.625 8.25)
                                                         (xy 231.625 2.75)
                                                         (xy 180 2.75)
                                                         (xy 180 8.25)
                                                         (xy 182.75 8.25)
                                                         (xy 182.75 11)
                                                         (xy 180 11)
                                                         (xy 180 66.75)
                                                         (xy 235.875 66.75)
                                                         (xy 233.125 64)
                                                         (xy 233.125 61.25)
                                                         (xy 235.875 61.25)
                                                         (xy 235.875 64)
                                                         (xy 238.625 66.75)
                                                         (xy 238.625 75)
                                                         (xy 234.5 75)
                                                         (xy 235.875 73.625)
                                                         (xy 237.25 73.625)
                                                         (xy 237.25 72.25)
                                                         (xy 0 72.25)
                                                         (xy 0 70.875)
                                                         (xy 4.125 70.875)
                                                         (xy 4.125 69.5)
                                                         (xy 0 69.5)
                                                         (xy 0 68.125)
                                                         (xy 4.125 68.125)
                                                         (xy 4.125 65.375)
                                                         (xy 0 65.375)
                                                         (xy 0 34.75)))))
             ;y = -0.211x + 38.08
(define beam-profile-hole-1 
  (scale-lst 0.001 (shift-lst -120 -37.5 0 (list (xy 2.75 37.5)
                                                 (xy 58.625 26.185)
                                                 (xy 58.625 59.875)
                                                 (xy 55.875 59.875)
                                                 (xy 55.875 61.25)
                                                 (xy 58.625 61.25)
                                                 (xy 58.625 62.625)
                                                 (xy 55.875 62.625)
                                                 (xy 55.875 64)
                                                 (xy 58.625 64)
                                                 (xy 58.625 70.875)
                                                 (xy 6.875 70.875)
                                                 (xy 6.875 60.375)
                                                 (xy 8.25 60.375)
                                                 (xy 8.25 59)
                                                 (xy 4.125 59)
                                                 (xy 4.125 60.275)
                                                 (xy 2.75 60.275)
                                                 (xy 2.75 37.5)))))

;y = -0.211x + 38.08
(define beam-profile-hole-2
  (scale-lst 0.001 (shift-lst -120 -37.5 0 (list (xy 61.375 25.129875)
                                                 (xy 117.25 14.340249)
                                                 (xy 117.25 70.875)
                                                 (xy 61.375 70.875)
                                                 (xy 61.375 25.129875)))))

(define beam-profile-hole-3
  (scale-lst 0.001 (shift-lst -120 -37.5 0 (list (xy 120 13.75998)
                                                 (xy 177.25 2.68025)
                                                 (xy 177.25 70.875)
                                                 (xy 120 70.875)
                                                 (xy 120 12.75998)))))

(define simple-beam-profile-hole-1
  (scale-lst 0.001 (shift-lst -240 -37.5 0(list (xy 2.75 37.5)
                                                (xy 58.625 26.185)
                                                (xy 58.625 72.25)
                                                (xy 2.75 72.25)
                                                (xy 2.75 37.5)))))
(define simple-beam-profile-hole-2
  (scale-lst 0.001 (shift-lst -240 -37.5 0(list (xy 61.375 26.185)
                                                (xy 61.375 72.25)
                                                (xy 120 72.25)
                                                (xy 120 14.34)
                                                (xy 61.375 26.185)))))
(define simple-beam-profile-hole-3
  (scale-lst 0.001 (shift-lst -240 -37.5 0(list (xy 122.75 14.34)
                                                (xy 177.25 2.75)
                                                (xy 177.25 72.25)
                                                (xy 122.75 72.25)
                                                (xy 122.75 14.34)))))
(define simple-beam-profile-hole-4
  (scale-lst 0.001 (shift-lst -240 -37.5 0(list (xy 2.75 37.5)
                                                (xy 58.625 26.185)
                                                (xy 58.625 72.25)
                                                (xy 2.75 72.25)
                                                (xy 2.75 37.5)))))
(define simple-beam-profile-hole-5
  (scale-lst 0.001 (shift-lst -240 -37.5 0(list (xy 2.75 37.5)
                                                (xy 58.625 26.185)
                                                (xy 58.625 72.25)
                                                (xy 2.75 72.25)
                                                (xy 2.75 37.5)))))
(define (lod-1-profile)
  (profile "Abu-Dhabi-Beam-LOD-1" simple-wide-beam-profile-points #:material "Plaster - Gypsum"))

(define (lod-2-profile)
  (profile "Abu-Dhabi-Beam-LOD-2" simple-wide-beam-profile-points)
  (add-hole-profile "Abu-Dhabi-Beam-LOD-2" simple-beam-profile-hole-1)
  (add-hole-profile "Abu-Dhabi-Beam-LOD-2" simple-beam-profile-hole-2)
  (add-hole-profile "Abu-Dhabi-Beam-LOD-2" simple-beam-profile-hole-3))

(define (lod-3-profile)
  (profile "Abu-Dhabi-Beam-LOD-3" complex-beam-profile-points)
  (add-hole-profile "Abu-Dhabi-Beam-LOD-3" beam-profile-hole-1)
  (add-hole-profile "Abu-Dhabi-Beam-LOD-3" beam-profile-hole-2)
  (add-hole-profile "Abu-Dhabi-Beam-LOD-3" beam-profile-hole-3))

(define (beams-from-lst lst [layer (default-layer)] #:width [w 0.15] #:height [h 0.15])
  (for/list ([pt0 lst]
        [pt1 (cdr lst)])
            (let ((b (beam pt0 pt1 #:beam-width w #:beam-height h)))
              (shape-layer layer b)
              b)))
(define (beams-from-lsst lsst [layer (default-layer)] #:width [w 0.15] #:height [h 0.15])
  (flatten (for/list ([lst lsst])
                     (beams-from-lst lst layer #:width w #:height h))))

(define (walls-from-lst lst [layer (default-layer)] #:t [t 0.3] #:off-set [off-set 0] #:height [height null])
  (for/list ([pt0 lst]
        [pt1 (cdr lst)])
            (let ((b (wall (list pt0 pt1) #:thickness t #:bottom-offset off-set #:layer layer #:height height)))
              (shape-layer layer b)
              b)))
(define (walls-from-lsst lsst [layer (default-layer)] #:t [t 0.3] #:off-set [off-set 0] #:height [height null])
  (flatten (for/list ([lst lsst])
                     (walls-from-lst lst layer #:t t #:off-set off-set))))

(define circular-pattern (list (xy 0 3)(xy -2 2)(xy -3 0)(xy -2 -2)(xy 0 -3)(xy 2 -2)(xy 3 0)(xy 2 2)))

(define star-pattern
  #;(list (list (xy 0 3)(xy 2 2)(xy 2 4)(xy 0 3))
          (list (xy 2 2)(xy 3 0)(xy 4 2))
          (list (xy 2 4)(xy 3 6)(xy 4 4))
          (list (xy 4 2)(xy 6 3)(xy 4 4))
          (list (xy 2 2)(xy 4 2)(xy 4 4)(xy 2 4)))
  (shift-lsst -3 -3 0 (list (list (xy 0 3)(xy 2 2)(xy 2 4)(xy 0 3))
                            (list (xy 2 2)(xy 3 0)(xy 4 2))
                            (list (xy 2 4)(xy 3 6)(xy 4 4))
                            (list (xy 4 2)(xy 6 3)(xy 4 4))
                            (list (xy 2 2)(xy 4 2)(xy 4 4)(xy 2 4)))))
(define star-pattern-2
  (let ((square (list (xy (- (/ 3 2)) 3)
                      (xy -3 (- (/ 3 2)))
                      (xy (/ 3 2) -3)
                      (xy 3 (/ 3 2))
                      (xy (- (/ 3 2)) 3))))
    (list square (rotate-lst-z square pi/6))))

(define (hexagonal-pattern r)
  (let ((pts (polygon-points (u0) 6 r)))
    (append pts (list (car pts)))))

(define (uvs n)
  (let ((ext (* 6 n))
        (step 6))
    (for/list ([u (division (- ext) ext (/ (* ext 2) step))])
              (for/list ([v (division (- ext) ext (/ (* ext 2) step))])
                        (xy u v)))))

(define (flat-grid)
  (for ([lst (uvs 1)])
       (for ([uv lst])
            (group (append (beams-from-lst (shift-lst (cx uv) (cy uv) 0 circular-pattern))
                           (beams-from-lsst (shift-lsst (cx uv) (cy uv) 0 star-pattern)))))))

(define (set-to-sphere pt [angle 0.013] [radius 165])
  (let* ([u-rad (* (cx pt) angle)]
         [v-rad (* (cy pt) angle)]
         [x-aux (/ (sin u-rad)(cos u-rad))]
         [y-aux (/ (sin v-rad)(cos v-rad))]
         [f (/ radius (sqrt (+ (* x-aux x-aux)(* y-aux y-aux) 1)))])
    (xyz (* x-aux f)(* y-aux f) f)))

(define (set-to-sphere-lst lst [angle 0.013] [radius 165])
  (for/list ([pt lst])
            (set-to-sphere pt angle radius)))

(define (set-to-sphere-lsst lsst [angle 0.013] [radius 165])
  (for/list ([lst lsst])
            (set-to-sphere-lst lst angle radius)))

(define (set-to-ellipsoid pt [angle 0.013] [radius 165] [a 1.2] [b 1.5] [c 1.7])
  (let* ([u-rad (* (cx pt) angle)]
         [v-rad (* (cy pt) angle)]
         [x-aux (/ (sin u-rad)(cos u-rad))]
         [y-aux (/ (sin v-rad)(cos v-rad))]
         [f (/ radius (sqrt (+ (/ (* x-aux x-aux) (* a a))
                               (/ (* y-aux y-aux) (* b b))
                               (* c c))))])
    (xyz (* x-aux f)(* y-aux f) f)))

(define (set-to-ellipsoid-lst lst [angle 0.013] [radius 165] [a 1.2] [b 1.5] [c 1.7])
  (for/list ([pt lst])
            (set-to-ellipsoid pt angle radius a b c)))

(define (set-to-ellipsoid-lsst lsst [angle 0.013] [radius 165] [a 1.2] [b 1.5] [c 1.7])
  (for/list ([lst lsst])
            (set-to-ellipsoid-lst lst angle radius a b c)))

(define profiles-ready? #f)
(define (prepare-profiles)
  (when (not profiles-ready?)
    (lod-1-profile)
    (lod-2-profile)
    (lod-3-profile)
    (set! profiles-ready? #t)))

(define (sphere-grid [n 1] [angle 0.013] [rad 0] #:layer [layer "ArchiCAD Layer"] #:radius [radius 165] #:height-level [height-level 0] #:lod [lod 1])
  (create-layer layer)
  (hide-layer layer)
  ;(show-layer layer)
  (prepare-profiles)
  (define before-sec (current-seconds))
  (for ([lst (uvs n)])
       (for ([uv lst])
            (parameterize ((default-beam-profile (cond [(eq? lod 0) ""]
                                                       [(eq? lod 1) "Abu-Dhabi-Beam-LOD-1"]
                                                       [(eq? lod 2) "Abu-Dhabi-Beam-LOD-2"]
                                                       [(eq? lod 3) "Abu-Dhabi-Beam-LOD-3"])))
              (beams-from-lsst (rotate-lsst-z (shift-lsst 0 0 (* height-level 0.075)  (set-to-sphere-lsst (shift-lsst (cx uv) (cy uv) 0 star-pattern) angle radius)) rad)
                               layer
                               #:width (if (eq? lod 0) 0.480 0.15)
                               #:height (if (eq? lod 0) 0.075 0.15)))))
  (displayln (- (current-seconds) before-sec)))

(define (ellipsoid-grid [n 1] [angle 0.013] [rad 0] #:layer [layer "ArchiCAD Layer"] #:radius [radius 165] #:height-level [height-level 0] #:lod [lod 1] #:a [a 1] #:b [b 1] #:c [c 1])
  (create-layer layer)
  (hide-layer layer)
  ;(show-layer layer)
  (prepare-profiles)
  (define before-sec (current-seconds))
  (for ([lst (uvs n)])
       (for ([uv lst])
            (parameterize ((default-beam-profile (cond [(eq? lod 0) ""]
                                                       [(eq? lod 1) "Abu-Dhabi-Beam-LOD-1"]
                                                       [(eq? lod 2) "Abu-Dhabi-Beam-LOD-2"]
                                                       [(eq? lod 3) "Abu-Dhabi-Beam-LOD-3"])))
              (beams-from-lsst (rotate-lsst-z (shift-lsst 0 0 (* height-level 0.075)  (set-to-ellipsoid-lsst (shift-lsst (cx uv) (cy uv) 0 star-pattern) angle radius a b c)) rad)
                               layer
                               #:width (if (eq? lod 0) 0.15 #;0.480 0.15)
                               #:height (if (eq? lod 0) 0.15 #;0.075 0.15)))))
  (displayln (- (current-seconds) before-sec)))

(define (structural-walls circle-radius circle-division el/line #:el-radius [el-radius 3] #:off-set [off-set 130] #:layer [layer "Structural Walls"] #:height [height null])
  (create-layer layer)
  ;(hide-layer layer)
  ;(show-layer layer)
  (define before-sec (current-seconds))
  (define upper-shell (rev-shell (list (xy 165 0)(xy -165 0)(xy 165 0)) (list pi) #:revolution-angle pi #:material "Glass" ))
  (define lower-shell (rev-shell (list (xy 165 0)(xy -165 0)(xy 165 0)) (list pi) #:revolution-angle pi #:material "Glass" #:height -10))
  (create-layer "shells")
  (shape-layer "shells" upper-shell)
  (shape-layer "shells" lower-shell)
  (define walls-to-trim (list))
  (for/list ([psi (division 0 2pi circle-division #f)])
       (for/list ([ro (division 0 circle-radius el/line #f)])
                 (let ([pt (pol ro psi)])
                   (set! walls-to-trim (append walls-to-trim
                                               (walls-from-lst
                                                (shift-lst (cx pt) (cy pt) 0 (rotate-lst-z (hexagonal-pattern  el-radius) psi))
                                                layer
                                                #:off-set off-set
                                                #:height height)))
                   #;(trim-elements (walls-from-lst
                                   (shift-lst (cx pt) (cy pt) 0 (rotate-lst-z (hexagonal-pattern  el-radius) psi))
                                   layer
                                   #:off-set off-set
                                   #:height height)
                                  shell-to-trim))))
  (trim-elements walls-to-trim (list upper-shell lower-shell))
  (displayln (- (current-seconds) before-sec)))

;(send (structural-walls 60 10 4 #:el-radius 30 #:height 1000))

#|
TODO:
  Beam Profile;
  Change the Z of each layer;
  Eliminate repeated beams due to the repeated element being complete. Have different elements that are applied.

|#
#|
(send (sphere-grid 8 0.014)(sphere-grid 8 0.007)
      (sphere-grid 8 0.014 0.226893)(sphere-grid 8 0.007 pi/4))

(send (delete-all-elements)(sphere-grid #:beam-profile "Abu-Dhabi-Beam"))

(send (delete-all-elements)
      (sphere-grid 12 (/ 2pi 360) 0          #:height-level 3 #:radius 165 #:layer "Abi Dhabi 4" #:lod 1)
      (sphere-grid 24 (/ (/ 2pi 360) 2) 0    #:height-level 2 #:radius 165 #:layer "Abi Dhabi 3" #:lod 1)
      (sphere-grid 12 (/ 2pi 360) 0.226893   #:height-level 1 #:radius 165 #:layer "Abi Dhabi 2" #:lod 1)
      (sphere-grid 24 (/ (/ 2pi 360) 2) pi/4 #:height-level 0 #:radius 165 #:layer "Abi Dhabi 1" #:lod 1))

;
(send (delete-all-elements)
      (define small-n 12)
      (define large-n 24)
      ;Upper
      (sphere-grid small-n (/ 2pi 360) 0          #:height-level 3 #:radius 165 #:layer "Abi Dhabi 4" #:lod 1)
      (sphere-grid large-n (/ (/ 2pi 360) 2) 0    #:height-level 2 #:radius 165 #:layer "Abi Dhabi 3" #:lod 1)
      (sphere-grid small-n (/ 2pi 360) 0.226893   #:height-level 1 #:radius 165 #:layer "Abi Dhabi 2" #:lod 1)
      (sphere-grid large-n (/ (/ 2pi 360) 2) pi/4 #:height-level 0 #:radius 165 #:layer "Abi Dhabi 1" #:lod 1)
      ;Lower
      (sphere-grid large-n (/ (/ 2pi 360) 2) (+ pi/4 pi/4) #:height-level -1 #:radius 165 #:layer "Abi Dhabi 5" #:lod 1)
      (sphere-grid small-n (/ 2pi 360) (+ pi/4 0.226893)   #:height-level -2 #:radius 165 #:layer "Abi Dhabi 6" #:lod 1)
      (sphere-grid large-n (/ (/ 2pi 360) 2) pi/4          #:height-level -3 #:radius 165 #:layer "Abi Dhabi 7" #:lod 1)
      (sphere-grid small-n (/ 2pi 360) pi/4                #:height-level -4 #:radius 165 #:layer "Abi Dhabi 8" #:lod 1))

;Testing
(send (delete-all-elements)
      (sphere-grid 1 (/ 2pi 360) 0          #:height-level 3 #:radius 165 #:layer "Abi Dhabi 4" #:lod 1)
      (sphere-grid 2 (/ (/ 2pi 360) 2) 0    #:height-level 2 #:radius 165 #:layer "Abi Dhabi 3" #:lod 1)
      (sphere-grid 1 (/ 2pi 360) 0.226893   #:height-level 1 #:radius 165 #:layer "Abi Dhabi 2" #:lod 1)
      (sphere-grid 2 (/ (/ 2pi 360) 2) pi/4 #:height-level 0 #:radius 165 #:layer "Abi Dhabi 1" #:lod 1)
      (sphere-grid 2 (/ (/ 2pi 360) 2) (+ pi/4 pi/4) #:height-level -1 #:radius 165 #:layer "Abi Dhabi -1" #:lod 1)
      (sphere-grid 1 (/ 2pi 360) (+ pi/4 0.226893)   #:height-level -2 #:radius 165 #:layer "Abi Dhabi -2" #:lod 1)
      (sphere-grid 2 (/ (/ 2pi 360) 2) pi/4          #:height-level -3 #:radius 165 #:layer "Abi Dhabi -3" #:lod 1)
      (sphere-grid 1 (/ 2pi 360) pi/4                #:height-level -4 #:radius 165 #:layer "Abi Dhabi -4" #:lod 1))


(send (delete-all-elements)
      (define base-angle 2pi)
      (sphere-grid 1 (/ base-angle 360) 0 #:height-level 3 #:radius 165 #:layer "Abi Dhabi 4" #:lod 1)
      (sphere-grid 2 (/ (/ base-angle 360) 2) 0 #:height-level 2 #:radius 165 #:layer "Abi Dhabi 3" #:lod 1)
      (sphere-grid 2 (/ (/ base-angle 360) 2) 0.226893 #:height-level 2 #:radius 165 #:layer "Abi Dhabi 2" #:lod 1)
      (sphere-grid 4 (/ (/ base-angle 360) 4) pi/4 #:height-level 1 #:radius 165 #:layer "Abi Dhabi 1" #:lod 1))

(send (delete-all-elements)
      (define base-angle 2pi)
      ;Upper
      (sphere-grid 6  (/ base-angle 360)       0        #:height-level 4  #:radius 165 #:layer "Abi Dhabi 4" #:lod 1)
      (sphere-grid 12 (/ (/ base-angle 360) 2) 0        #:height-level 3  #:radius 165 #:layer "Abi Dhabi 3" #:lod 1)
      (sphere-grid 12 (/ (/ base-angle 360) 2) 0.226893 #:height-level 2  #:radius 165 #:layer "Abi Dhabi 2" #:lod 1)
      (sphere-grid 24 (/ (/ base-angle 360) 4) pi/4     #:height-level 1  #:radius 165 #:layer "Abi Dhabi 1" #:lod 1)
      ;Lower
      (sphere-grid 6  (/ base-angle 360)       0.226893              #:height-level -1 #:radius 165 #:layer "Abi Dhabi 5" #:lod 1)
      (sphere-grid 12 (/ (/ base-angle 360) 2) 0.226893              #:height-level -2 #:radius 165 #:layer "Abi Dhabi 6" #:lod 1)
      (sphere-grid 12 (/ (/ base-angle 360) 2) (+ 0.226893 0.226893) #:height-level -3 #:radius 165 #:layer "Abi Dhabi 7" #:lod 1)
      (sphere-grid 24 (/ (/ base-angle 360) 4) (+ pi/4 0.226893)     #:height-level -4 #:radius 165 #:layer "Abi Dhabi 8" #:lod 1))

(send (delete-all-elements)
      (define base-angle 2pi)
      ;Upper
      (sphere-grid 4  (/ base-angle 360)       0        #:height-level 4  #:radius 165 #:layer "Abi Dhabi 4" #:lod 1)
      (sphere-grid 7 (/ (/ base-angle 360) (/ 7 4)) 0.226893        #:height-level 3  #:radius 165 #:layer "Abi Dhabi 3" #:lod 1)
      (sphere-grid 10 (/ (/ base-angle 360) (/ 10 4)) pi/4 #:height-level 2  #:radius 165 #:layer "Abi Dhabi 2" #:lod 1))


(send (delete-all-elements)
      (define base-angle 2pi)
      ;Upper
      (sphere-grid 4  (/ base-angle 360)       0        #:height-level 4  #:radius 165 #:layer "Abi Dhabi 4" #:lod 1)
      (sphere-grid 7 (/ (/ base-angle 360) (/ 7 4)) 0        #:height-level 3  #:radius 165 #:layer "Abi Dhabi 3" #:lod 1)
      (sphere-grid 7 (/ (/ base-angle 360) (/ 7 4)) 0.226893        #:height-level 2  #:radius 165 #:layer "Abi Dhabi 2" #:lod 1)
      (sphere-grid 10 (/ (/ base-angle 360) (/ 10 4)) pi/4 #:height-level 1  #:radius 165 #:layer "Abi Dhabi 1" #:lod 1)
      ;Lower
      (sphere-grid 4  (/ base-angle 360)       0.226893        #:height-level -1  #:radius 165 #:layer "Abi Dhabi 5" #:lod 1)
      (sphere-grid 7 (/ (/ base-angle 360) (/ 7 4)) 0.226893        #:height-level -2  #:radius 165 #:layer "Abi Dhabi 6" #:lod 1)
      (sphere-grid 7 (/ (/ base-angle 360) (/ 7 4)) (+ 0.226893 0.226893)        #:height-level -3  #:radius 165 #:layer "Abi Dhabi 7" #:lod 1)
      (sphere-grid 10 (/ (/ base-angle 360) (/ 10 4)) (+ pi/4 0.226893) #:height-level -4  #:radius 165 #:layer "Abi Dhabi 8" #:lod 1)
)

|#


#|
ELLIPSOID
(send (delete-all-elements)
        (define base-angle 2pi)
        (ellipsoid-grid 20 (/ (/ base-angle 360) 6) 0 #:height-level 4 #:radius 165 #:layer "Abi Dhabi 20" #:lod 0 #:a 1 #:b 1 #:c 0.3)
        (ellipsoid-grid 25 (/ (/ base-angle 360) (* 6 (/ 25 20))) 0 #:height-level 4 #:radius 165 #:layer "Abi Dhabi 25" #:lod 0 #:a 1 #:b 1 #:c 0.3)
        (ellipsoid-grid 30 (/ (/ base-angle 360) (* 6 (/ 30 20))) 0 #:height-level 4 #:radius 165 #:layer "Abi Dhabi 35" #:lod 0 #:a 1 #:b 1 #:c 0.3)
        (ellipsoid-grid 40 (/ (/ base-angle 360) (* 6 (/ 40 20))) 0 #:height-level 4 #:radius 165 #:layer "Abi Dhabi 40" #:lod 0 #:a 1 #:b 1 #:c 0.3))

send (delete-all-elements)
        (define base-angle 2pi)
        (define div 20)
        (ellipsoid-grid 20 (/ (/ base-angle 360) div) 0 #:height-level 4 #:radius 165 #:layer "Abi Dhabi 20" #:lod 0 #:a 1 #:b 1 #:c 0.3)
        (ellipsoid-grid 25 (/ (/ base-angle 360) (* div (/ 25 20))) 0 #:height-level 4 #:radius 165 #:layer "Abi Dhabi 25" #:lod 0 #:a 1 #:b 1 #:c 0.3)
        (ellipsoid-grid 30 (/ (/ base-angle 360) (* div (/ 30 20))) 0 #:height-level 4 #:radius 165 #:layer "Abi Dhabi 35" #:lod 0 #:a 1 #:b 1 #:c 0.3)
        (ellipsoid-grid 40 (/ (/ base-angle 360) (* div (/ 40 20))) 0 #:height-level 4 #:radius 165 #:layer "Abi Dhabi 40" #:lod 0 #:a 1 #:b 1 #:c 0.3))

(send (delete-all-elements)
        (define base-angle 2pi)
        (define div 2)
        (ellipsoid-grid 20 (/ (/ base-angle 360) div) 0 #:height-level 4 #:radius 165 #:layer "Abi Dhabi 20" #:lod 0 #:a 1 #:b 1 #:c 0.3)
        (ellipsoid-grid 25 (/ (/ base-angle 360) (* div (/ 25 20))) 0 #:height-level 4 #:radius 165 #:layer "Abi Dhabi 25" #:lod 0 #:a 1 #:b 1 #:c 0.3)
        (ellipsoid-grid 30 (/ (/ base-angle 360) (* div (/ 30 20))) 0 #:height-level 4 #:radius 165 #:layer "Abi Dhabi 30" #:lod 0 #:a 1 #:b 1 #:c 0.3)
        (ellipsoid-grid 35 (/ (/ base-angle 360) (* div (/ 35 20))) 0 #:height-level 4 #:radius 165 #:layer "Abi Dhabi 35" #:lod 0 #:a 1 #:b 1 #:c 0.3)
        (ellipsoid-grid 40 (/ (/ base-angle 360) (* div (/ 40 20))) 0 #:height-level 4 #:radius 165 #:layer "Abi Dhabi 40" #:lod 0 #:a 1 #:b 1 #:c 0.3))
|#

(define (sphere-grids n angle)
  (sphere-grid n angle 0 #:radius 165 #:layer (string-append (string-append "Abi Dhabi " (~a n)) " 0") #:lod 1)
  (sphere-grid n angle 0.226893  #:radius 165 #:layer (string-append (string-append "Abi Dhabi " (~a n)) " 13") #:lod 1)
  (sphere-grid n angle pi/4 #:radius 165 #:layer (string-append (string-append "Abi Dhabi " (~a n)) " 45") #:lod 1)
  )

#|


(send (delete-all-elements)
      (define base-angle 2pi)
      (sphere-grids 1 (/ base-angle 360))
      (sphere-grids 2 (/ (/ base-angle 360) (/ 2 1)))
)
|#



