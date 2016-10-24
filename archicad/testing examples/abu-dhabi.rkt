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

(define simple-beam-profile-points
  (scale-lst 0.001 (shift-lst -120 -37.5 0 (list (xy 0 34.75)
                                                 (xy 180 0)
                                                 (xy 240 0)
                                                 (xy 240 75)
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
  (scale-lst 0.001 (shift-lst -120 -37.5 0 (list (xy 0 34.75)
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
  (profile "Abu-Dhabi-Beam-LOD-1" simple-wide-beam-profile-points))

(define (lod-2-profile)
  (profile "Abu-Dhabi-Beam-LOD-2" simple-wide-beam-profile-points)
  (add-hole-profile "Abu-Dhabi-Beam-LOD-2" simple-beam-profile-hole-1)
  (add-hole-profile "Abu-Dhabi-Beam-LOD-2" simple-beam-profile-hole-2)
  (add-hole-profile "Abu-Dhabi-Beam-LOD-2" simple-beam-profile-hole-3))

(define (lod-3-profile)
  (profile "Abu-Dhabi-Beam-LOD-3" complex-beam-profile-points)
  (add-hole-profile "Abu-Dhabi-Beam-LOD-3" beam-profile-hole-1))

(define (beams-from-lst lst [layer "ArchiCAD Layer"] #:width [w 0.15] #:height [h 0.15])
  (for/list ([pt0 lst]
        [pt1 (cdr lst)])
            (let ((b (beam pt0 pt1 #:beam-width w #:beam-height h)))
              (shape-layer layer b)
              b)))
(define (beams-from-lsst lsst [layer "ArchiCAD Layer"] #:width [w 0.15] #:height [h 0.15])
  (flatten (for/list ([lst lsst])
                     (beams-from-lst lst layer #:width w #:height h))))

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

(define element-pattern
  (list (list (xy 0 3)(xy -2 2)(xy -3 0)(xy -2 -2)(xy 0 -3)(xy 2 -2)(xy 3 0)(xy 2 2))
        (list (xy 0 3)(xy 2 2)(xy 2 4)(xy 0 3))
        (list (xy 3 0)(xy 4 2))
        (list (xy 2 4)(xy 3 6)(xy 4 4))
        (list (xy 4 2)(xy 6 3)(xy 4 4))
        (list (xy 2 2)(xy 4 2)(xy 4 4)(xy 2 4))))
(define up-element-pattern
  (list (list (xy 0 3)(xy -2 2)(xy -3 0)(xy -2 -2)(xy 0 -3))
        (list (xy 0 3)(xy 2 2)(xy 2 4)(xy 0 3))
        (list (xy 2 2)(xy 3 0)(xy 4 2))
        (list (xy 2 4)(xy 3 6)(xy 4 4))
        (list (xy 4 2)(xy 6 3)(xy 4 4))
        (list (xy 2 2)(xy 4 2)(xy 4 4)(xy 2 4))))
(define left-element-pattern
  (list (list (xy -3 0)(xy -2 -2)(xy 0 -3)(xy 2 -2)(xy 3 0)(xy 2 2))
        (list (xy 0 3)(xy 2 2)(xy 2 4)(xy 0 3))
        (list (xy 3 0)(xy 4 2))
        (list (xy 2 4)(xy 3 6)(xy 4 4))
        (list (xy 4 2)(xy 6 3)(xy 4 4))
        (list (xy 2 2)(xy 4 2)(xy 4 4)(xy 2 4))))
(define middle-element-pattern
  (list (list (xy 0 3)(xy 2 2)(xy 2 4)(xy 0 3))
          (list (xy 2 2)(xy 3 0)(xy 4 2))
          (list (xy 2 4)(xy 3 6)(xy 4 4))
          (list (xy 4 2)(xy 6 3)(xy 4 4))
          (list (xy 2 2)(xy 4 2)(xy 4 4)(xy 2 4))))
  

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

(define (rotate-lst-z lst rad)
  (for/list ([pt lst])
            (rotate-p-z pt rad)))
(define (rotate-lsst-z lsst rad)
  (for/list ([lst lsst])
            (rotate-lst-z lst rad)))

(define profiles-ready? #f)
(define (prepare-profiles)
  (when (not profiles-ready?)
    (lod-1-profile)
    (lod-2-profile)
    (lod-3-profile)
    (set! profiles-ready? #t)))

(define (sphere-grid [n 1] [angle 0.013] [rad 0] #:layer [layer "ArchiCAD Layer"] #:radius [radius 165] #:height-level [height-level 0] #:lod [lod 1])
  (create-layer layer)
  (prepare-profiles)
  (define level-dif 0.5)
  (for ([lst (uvs n)]
        [i (length (uvs n))])
       (for ([uv lst]
             [j (length lst)])
            (parameterize ((default-beam-profile (cond [(eq? lod 0) ""]
                                                       [(eq? lod 1) "Abu-Dhabi-Beam-LOD-1"]
                                                       [(eq? lod 2) "Abu-Dhabi-Beam-LOD-2"]
                                                       [(eq? lod 3) "Abu-Dhabi-Beam-LOD-3"])))
              (let ((current-pattern (cond [(and (= i 0)(= j 0)) element-pattern]
                                           [(= i 0) up-element-pattern]
                                           [(and (> i 0)(= j 0)) left-element-pattern]
                                           [else middle-element-pattern])))
                (group (beams-from-lsst (rotate-lsst-z (shift-lsst 0 0 (* height-level level-dif)  (set-to-sphere-lsst (shift-lsst (cx uv) (cy uv) 0 current-pattern) angle radius)) rad)
                                        layer
                                        #:width (if (eq? lod 0) 0.480 0.15)
                                        #:height (if (eq? lod 0) 0.075 0.15))))))))


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
      (sphere-grid 12  (/ 2pi 360) 0             #:height-level 3 #:radius 165 #:layer "Abi Dhabi 4" #:lod 1)
      (sphere-grid 24 (/ (/ 2pi 360) 2) 0        #:height-level 2 #:radius 165 #:layer "Abi Dhabi 3" #:lod 1)
      (sphere-grid 24 (/ (/ 2pi 360) 2) 0.226893 #:height-level 1 #:radius 165 #:layer "Abi Dhabi 2" #:lod 1)
      (sphere-grid 48 (/ (/ 2pi 360) 4) pi/4     #:height-level 0 #:radius 165 #:layer "Abi Dhabi 1" #:lod 1))

|#





