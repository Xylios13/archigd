#lang racket
(require "main.rkt")

;(define landing-width 2.5)

(define (left-corner-points pt width length)
  (list pt (+x pt width) (+xy pt width length) (+y pt length)))

(define (dom-ino-shape pt width length [landing-width 2.5])
  (define sub-div (/ length 3))
  (list pt
        (+x pt width)
        (+xy pt width (- sub-div 1.6))
        (+xy pt (+ width landing-width) (- sub-div 1.6))
        (+xy pt (+ width landing-width) sub-div)
        (+xy pt width sub-div)
        (+xy pt width length)
        (+y pt length)))

(define (floors lower-left-point width length levels [landing-width 2.5])
  (let* ([step-size (/ landing-width 2)]
         [hole-width (+ step-size 0)]
         [hole-pt0 (+xy lower-left-point (- width 3) (/ length 4))]
         [sub-div (/ length 3)])
  (for ([lvl levels])
       (parameterize ([current-level lvl])
         (define slab-id (slab (left-corner-points lower-left-point width length #;landing-width)
                               #:type-of-material "Basic"
                               #:material "GENERIC - PREFABRICATED"))
         (for ([i 3])
              (when (eq? lvl (first levels))
                (beam (+xyz lower-left-point (+ (* i (/ (- width 0.3) 2)) 0.15) (- (+ (* sub-div 2) 1.6) 0.15) -0.3)
                      (+xyz lower-left-point (+ (* i (/ (- width 0.3) 2)) 0.15) (+ (+ (* sub-div 2) 1.6) 0.15) -0.3)
                      #:beam-width 0.3
                      #:beam-height 0.3)
                (beam (+xyz lower-left-point (+ (* i (/ (- width 0.3) 2)) 0.15) (- sub-div 1.6 0.15) -0.3)
                      (+xyz lower-left-point (+ (* i (/ (- width 0.3) 2)) 0.15) (- (+ sub-div 0.15) 1.6) -0.3)
                      #:beam-width 0.3
                      #:beam-height 0.3))
              (unless (eq? lvl (last levels))
                (column (+xy lower-left-point (+ (* i (/ (- width 0.15) 2)) 0.075)  (+ (* sub-div 2) 1.6)))
                (column (+xy lower-left-point (+ (* i (/ (- width 0.15) 2)) 0.075)  (- sub-div 1.6)))))
         (stairs "Concrete Landing 18"
                   (+xy lower-left-point (+ width landing-width)  sub-div)
                   #:angle pi
                   #:additional-parameters (list (list "gs_shape_landing_m" 1)
                                                 (list "flwl" landing-width)
                                                 (list "matt" "Paint - Light Gray")
                                                 (list "mati" "Paint - Light Gray")
                                                 (list "matbot" "Paint - Light Gray")
                                                 (list "sst" 0.3)))
         (unless (eq? lvl (last levels))
           (stairs "Stair Straight 18"
                   (+xy lower-left-point (+ width landing-width)  sub-div)
                   #:angle pi/2
                   #:height (/ (default-level-to-level-height) 2)
                   #:use-xy-fix-size #t
                   #:x-ratio sub-div
                   #:y-ratio step-size
                   #:additional-parameters (list (list "rail_m" 4)))
           (stairs "Concrete Landing 18"
                   (+xy lower-left-point width  (* sub-div 2))
                   #:angle 0
                   #:bottom-offset (/ (default-level-to-level-height) 2)
                   #:additional-parameters (list (list "gs_shape_landing_m" 1)
                                                 (list "flwl" landing-width)
                                                 (list "matt" "Paint - Light Gray")
                                                 (list "mati" "Paint - Light Gray")
                                                 (list "matbot" "Paint - Light Gray")
                                                 (list "sst" 0.3)))
           (stairs "Stair Straight 18"
                   (+xy lower-left-point width  (* sub-div 2))
                   #:angle 3pi/2
                   #:use-xy-fix-size #t
                   #:x-ratio sub-div
                   #:y-ratio step-size
                   #:bottom-offset (/ (default-level-to-level-height) 2)
                   #:additional-parameters (list (list "rail_m" 4))))))))

(define (building-walls points levels)
  (let* ((p1 (car points))
         (p2 (car (cdr points)))
         (wall-length (sqrt (+ (* (- (cx p2) (cx p1))
                                  (- (cx p2) (cx p1)))
                               (* (- (cy p2) (cy p1))
                                  (- (cy p2) (cy p1)))))))
    (for ([lvl levels])
         (displayln (wall (cons (last points) points) lvl))
         #;(for ([wall-index (walls (cons (last points) points) lvl)])
              (door wall-index (/ wall-length 2))))))

(define (building center-point width length n-floors [landing-width 2.5])
  (let ((lower-left-point (+xy center-point (- (/ width 2)) (- (/ length 2))))
        (levels (for/list ([i n-floors])
                          (level (* i (default-level-to-level-height))))))
    (floors lower-left-point width length levels landing-width)
    #;(roof (left-corner-points lower-left-point width length #;landing-width)
          #:bottom-level (upper-level #:level (last levels))
          #:type-of-material "Basic"
          #:material "GENERIC - PREFABRICATED")
    #;(stairs "Concrete Landing 18"
                   (+xy lower-left-point (+ width landing-width)  (/ length 3))
                   #:bottom-level (upper-level #:level (last levels))
                   #:angle pi
                   #:additional-parameters (list (list "gs_shape_landing_m" 1)
                                                 (list "flwl" landing-width)
                                                 (list "matt" "Paint - Light Gray")
                                                 (list "mati" "Paint - Light Gray")
                                                 (list "matbot" "Paint - Light Gray")))
         ))

;(send (delete-levels)(building (u0) 32 16 2))
