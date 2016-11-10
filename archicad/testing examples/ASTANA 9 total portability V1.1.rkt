#lang racket
;(require rosetta/autocad)
;(require rosetta/sketchup)
;(require rosetta/rhino)
(require rosetta/archicad)
;(require "rosetta/archicad/backend.rkt")

(delete-all-shapes)
(define p (x 0))

;PARAMETERS______________________________________________________________

;Steel frames

(define n-frames 36)
(define frame-x 32)
(define frame-y 53)
(define x-divisions 5)
(define y-divisions 8)

(define ro-f (sqrt (+ (sqr (/ frame-x 2)) (sqr (/ frame-y 2)))))
(define psiA (atan (/ frame-y 2) (/ frame-x 2))) 
(define psiB (- pi (atan (/ frame-y 2) (/ frame-x 2)))) 
(define psiC (+ pi (atan (/ frame-y 2) (/ frame-x 2)))) 
(define psiD (- (atan (/ frame-y 2) (/ frame-x 2))))

;Bays

(define ro-x1 (sqrt (+ (sqr (/ frame-y 2)) (sqr (* (/ frame-x 5) 1.5)))))
(define ro-x2 (sqrt (+ (sqr (/ frame-y 2)) (sqr (* (/ frame-x 5) 0.5)))))
(define psi-x1 (atan (/ (* 1.5 (/ frame-x 5)) (/ frame-y 2))))
(define psi-x2 (atan (/ (* 0.5 (/ frame-x 5)) (/ frame-y 2))))

(define ro-y1 (sqrt (+ (sqr (/ frame-x 2)) (sqr (* (/ frame-y 8) 3)))))
(define ro-y2 (sqrt (+ (sqr (/ frame-x 2)) (sqr (* (/ frame-y 8) 2)))))
(define ro-y3 (sqrt (+ (sqr (/ frame-x 2)) (sqr (* (/ frame-y 8) 1)))))
(define ro-y4 (/ frame-x 2))
(define psi-y1 (atan (/ (* 3 (/ frame-y 8)) (/ frame-x 2))))
(define psi-y2 (atan (/ (* 2 (/ frame-y 8)) (/ frame-x 2))))
(define psi-y3 (atan (/ (* 1 (/ frame-y 8)) (/ frame-x 2))))

;Living Blocks

(define block-h 15)
(define block-l 14)
(define mid-block-levels 4)
(define out-block-levels 3)
(define mid-floor-h (/ block-h mid-block-levels))
(define out-floor-h (/ block-h out-block-levels))

(define mid-block-r 35)
(define slab-thickness 0.5)
(define wall-thickness 0.2)
(define door 2)
(define spiral-stairs-width 2)

(define glass-thickness 0.01)
(define column-r 0.25)
(define column-d (* 2 column-r))

#|BIM PARAMETERS|#

