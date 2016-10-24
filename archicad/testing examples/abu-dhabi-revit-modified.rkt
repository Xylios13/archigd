#lang racket
(require rosetta/archicad)

(define beam-family (load-beam-family "C:\\ProgramData\\Autodesk\\RVT 2015\\Libraries\\US Metric\\Structural Framing\\Wood\\M_Timber.rfa" #:width "b" #:height "d"))

(define (rotate-p-z p a)
  (xyz (- (* (cx p) (cos a))
          (* (cy p) (sin a)))
       (+ (* (cx p) (sin a))
          (* (cy p) (cos a)))
       (cz p)))

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

(define (beams-from-lst lst #:width [w 0.15] #:height [h 0.15])
  (for/list ([pt0 lst]
        [pt1 (cdr lst)])
            (parameterize ((default-beam-family (beam-family-element beam-family #:width w #:height h)))
             (beam pt0 pt1))))

(define (beams-from-lsst lsst #:width [w 0.15] #:height [h 0.15])
  (flatten (for/list ([lst lsst])
                     (beams-from-lst lst #:width w #:height h))))

(define star-pattern
  (shift-lsst -3 -3 0 (list (list (xy 0 3)(xy 2 2)(xy 2 4)(xy 0 3))
                            (list (xy 2 2)(xy 3 0)(xy 4 2))
                            (list (xy 2 4)(xy 3 6)(xy 4 4))
                            (list (xy 4 2)(xy 6 3)(xy 4 4))
                            (list (xy 2 2)(xy 4 2)(xy 4 4)(xy 2 4)))))

(define (uvs n)
  (let ((ext (* 6 n))
        (step 6))
    (for/list ([u (division (- ext) ext (/ (* ext 2) step))])
              (for/list ([v (division (- ext) ext (/ (* ext 2) step))])
                        (xy u v)))))

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

(define (sphere-grid [n 1] [angle 0.013] [rad 0] #:radius [radius 165] #:height-level [height-level 0] #:lod [lod 0])
  (define before-sec (current-seconds))
  (for ([lst (uvs n)])
       (for ([uv lst])
            (beams-from-lsst (rotate-lsst-z (shift-lsst 0
                                                        0
                                                        (* height-level 0.075)
                                                        (set-to-sphere-lsst (shift-lsst (cx uv)
                                                                                        (cy uv)
                                                                                        0
                                                                                        star-pattern)
                                                                            angle
                                                                            radius))
                                            rad)
                             #:width (if (eq? lod 0) 0.480 0.15)
                             #:height (if (eq? lod 0) 0.075 0.15))))
  (displayln (- (current-seconds) before-sec)))



(delete-all-elements)
(let ([base-angle 2pi])
  ;Upper
  (sphere-grid 4 (/ base-angle 360) 0 #:height-level 4 #:radius 165)
  (sphere-grid 7 (/ (/ base-angle 360) (/ 7 4)) 0 #:height-level 3 #:radius 165)
  (sphere-grid 7 (/ (/ base-angle 360) (/ 7 4)) 0.226893 #:height-level 2 #:radius 165)
  (sphere-grid 10 (/ (/ base-angle 360) (/ 10 4)) pi/4 #:height-level 1 #:radius 165)
  ;Lower
  (sphere-grid 4  (/ base-angle 360) 0.226893 #:height-level -1 #:radius 165)
  (sphere-grid 7 (/ (/ base-angle 360) (/ 7 4)) 0.226893 #:height-level -2 #:radius 165)
  (sphere-grid 7 (/ (/ base-angle 360) (/ 7 4)) (+ 0.226893 0.226893) #:height-level -3 #:radius 165)
  (sphere-grid 10 (/ (/ base-angle 360) (/ 10 4)) (+ pi/4 0.226893) #:height-level -4 #:radius 165))
(disconnect)







