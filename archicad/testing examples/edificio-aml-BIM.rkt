#lang racket
(require "main.rkt")

(define building-length 90600)
(define building-width 33600)
(define building-height 13500)
(define building-floors 4)
(define slab-height 500)
(define column-width 600)
(define bar-thickness 200)

#|
(define building-length 906)
(define building-width 336)
(define building-height 135)
(define building-floors 4)
(define slab-height 5)
(define column-width 6)
(define bar-thickness 2)
|#

;Floor plans
(define (floorplans p)
  (for/list ((h (division 0 building-height (- building-floors 1))))
            (box (+z p h) building-length building-width slab-height)))

#;(floorplans (xyz 0 0 0))
;columns

(define (columns p n m)
  (for/list ((x (division 0 (- building-length column-width) n)))
            (for/list ((y (division 0 (- building-width column-width) m)))
                      (box (+xy p x y) column-width column-width building-height))))

#;(columns (xyz 0 0 0) 11 6)

;Facade

(define (facade p length height n m)
  (let ((element-length (/ length n))
        (element-height (/ height m)))
    (for/list ((r (division 0 length n #f)))
              (for/list ((z (division 0 height m #f)))
                        (facade-element (+xz p r (+ z (/ element-height 2)))
                                        element-length
                                        element-height
                                        (+ 2000 (* 1000 (cos (+ (* z 2pi (/ 1 height)) (* r 4pi (/ 1 length)))))))))))

(define (facade-element p width height dist)
  (let ((e (/ height 2)))
    (let ((p0 p)
          (p1 (+xy p (/ width 2) (- dist))))
      (let ((v (unitize (p-p p1 p0))))
        (let ((pa (+c p0 (v*r v (/ e -2))))
              (pb (+c p1 (v*r v (/ e +2)))))
          (right-cuboid pa e e pb))))
    (let ((p0 (+xz p width e))
          (p1 (+xyz p (/ width 2) (- dist) e)))
      (let ((v (unitize (p-p p1 p0))))
        (let ((pa (+c p0 (v*r v (/ e -2))))
              (pb (+c p1 (v*r v (/ e +2)))))
          (right-cuboid pa e e pb))))))

(define (facades p length width height n0 n1 m)
  (facade p length height n0 m)
  (facade (loc-from-o-phi (+x p length) pi/2) width height n1 m)
  (facade (loc-from-o-phi (+xy p length width) pi) length height n0 m)
  (facade (loc-from-o-phi (+y p width) 3pi/2) width height n1 m))

#;(facades (xy 0 0) building-length building-width building-height 50 20 40)


(define (building p n m)
  (let ((levels-lst (map-division (lambda (z)
                                    (create-level #:height z))
                                  0 building-height building-floors)))
    (for ((level levels-lst))
         (parameterize ((current-level level)
                        (default-level-to-level-height (/ building-height building-floors)))
           (if (eq? level (last levels-lst))
               (roof (list p
                           (+x p building-length)
                           (+xy p building-length building-width)
                           (+y p building-width))
                     #:thickness slab-height
                     ;#:bottom-level (upper-level #:level (last levels-lst) #:height (/ building-height building-floors))
                     #:type-of-material "Basic"
                     #:material "Glass")
               (begin (for/list ((x (division 0 (- building-length column-width) n)))
                                (for/list ((y (division 0 (- building-width column-width) m)))
                                          (column (+xy p (+ x (/ column-width 2)) (+ y (/ column-width 2)))
                                                  #:circle-based? #f
                                                  #:depth column-width
                                                  #:width column-width)))
                      (slab (list p
                                  (+x p building-length)
                                  (+xy p building-length building-width)
                                  (+y p building-width))
                            #:type-of-material "Basic"
                            #:thickness slab-height)))))))

(define (facade-beam p length height n m)
  (let ((element-length (/ length n))
        (element-height (/ height m)))
    (for/list ((r (division 0 length n #f)))
              (for/list ((z (division 0 height m #f)))
                        (facade-element-beam (+xz p r (+ z (/ element-height 2)))
                                             element-length
                                             element-height
                                             (+ 2000 (* 1000 (cos (+ (* z 2pi (/ 1 height)) (* r 4pi (/ 1 length)))))))))))

(define (facade-element-beam p width height dist)
  (let ((e (/ height 2)))
    (let ((p0 p)
          (p1 (+xy p (/ width 2) (- dist))))
      (let ((v (unitize (p-p p1 p0))))
        (let ((pa (+c p0 (v*r v (/ e -2))))
              (pb (+c p1 (v*r v (/ e +2)))))
          (beam p0
                p1
                #:beam-height e
                #:beam-width e
                #:material "Timber - Floor"))))
    (let ((p0 (+xz p width e))
          (p1 (+xyz p (/ width 2) (- dist) e)))
      (let ((v (unitize (p-p p1 p0))))
        (let ((pa (+c p0 (v*r v (/ e -2))))
              (pb (+c p1 (v*r v (/ e +2)))))
          (beam p0
                p1 
                #:beam-height e
                #:beam-width e
                #:material "Timber - Floor"))))))

(define (facades-beam p length width height n0 n1 m)
  (facade-beam p length height n0 m)
  (facade-beam (loc-from-o-phi (+x p length) pi/2) width height n1 m)
  (facade-beam (loc-from-o-phi (+xy p length width) pi) length height n0 m)
  (facade-beam (loc-from-o-phi (+y p width) 3pi/2) width height n1 m))


;;(send (floorplans (xyz 0 0 0)) (columns (xyz 0 0 0) 11 6) (facades (xy 0 0) building-length building-width building-height 50 20 40))
;;(send (floorplans (xyz 0 0 0)) (columns (xyz 0 0 0) 11 6) (facades (xy 0 0) building-length building-width building-height 50 20 10))

;;(send (delete-levels) (building (xyz 0 0 0) 11 6) (facades-beam (xy 0 0) building-length building-width building-height 50 20 40))