(default-slab-family
  (slab-family-element (load-slab-family "Slab")
                       #:thickness slab-thickness))
(default-column-family
  (column-family-element (load-column-family "Column")
                         #:width column-d #:depth column-d #:circular-section? #f))
(default-beam-family
  (beam-family-element (load-beam-family "Beam")
                       #:width column-d #:height column-d))

(define beam-family-for-slab
  (let ((dz-slb (/ (* 2 block-h) n-frames)))
    (beam-family-element (load-beam-family "SlabBeam")
                         #:width slab-thickness #:height dz-slb)))
(default-wall-family
  (wall-family-element (load-wall-family "Wall")
                        #:thickness wall-thickness))

(define glass-family
  (wall-family-element (load-wall-family "GlassWall")
                        #:thickness glass-thickness))


;STEEL FRAMES _________________________________________________________________

(define (bar pts)
  (define (dif-almost0? a b) (< -.5 (- a b) .5))
  (define (vertical? p1 p2)
    (and (dif-almost0? (cx p1) (cx p2)) (dif-almost0? (cy p1) (cy p2))))
  (for/list ((p0 pts) (p1 (cdr pts)))
    (beam p0 p1)))

(define (frames p)
  (let ((p1 (+z p (* 1.6 block-h)))
        (r (+ mid-block-r (/ block-l 2))))
    (for/list ((fi (division 2pi 0 n-frames #f))
               (psi (division (- (/ pi 2)) (/ pi 2) n-frames)))
      (bar (list
            (+sph (+pol p1 r fi) ro-f fi (+ psi psiA))
            (+sph (+pol p1 r fi) ro-f fi (+ psi psiB))
            (+sph (+pol p1 r fi) ro-f fi (+ psi psiC))
            (+sph (+pol p1 r fi) ro-f fi (+ psi psiD))
            (+sph (+pol p1 r fi) ro-f fi (+ psi psiA)))))))

;STEEL BAYS____________________________________________________________________
          
(define (bays-contour-points p)
  (let ((p1 (+z p (* 1.6 block-h)))
        (r (+ mid-block-r (/ block-l 2))))
    (for/list ((fi (division 2pi 0 n-frames))
               (psi (division (- (/ pi 2)) (/ pi 2) n-frames)))
      (list (+sph (+pol p1 r fi) ro-f fi (+ psi psiA))
            (+sph (+pol p1 r fi) ro-f fi (+ psi psiB))
            (+sph (+pol p1 r fi) ro-f fi (+ psi psiC))      
            (+sph (+pol p1 r fi) ro-f fi (+ psi psiD))))))

(define (bays-points p)
  (define (27-track f-points n count)
    (cond ((= count 27) (list))
          ((= n (length f-points))
           (cons (list-ref f-points 25)
                 (27-track f-points 0 (+ count 1))))
          (else (cons (list-ref f-points n)
                      (27-track f-points (+ n 1) (+ count 1))))))
  (let ((p1 (+z p (* 1.6 block-h)))
        (r (+ mid-block-r (/ block-l 2))))
    (define (f-points fi psi)
      (list (+sph (+pol p1 r fi) ro-y1 fi (+ psi 2pi psi-y1))
            (+sph (+pol p1 r fi) ro-y2 fi (+ psi 2pi psi-y2))   
            (+sph (+pol p1 r fi) ro-y3 fi (+ psi 2pi psi-y3))
            (+sph (+pol p1 r fi) ro-y4 fi (+ psi 2pi))
            (+sph (+pol p1 r fi) ro-y3 fi (+ psi (- 2pi psi-y3)))
            (+sph (+pol p1 r fi) ro-y2 fi (+ psi (- 2pi psi-y2)))
            (+sph (+pol p1 r fi) ro-y1 fi (+ psi (- 2pi psi-y1)))
            (+sph (+pol p1 r fi) ro-f fi (+ psi psiD))
            (+sph (+pol p1 r fi) ro-x1 fi (+ psi (* 3 pi/2) psi-x1))
            (+sph (+pol p1 r fi) ro-x2 fi (+ psi (* 3 pi/2) psi-x2))
            (+sph (+pol p1 r fi) ro-x2 fi (+ psi (- (* 3 pi/2) psi-x2)))
            (+sph (+pol p1 r fi) ro-x1 fi (+ psi (- (* 3 pi/2) psi-x1)))
            (+sph (+pol p1 r fi) ro-f fi (+ psi psiC))
            (+sph (+pol p1 r fi) ro-y1 fi (+ psi pi psi-y1))
            (+sph (+pol p1 r fi) ro-y2 fi (+ psi pi psi-y2))   
            (+sph (+pol p1 r fi) ro-y3 fi (+ psi pi psi-y3))
            (+sph (+pol p1 r fi) ro-y4 fi (+ psi pi))
            (+sph (+pol p1 r fi) ro-y3 fi (+ psi (- pi psi-y3)))
            (+sph (+pol p1 r fi) ro-y2 fi (+ psi (- pi psi-y2)))
            (+sph (+pol p1 r fi) ro-y1 fi (+ psi (- pi psi-y1)))
            (+sph (+pol p1 r fi) ro-f fi (+ psi psiB))
            (+sph (+pol p1 r fi) ro-x1 fi (+ psi pi/2 psi-x1))
            (+sph (+pol p1 r fi) ro-x2 fi (+ psi pi/2 psi-x2))
            (+sph (+pol p1 r fi) ro-x2 fi (+ psi (- pi/2 psi-x2)))
            (+sph (+pol p1 r fi) ro-x1 fi (+ psi (- pi/2 psi-x1)))
            (+sph (+pol p1 r fi) ro-f fi (+ psi psiA))))
    (for/list ((fi (division 2pi 0 n-frames))
                     (psi (division (- (/ pi 2)) (/ pi 2) n-frames))
                     (n (append (division 0 26 26) (division 0 26 26))))
            (27-track (f-points fi psi) n 0))))

(define (transpose-matrix mtrx)
  (if (null? (car mtrx))
      (list)
      (cons (map car mtrx)
            (transpose-matrix (map cdr mtrx)))))

(define (bays p)
  (map bar (transpose-matrix (bays-contour-points p)))
  (map bar (transpose-matrix (bays-points p))))


;BLOCKS__________________________________________________________________________

;round middle slabs

(define (mid-block-slab p z)
  (let ((z1 (+ z block-h)))
    (slab-opening-path
     (slab-path (virtual (circle p (+ (cx p) (+ mid-block-r block-l))))
                (level z1))
     (virtual (circle p (+ (cx p) mid-block-r))))))

(define (mid-block-slabs p)
  (for/list ((z (division 0 block-h mid-block-levels)))
    (mid-block-slab p z)))

;slab-piece defenition

(define (block-slab p fi0 r0 r1 z i/o)
  (let ((dfi (/ 2pi n-frames)))
    (define (case-normal p fi0 r0 r1 z)
      (slab
       (append (for/list ((fi (division fi0 (+ fi0 dfi) 10))
                          (ri (division r0 r1 10)))
                 (+pol p ri fi))
               (for/list ((fi (division (+ fi0 dfi) fi0 10))
                          (ri (division r1 r0 10)))
                 (+pol p (+ ri block-l) fi)))))
    (define (case-in-cut p fi0 r0 r1 z)
      (slab
       (append (for/list ((fi (division fi0 (+ fi0 dfi) 10))
                          (ri (division r0 r1 10)))
                 (+pol p ri fi))
               (if (= fi0 (+ 2pi dfi))
                   (for/list ((fi (division (+ fi0 dfi) fi0 10 #f)))
                     (+pol p mid-block-r fi))
                   (for/list ((fi (division (+ fi0 dfi) fi0 10)))
                     (+pol p mid-block-r fi))))))
    (define (case-out-cut p fi0 r0 r1 z)
      (slab (append                
             (if (< fi0 dfi)
                 (cdr (for/list ((fi (division fi0 (+ fi0 dfi) 10)))
                        (+pol p (+ mid-block-r block-l) fi)))
                 (for/list ((fi (division fi0 (+ fi0 dfi) 10)))
                   (+pol p (+ mid-block-r block-l) fi)))
             (for/list ((fi (division (+ fi0 dfi) fi0 10))
                        (ri (division r1 r0 10)))
               (+pol p (+ ri block-l) fi)))))
    (parameterize ((current-level (level z)))
      (cond ((and (= i/o 1) (< (- block-h 3) z (+ 3 (* block-h 2))) (> fi0 2pi))
             (case-in-cut p fi0 r0 r1 z))
            ((and (= i/o 1) (< (- block-h 3) z (+ 3 (* block-h 2))) (> fi0 0))
             (case-out-cut p fi0 r0 r1 z))
            ((and (= i/o 0) (< block-h z (* 2 block-h))(> fi0 2pi))
             (case-in-cut p fi0 r0 r1 z))
            ((and (= i/o 0) (< block-h z (* 2 block-h)) (> fi0 0))
             (case-out-cut p fi0 r0 r1 z))
            (else
             (case-normal p fi0 r0 r1 z))))))

;outer top and bottom slabs

(define (rfunc ri)
  (- mid-block-r (* block-l (sin (* 0.5 ri)))))

(define (out-block-out-slabs p)
  (for/list ((dz (division 0 (* 2 block-h) n-frames #f))
             (ri1 (division 0 2pi n-frames #f))
             (ri2 (division (/ 2pi n-frames)
                            (+ 2pi (/ 2pi n-frames)) n-frames #f))
             (fi0 (division 4pi 2pi n-frames #f)))
    (block-slab p fi0 (rfunc ri2) (rfunc ri1) dz 0)
    (block-slab p fi0 (rfunc ri2) (rfunc ri1) (+ dz block-h) 0))
  (for/list ((dz (division (* 2 block-h) 0 n-frames #f))
             (ri1 (division 2pi 4pi n-frames #f))
             (ri2 (division (+ 2pi (/ 2pi n-frames))
                            (+ 4pi (/ 2pi n-frames)) n-frames #f))
             (fi0 (division 2pi 0 n-frames #f)))
    (block-slab p fi0 (rfunc ri2) (rfunc ri1) dz 0)
    (block-slab p fi0 (rfunc ri2) (rfunc ri1) (+ dz block-h) 0)))

;outer middle slabs

(define (out-block-in-slabs p fi0 fir z0)
  (let ((zmid (/ block-h mid-block-levels))
        (dz (/ block-h (/ n-frames 2)))
        (dfi (/ pi (/ n-frames 2)))
        (r (rfunc fir))
        (r1 (rfunc (+ fir (/ pi (/ n-frames 2)))))
        (r2 (rfunc (+ fir (* 2 (/ pi (/ n-frames 2))))))
        (r3 (rfunc (+ fir (* 3 (/ pi (/ n-frames 2)))))))
    (cond ((> fi0 (* 37 (/ pi 18)))
           (begin
             (block-slab p fi0 r1 r (+ z0 zmid) 1)
             (block-slab p fi0 r1 r (+ z0 (* 3 zmid)) 1)
             (block-slab p (- fi0 dfi) r2 r1 (+ z0 dz dz dz zmid) 1)
             (block-slab p (- fi0 dfi) r2 r1 (+ z0 (* 3 zmid)) 1)
             (block-slab p (- fi0 dfi dfi) r3 r2 (+ z0 dz dz dz zmid) 1)
             (block-slab p (- fi0 dfi dfi) r3 r2 (+ z0 (* 3 zmid)) 1)
             (out-block-in-slabs p (- fi0 (* 3 dfi)) (+ fir (* 3 dfi)) (+ z0 (* 3 dz)))))
          ((> fi0 (+ dfi 0))
           (begin
             (block-slab p fi0 r1 r (+ z0 zmid) 1)
             (block-slab p fi0 r1 r (+ z0 (* 3 zmid)) 1)
             (block-slab p (- fi0 dfi) r2 r1 (+ z0 zmid) 1)
             (block-slab p (- fi0 dfi) r2 r1 (- (+ z0 (* 3 zmid)) (* 3 dz)) 1)
             (block-slab p (- fi0 dfi dfi) r3 r2 (+ z0 zmid) 1)
             (block-slab p (- fi0 dfi dfi) r3 r2 (- (+ z0 (* 3 zmid)) (* 3 dz)) 1)
             (out-block-in-slabs p (- fi0 (* 3 dfi)) (+ fir (* 3 dfi)) (- z0 (* 3 dz)))))
          (else #t))))

;slab beams

(define (slab-beam p fi0 ri z)
  (define dz-slb (/ (* 2 block-h) n-frames))
  (define (dfi ri)
    (atan (/ slab-thickness (* 2 ri))))
  (let
      ((p1i (+cyl p ri (- fi0 (dfi ri)) (+ z dz-slb)))
       (p2i (+cyl p (+ ri block-l) (- fi0 (dfi (+ ri block-l))) (+ z dz-slb)))
       (p3i (+cyl p mid-block-r (- fi0 (dfi mid-block-r)) (+ z dz-slb)))
       (p1o (+cyl p ri (+ fi0 (dfi ri)) z))
       (p2o (+cyl p (+ ri block-l) (+ fi0 (dfi (+ ri block-l))) z))
       (p4o (+cyl p (+ mid-block-r block-l) (+ fi0 (dfi (+ mid-block-r block-l))) z)))
    (define (case-in-norm p fi0 ri z)
      (if (and (= fi0 4pi) (= z block-h)) #t (beam p2i p1i)))
    (define (case-out-norm p fi0 ri z) (beam p1o p2o))
    (define (case-in-cut p fi0 ri z)
      (if (= (cx p3i) (cx p1i)) #t (beam p3i p1i)))
    (define (case-out-cut p fi0 ri z)
      (if (= (cx p4o) (cx p2o)) #t (beam p4o p2o)))
    (parameterize ((default-beam-family beam-family-for-slab))
      (cond ((and (< block-h z (* 2 block-h))(> fi0 2pi))
             (case-in-cut p fi0 ri z))
            ((and (< block-h z (* 2 block-h)) (> fi0 0))
             (case-out-cut p fi0 ri z))
            ((> fi0 2pi)
             (case-in-norm p fi0 ri z))
            (else (case-out-norm p fi0 ri z))))))

(define (out-slab-beams p)
  (for/list ((dz (division (- slab-thickness) (- (* 2 block-h) slab-thickness) n-frames #f))
             (ri (division (/ 2pi n-frames) (+ 2pi (/ 2pi n-frames)) n-frames #f))
             (fi0 (division 4pi 2pi n-frames #f)))
    (slab-beam p fi0 (rfunc ri) dz)
    (slab-beam p fi0 (rfunc ri) (+ dz block-h)))
  (for/list ((dz (division (- (* 2 block-h) slab-thickness) (- slab-thickness) n-frames #f))
             (ri (division (+ 2pi (/ 2pi n-frames)) (+ 4pi (/ 2pi n-frames)) n-frames #f))
             (fi0 (division 2pi 0 n-frames #f)))
    (slab-beam p fi0 (rfunc ri) dz)
    (slab-beam p fi0 (rfunc ri) (+ dz block-h))))

;COLUMNS AND CROSSBEAMS_______________________________________________________________________

(define (column-rotation p fi r z h)
  (let ((p1 (+cyl p r fi z))
        (p2 (+cyl p r fi (+ z h))))
    (if (= h 0)
        (list)
        (beam p1 p2 fi))))

(define (mid-columns p)
  (for/list ((fi (division 0 2pi n-frames 2)))
    (column-rotation p fi (+ mid-block-r 0.25) block-h block-h)
    (column-rotation p fi (+ mid-block-r block-l -0.25) block-h block-h)))

(define (cross-beams p)
 ; (parameterize ((trim? #t))
  (let ((dfi (/ 2pi n-frames))
        (ri (+ mid-block-r 0.25))
        (ro (+ mid-block-r block-l -0.25))
        (h0 block-h)
        (h1 (* 2 block-h)))
    (for/list ((fi (division 0 2pi (/ n-frames 2) 2)))
      (beam (+cyl p ri fi h0) (+cyl p ri (+ fi dfi) h1))
      (beam (+cyl p ri (+ fi (* 2 dfi)) h0) (+cyl p ri (+ fi dfi) h1))
      (beam (+cyl p ro fi h0) (+cyl p ro (+ fi dfi) h1))
      (beam (+cyl p ro (+ fi (* 2 dfi)) h0) (+cyl p ro (+ fi dfi) h1)))))

(define (out-columns p)
  (let ((dz (/ block-h (/ n-frames 2))))
    (for/list ((z (division 0 block-h (/ n-frames 2) #f))
               (ri (division (/ pi (/ n-frames 2))
                             (+ pi (/ pi (/ n-frames 2))) (/ n-frames 2) #f))
               (fi0 (division 4pi 3pi (/ n-frames 2) #f)))
      (column-rotation p fi0 (+ (rfunc ri) column-r) (- z slab-thickness) (+ slab-thickness block-h dz))
      (column-rotation p fi0 (+ (rfunc ri) block-l (- column-r)) block-h (- (+ block-h (- z) slab-thickness))))
    
    (for/list ((z (division block-h (* 2 block-h) (/ n-frames 2) #f))
               (ri (division (+ pi (/ pi (/ n-frames 2)))
                             (+ 2pi (/ pi (/ n-frames 2))) (/ n-frames 2) #f))
               (fi0 (division 3pi 2pi (/ n-frames 2) #f)))
      (column-rotation p fi0 (+ (rfunc ri) column-r) (- z slab-thickness) (+ slab-thickness block-h dz))
      (column-rotation p fi0 (+ (rfunc ri) block-l (- column-r))
              (* 2 block-h) (+ z dz (- block-h))))
    
    (for/list ((z (division (* 2 block-h) block-h (/ n-frames 2) #f))
               (ri (division (+ 2pi (/ pi (/ n-frames 2)))
                             (+ 3pi (/ pi (/ n-frames 2))) (/ n-frames 2) #f))
               (fi0 (division 2pi pi (/ n-frames 2) #f)))
      (column-rotation p fi0 (+ (rfunc ri) block-l (- column-r)) (- z dz slab-thickness) (+ slab-thickness block-h dz))
      (column-rotation p fi0 (+ (rfunc ri) column-r) (* 2 block-h) (- z block-h)))
    
    (for/list ((z (division block-h 0 (/ n-frames 2) #f))
               (ri (division (+ 3pi (/ pi (/ n-frames 2)))
                             (+ 4pi (/ pi (/ n-frames 2))) (/ n-frames 2) #f))
               (fi0 (division pi 0 (/ n-frames 2) #f)))
      (column-rotation p fi0 (+ (rfunc ri) block-l (- column-r)) (- z dz slab-thickness) (+ slab-thickness block-h dz))
      (column-rotation p fi0 (+ (rfunc ri) column-r) block-h (- (+ block-h (- z) slab-thickness dz))))))


;STAIRS________________________________________________________________________

;all around stairs
#|CAD|#
#;(define (steplist p fi0 r0 r1 z)
  (let ((dfi (/ 2pi (* 6 n-frames))))
    (append (for/list ((fi (division fi0 (+ fi0 dfi) 4))
                       (ri (division r1 r0 4)))
              (+cyl p ri fi z))
            (for/list ((fi (division (+ fi0 dfi) fi0 4))
                       (ri (division r0 r1 4)))
              (+cyl p (+ ri spiral-stairs-width) fi z)))))

#;(define (6step p fi0 fir z-slb case)
  #|CAD|#
  (let ((dfi (/ 2pi (* 6 n-frames)))
        (z-stp (/ block-h (* n-frames 3)))
        (r0 (rfunc fir))
        (r1 (rfunc (+ fir (/ 2pi (* 6 n-frames)))))
        (r2 (rfunc (+ fir (* 2 (/ 2pi (* 6 n-frames))))))
        (r3 (rfunc (+ fir (* 3 (/ 2pi (* 6 n-frames))))))
        (r4 (rfunc (+ fir (* 4 (/ 2pi (* 6 n-frames))))))
        (r5 (rfunc (+ fir (* 5 (/ 2pi (* 6 n-frames))))))
        (r6 (rfunc (+ fir (* 6 (/ 2pi (* 6 n-frames))))))
        (rfora (- block-l spiral-stairs-width)))
    (if (= case 1)
        (begin   
          (extrusion (surface-polygon (steplist p (- fi0 (* 2 dfi)) r1 r2 z-slb)) (* -1 z-stp))
          (extrusion (surface-polygon (steplist p (- fi0 (* 3 dfi)) r2 r3 z-slb)) (* -2 z-stp))
          (extrusion (surface-polygon (steplist p (- fi0 (* 4 dfi)) r3 r4 z-slb)) (* -3 z-stp))
          (extrusion (surface-polygon (steplist p (- fi0 (* 5 dfi)) r4 r5 z-slb)) (* -4 z-stp))
          (extrusion (surface-polygon (steplist p (- fi0 (* 6 dfi)) r5 r6 z-slb)) (* -5 z-stp)))
        (begin   
          (extrusion
           (surface-polygon (steplist p (- fi0 (* 1 dfi)) (+ rfora r0) (+ rfora r1) z-slb)) (* -5 z-stp))
          (extrusion
           (surface-polygon (steplist p (- fi0 (* 2 dfi)) (+ rfora r1) (+ rfora r2) z-slb)) (* -4 z-stp))
          (extrusion
           (surface-polygon (steplist p (- fi0 (* 3 dfi)) (+ rfora r2) (+ rfora r3) z-slb)) (* -3 z-stp))
          (extrusion
           (surface-polygon (steplist p (- fi0 (* 4 dfi)) (+ rfora r3) (+ rfora r4) z-slb)) (* -2 z-stp))
          (extrusion
           (surface-polygon (steplist p (- fi0 (* 5 dfi)) (+ rfora r4) (+ rfora r5) z-slb)) (* -1 z-stp))))))
#|BIM
;PROBEMA: O RAIO NÃO E CONSTANTE - SEGUA A FUNCAO (rfunc ri)
  (stairs "Stair Spiral 18" ;inside - bottom
        p
        #:use-xy-fix-size #t
        #:x-ratio (* 2 (+ mid-block-r spiral-stairs-width)) ;Dimention1 - outer diameter
        #:additional-parameters (list (list "zzyzx" (* 2 block-h)) ;Total Height
                                      (list "nRisers" (* n-frames 6)) ;No. of Risers
                                      (list "angle" -2pi) ;Angle of Total Rotation
                                      (list "swelldia" (* 2 mid-block-r)) ;Stair Well Diameter - inner diameter
                                      (list "rightRailType_m" 0) ;Railing Type 0/1
                                      (list "leftRailType_m" 0) ;Railing Type 0/1
                                      (list "stairBaseType_m" 0))) ;Stair structural Base
  (stairs "Stair Spiral 18" ;outside - bottom
        p
        #:use-xy-fix-size #t
        #:x-ratio (* 2 (+ mid-block-r block-l)) ;Dimention1 - outer diameter
        #:additional-parameters (list (list "zzyzx" (* 2 block-h)) ;Total Height
                                      (list "nRisers" (* n-frames 6)) ;No. of Risers
                                      (list "angle" 2pi) ;Angle of Total Rotation
                                      (list "swelldia" (* 2 (- (+ mid-block-r block-l)spiral-stairs-width))) ;Stair Well Diameter - inner diameter
                                      (list "rightRailType_m" 0) ;Railing Type 0/1
                                      (list "leftRailType_m" 0) ;Railing Type 0/1
                                      (list "stairBaseType_m" 0))) ;Stair structural Base
|#
#;(define (spiral-steps p)
  (let ((dz-slab (/ block-h (/ n-frames 2)))
        (dr-slab (/ 2pi n-frames)))
    (for/list ((dz (division 0 (* 2 block-h) n-frames #f))
               (fi0 (division (+ 4pi dr-slab) (+ 2pi dr-slab) n-frames #f))
               (fir (division 0 2pi n-frames #f)))
      (6step p fi0 fir (+ slab-thickness dz) 1)
      (6step p fi0 fir (+ slab-thickness dz block-h) 1))
    (for/list ((dz (division (- (* 2 block-h) dz-slab) (- dz-slab) n-frames #f))
               (fi0 (division 2pi 0 n-frames #f))
               (fir (division (+ dr-slab 2pi) (+ dr-slab 4pi) n-frames #f)))
      (6step p fi0 fir (+ slab-thickness dz) 2)
      (6step p fi0 fir (+ slab-thickness dz block-h) 2))))
 
; in block stairs

(define stp-rise 0.18)
(define stp-run 0.29)
(define stair-width 1.5)
(define dz-top/bottom (/ block-h (/ n-frames 2)))
(define stair-h (* 3 dz-top/bottom))
(define n-steps (floor (/ stair-h stp-rise)))

(define (flight-of-stair p fi0 z n rmin rmax up/down)
  (define alfa (* 2 (atan (/ stp-run (* 2 rmin)))))
  #|CAD|#
  #;(let ((dfi (/ pi (/ n-frames 2)))
        (p1 (+cyl p rmin fi0 z))
        (p2 (+cyl p rmax fi0 z))
        (p3 (+cyl p rmax (+ alfa fi0) z))
        (p4 (+cyl p rmin (+ alfa fi0) z))
        (p5 (+cyl p rmax (- fi0 alfa) z))
        (p6 (+cyl p rmin (- fi0 alfa) z)))
    (if (= up/down 0)
        (if (= n 0) #t
            (begin (extrusion (surface-polygon p1 p2 p3 p4) (* -2 stp-rise))
                   (stair p (+ fi0 alfa) (- z stp-rise) (sub1 n) rmin rmax 0)))
        (if (= n 0) #t
            (begin (extrusion (surface-polygon p1 p2 p5 p6) (* -2 stp-rise)) 
                   (stair p (- fi0 alfa) (- z stp-rise) (sub1 n) rmin rmax 1)))))
  #|BIM|#
  (if (= up/down 0)
      (stairs "Stair Spiral 18" p ;Object Stair type
              #:bottom-offset z ;Base Height
              #:angle fi0 ;Rotation (initial angle)
              #:use-xy-fix-size #t
              #:x-ratio (* 2 rmax) ;Dimention1 - outer diameter
              #:properties (list "zzyzx" stair-h ;Total Height
                                 "nRisers" n-steps ;No. of Risers
                                 "angle" (* n-steps alfa) ;Angle of Total Rotation
                                 "swelldia" (* 2 rmin) ;Stair Well Diameter - inner diameter
                                 "rightRailType_m" 0 ;Railing Type 0/1
                                 "leftRailType_m" 0 ;Railing Type 0/1
                                 "stairBaseType_m" 0)) ;Stair structural Base
      (stairs "Stair Spiral 18" p ;Object Stair type
              #:bottom-offset z ;Base Height
              #:angle fi0 ;Rotation (initial angle)
              #:use-xy-fix-size #t
              #:x-ratio (* 2 rmax) ;Dimention1 - outer diameter
              #:properties (list "zzyzx" stair-h ;Total Height
                                 "nRisers" n-steps ;No. of Risers
                                 "angle" (- (* n-steps alfa)) ;Angle of Total Rotation
                                 "swelldia" (* 2 rmin) ;Stair Well Diameter - inner diameter
                                 "rightRailType_m" 0 ;Railing Type 0/1
                                 "leftRailType_m" 0 ;Railing Type 0/1
                                 "stairBaseType_m" 0)))) ;Stair structural Base

(define (3slab-set p fi3 z3 rmin rmax)
  (let ((zmid (/ block-h mid-block-levels))
        (dfi (/ pi (/ n-frames 2)))
        (dfu3 (/ 2pi (/ n-frames 3)))
        (dz (/ block-h (/ n-frames 2))))
    (cond ((= fi3 4pi)
           (flight-of-stair p fi3 (+ z3 stair-h zmid) n-steps rmax (+ rmax stair-width) 0)
           (flight-of-stair p (- fi3 dfi dfi) (+ z3 stair-h (* 3 zmid)) n-steps rmin rmax 0))
          ((> fi3 (+ dfu3 2pi))
           (flight-of-stair p fi3 (+ z3 stair-h zmid) n-steps rmin rmax 0)
           (flight-of-stair p (- fi3 dfi dfi) (+ z3 stair-h (* 3 zmid)) n-steps rmin rmax 0))
          ((= fi3 (+ dfu3 2pi))
           (flight-of-stair p fi3 (+ z3 stair-h zmid) n-steps rmin rmax 0)
           (flight-of-stair p (- fi3 dfi dfi) (+ z3 stair-h (* 3 zmid))n-steps rmax (+ rmax stair-width) 0))
          ((= fi3 2pi)
           (flight-of-stair p fi3 (+ z3 (* 3 zmid) (* -2 stp-rise)) n-steps (- rmin stair-width) rmin 1)
           (flight-of-stair p (- fi3 dfi dfi) (+ z3 zmid (* -2 stp-rise)) n-steps rmin rmax 1))
          ((> fi3 (+ dfi dfu3))
           (flight-of-stair p fi3 (+ z3 (* 3 zmid) (* -2 stp-rise)) n-steps rmin rmax 1)
           (flight-of-stair p (- fi3 dfi dfi) (+ z3 zmid (* -2 stp-rise)) n-steps rmin rmax 1))
          (else
           (flight-of-stair p fi3 (+ z3 (* 3 zmid) (* -2 stp-rise)) n-steps rmin rmax 1)
           (flight-of-stair p (- fi3 dfi dfi) (+ z3 zmid (* -2 stp-rise)) n-steps (- rmin stair-width) rmin 1)))))
           
(define (stair-cases p)
  (let ((rmin-in (- mid-block-r stair-width))
        (rmax-in mid-block-r)
        (rmin-out (+ mid-block-r block-l))
        (rmax-out (+ mid-block-r block-l stair-width))
        (dfi (/ pi (/ n-frames 2))))
    (for/list ((fi3 (division 4pi 2pi (/ n-frames 3) #f))
               (z3 (division 0 (* 2 block-h) (/ n-frames 3) #f)))
      (3slab-set p fi3 (+ z3 slab-thickness (- stp-rise)) rmin-in rmax-in))
    (for/list ((fi3 (division 2pi 0 (/ n-frames 3) #f))
               (z3 (division (* 2 block-h) 0 (/ n-frames 3) #f)))
      (3slab-set p fi3 (+ z3 slab-thickness (- stp-rise)) rmin-out rmax-out))))


;LIBRARY WALLS__________________________________________________________________

(define (book-wall p fi z)
  (let ((p1 (+cyl p (+ mid-block-r door) fi z))
        (p2 (+cyl p (+ mid-block-r (/ block-l 2) (/ door -2)) fi z))
        (p3 (+cyl p (+ mid-block-r (/ block-l 2) (/ door 2)) fi z))
        (p4 (+cyl p (+ mid-block-r block-l (- door)) fi z))
        (pa (+x (u0) (- (/ wall-thickness 2))))
        (pb (+xy (u0) (/ wall-thickness 2) (/ block-h 4))))
    (parameterize ((current-level (level z))
                   (default-level-to-level-height (/ block-h 4)))
      (wall p1 p2) (wall p3 p4))))

(define (book-walls p)
  (for/list ((z (division block-h (* 2 block-h) mid-block-levels #f)))
    (for/list ((fi0 (division 0 2pi n-frames #f)))
      (book-wall p fi0 z))))

(define (glass-wall fi z r)
  (let ((dfi (/ 2pi n-frames))
        (dz (/ (- (+ slab-thickness (* 2 block-h)) (+ slab-thickness block-h)) 4)))
    (cond ((and (= z (+ slab-thickness block-h)) (or (= fi 3pi) (= fi pi)))
           (list))
          ((and (= z (+ slab-thickness block-h dz))
                (or (= fi (* 3.5 pi))
                    (= fi (- (* 3.5 pi) dfi)) (= fi (- (* 3.5 pi) (* 2 dfi)))
                    (<= (* 0.5 pi) fi (+ (* 0.5 pi) (* 2 dfi)))
                    (= fi 3pi)
                    (= fi (+ 3pi dfi)) (= fi (+ 3pi (* 2 dfi)))
                    (= fi pi)
                    (= fi (- pi dfi)) (= fi (- pi (* 2 dfi)))))
           (list))
          ((and (= z (+ slab-thickness block-h (* 2 dz)))
                (or (= fi (* 3.5 pi)) (= fi (* 2.5 pi)) (= fi (* 1.5 pi)) (= fi (* 0.5 pi))))
           (list))           
          ((and (= z (+ slab-thickness block-h(* 3 dz)))
                (or (= fi (* 2.5 pi))
                    (= fi (+ (* 2.5 pi) dfi)) (= fi (+ (* 2.5 pi) (* 2 dfi)))
                    (= fi (* 1.5 pi))
                    (= fi (- (* 1.5 pi) dfi)) (= fi (- (* 1.5 pi) (* 2 dfi)))
                    (= fi 3pi)
                    (= fi (- 3pi dfi)) (= fi (- 3pi (* 2 dfi)))
                    (= fi pi)
                    (= fi (+ pi dfi)) (= fi (+ pi (* 2 dfi)))))
           (list))
          (else
           (parameterize ((current-level (level (- z slab-thickness)))
                          (default-level-to-level-height (/ block-h 4))
                          (default-wall-family glass-family))
             (walls
              (for/list ((fii (division fi (+ fi dfi) 10)))
                (+pol p r fii))))))))

(define (glass-walls p)
  (for/list ((fi (division 4pi 2pi n-frames)))
    (for/list ((z (division (+ slab-thickness block-h)
                            (+ slab-thickness (* 2 block-h)) mid-block-levels #f)))
      (glass-wall fi z (+ mid-block-r glass-thickness))))
  (for/list ((fi (division 2pi 0 n-frames #f)))
    (for/list ((z (division (+ slab-thickness block-h)
                            (+ slab-thickness (* 2 block-h)) mid-block-levels #f)))
      (glass-wall fi z (+ mid-block-r block-l (- glass-thickness) -0.2)))))


;CORES___________________________________________________________________________
#|
(define (core4 p fi h)
  (let ((p1 (+pol p (+ mid-block-r door) 0))
        (p2 (+pol p (+ mid-block-r block-l (- door)) 0))
        (pa (+x (u0) (- (/ wall-thickness 2))))
        (pb (+xy (u0) (/ wall-thickness 2) h)))
    (rotate
     (union (sweep (line (+y p1 3) (+y p2 3) (+y p2 -3) (+y p1 -3) (+y p1 3))
                   (surface-rectangle pa pb))
            (extrusion
             (surface-polygon (+yz p1 3 h) (+yz p2 3 h) (+yz p2 -3 h) (+yz p1 -3 h))
             slab-thickness))
     fi p)))

(define (core1 p h)
  (let ((p1 (+pol p (+ mid-block-r door) 0))
        (p2 (+pol p (+ mid-block-r block-l (- door)) 0))
        (pa (+x (u0) (- (/ wall-thickness 2))))
        (pb (+xy (u0) (/ wall-thickness 2) h)))
    (union (sweep (line p1 (+y p1 6) (+y p2 6) p2 p1)
                  (surface-rectangle pa pb))
           (extrusion
            (surface-polygon (+z p1 h) (+yz p1 6 h) (+yz p2 6 h) (+z p2 h) (+z p1 h))
            slab-thickness))))

(define (cores p)
  (core1 p (+ (* 3 block-h) (/ block-h out-block-levels)))
  (core4 p (* 7 (/ pi 18)) (* 3 block-h))
  (core4 p (* 7 (/ pi 9)) (+ (* 2 block-h) (/ block-h out-block-levels)))
  (core4 p (- pi/3) (* 3 block-h))
  (core4 p (* -13 (/ pi 18)) (+ (* 2 block-h) (/ block-h out-block-levels))))
|#                       

;CONSTRUCT_________________________________________________________________

(mid-block-slabs p)
;(out-block-out-slabs p)
;(out-slab-beams p)
;(out-block-in-slabs p 4pi 0 0)

;(mid-columns p)
;(cross-beams p)
;(out-columns p)

(book-walls p)
;(glass-walls p)
;(cores p)
;(spiral-steps p)
;(stair-cases p)

;(frames p)
;(bays p)